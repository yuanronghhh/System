#include <System/Platform/Common/SysTimeZone.h>
#include <System/Platform/Common/SysThread.h>
#include <System/Platform/Common/SysFile.h>
#include <System/Platform/Common/SysDate.h>
#include <System/Platform/Common/SysMappedFile.h>
#include <System/DataTypes/SysArray.h>
#include <System/DataTypes/SysBytes.h>
#include <System/DataTypes/SysHashTable.h>
#include <System/Utils/SysStr.h>
#include <System/Utils/SysPath.h>

/**
 * this code from glib Thread
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

/**
 * SysTimeZone:
 *
 * #SysTimeZone is an opaque structure whose members cannot be accessed
 * directly.
 *
 * Since: 2.26
 **/
/* IANA zoneinfo file format {{{1 */

/* unaligned */
typedef struct { SysChar bytes[8]; } SysInt64_be;
typedef struct { SysChar bytes[4]; } SysInt32_be;
typedef struct { SysChar bytes[4]; } SysUInt32_be;

static SysBool parse_tz_boundary (const SysChar  *identifier, TimeZoneDate *boundary);
static void find_relative_date (TimeZoneDate *buffer);
static SysUInt rules_from_identifier (const SysChar   *identifier, TimeZoneRule **rules);
static void init_zone_from_rules (SysTimeZone    *gtz,
    TimeZoneRule *rules,
    SysUInt         rules_num,
    SysChar        *identifier);

static inline SysInt64 SysInt64_from_be (const SysInt64_be be) {
  SysInt64 tmp; memcpy (&tmp, &be, sizeof tmp); return SYSINT64_FROM_BE (tmp);
}

static inline SysInt32 SysInt32_from_be (const SysInt32_be be) {
  SysInt32 tmp; memcpy (&tmp, &be, sizeof tmp); return SYSINT32_FROM_BE (tmp);
}

static inline SysUInt32 SysUInt32_from_be (const SysUInt32_be be) {
  SysUInt32 tmp; memcpy (&tmp, &be, sizeof tmp); return SYSUINT32_FROM_BE (tmp);
}

/* The layout of an IANA timezone file header */
struct tzhead
{
  SysChar      tzh_magic[4];
  SysChar      tzh_version;
  SysUChar     tzh_reserved[15];

  SysUInt32_be tzh_ttisgmtcnt;
  SysUInt32_be tzh_ttisstdcnt;
  SysUInt32_be tzh_leapcnt;
  SysUInt32_be tzh_timecnt;
  SysUInt32_be tzh_typecnt;
  SysUInt32_be tzh_charcnt;
};

struct ttinfo
{
  SysInt32_be tt_gmtoff;
  SysUInt8    tt_isdst;
  SysUInt8    tt_abbrind;
};

/* A MSWindows-style time zone transition rule. Generalizes the
   MSWindows TIME_ZONE_INFORMATION struct. Also used to compose time
   zones from tzset-style identifiers.
 */
/* SysTimeZone's internal representation of a Daylight Savings (Summer)
   time interval.
 */
typedef struct
{
  SysInt32     gmt_offset;
  SysBool      is_dst;
  SysChar     *abbrev;
} TransitionInfo;

/* SysTimeZone's representation of a transition time to or from Daylight
   Savings (Summer) time and Standard time for the zone. */
typedef struct
{
  SysInt64 time;
  SysInt   info_index;
} Transition;

/* SysTimeZone structure */
struct _SysTimeZone
{
  SysChar   *name;
  SysArray  *t_info;         /* Array of TransitionInfo */
  SysArray  *transitions;    /* Array of Transition */
  SysInt     ref_count;
};

SYS_LOCK_DEFINE_STATIC (time_zones);
static SysHashTable/*<string?, SysTimeZone>*/ *time_zones;
SYS_LOCK_DEFINE_STATIC (tz_default);
static SysTimeZone *tz_default = NULL;
SYS_LOCK_DEFINE_STATIC (tz_local);
static SysTimeZone *tz_local = NULL;

#define min_TZYEAR 1916 /* Daylight Savings started in WWI */
#define max_TZYEAR 2999 /* And it's not likely ever to go away, but
                           there's no point in getting carried
                           away. */

/**
 * sys_time_zone_unref:
 * @tz: a #SysTimeZone
 *
 * Decreases the reference count on @tz.
 *
 * Since: 2.26
 **/
void
sys_time_zone_unref (SysTimeZone *tz)
{
  int ref_count;

again:
  ref_count = sys_atomic_int_get (&tz->ref_count);

  sys_assert (ref_count > 0);

  if (ref_count == 1)
    {
      if (tz->name != NULL)
        {
          SYS_LOCK(time_zones);

          /* someone else might have grabbed a ref in the meantime */
          if SYS_UNLIKELY (sys_atomic_int_get (&tz->ref_count) != 1)
            {
              SYS_UNLOCK(time_zones);
              goto again;
            }

          if (time_zones != NULL)
            sys_hash_table_remove (time_zones, tz->name);
          SYS_UNLOCK(time_zones);
        }

      if (tz->t_info != NULL)
        {
          SysUInt idx;
          for (idx = 0; idx < tz->t_info->len; idx++)
            {
              TransitionInfo *info = &sys_array_index (tz->t_info, TransitionInfo, idx);
              sys_free (info->abbrev);
            }
          sys_array_free (tz->t_info, true);
        }
      if (tz->transitions != NULL)
        sys_array_free (tz->transitions, true);
      sys_free (tz->name);

      sys_slice_free (SysTimeZone, tz);
    }

  else if SYS_UNLIKELY (!sys_atomic_int_cmpxchg (&tz->ref_count,
                                                          ref_count,
                                                          ref_count - 1))
    goto again;
}

/**
 * sys_time_zone_ref:
 * @tz: a #SysTimeZone
 *
 * Increases the reference count on @tz.
 *
 * Returns: a new reference to @tz.
 *
 * Since: 2.26
 **/
SysTimeZone *
sys_time_zone_ref (SysTimeZone *tz)
{
  sys_assert (tz->ref_count > 0);

  sys_atomic_int_inc (&tz->ref_count);

  return tz;
}

/* fake zoneinfo creation (for RFC3339/ISO 8601 timezones) {{{1 */
/*
 * parses strings of the form h or hh[[:]mm[[[:]ss]]] where:
 *  - h[h] is 0 to 24
 *  - mm is 00 to 59
 *  - ss is 00 to 59
 * If RFC8536, TIME_ is a transition time sans sign,
 * so colons are required before mm and ss, and hh can be up to 167.
 * See Internet RFC 8536 section 3.3.1:
 * https://tools.ietf.org/html/rfc8536#section-3.3.1
 * and POSIX Base Definitions 8.3 TZ rule time:
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap08.html#tasys_08_03
 */
static SysBool
parse_time (const SysChar *time_,
            SysInt32      *offset,
            SysBool    rfc8536)
{
  if (*time_ < '0' || '9' < *time_)
    return false;

  *offset = 60 * 60 * (*time_++ - '0');

  if (*time_ == '\0')
    return true;

  if (*time_ != ':')
    {
      if (*time_ < '0' || '9' < *time_)
        return false;

      *offset *= 10;
      *offset += 60 * 60 * (*time_++ - '0');

      if (rfc8536)
        {
          /* Internet RFC 8536 section 3.3.1 and POSIX 8.3 TZ together say
             that a transition time must be of the form [+-]hh[:mm[:ss]] where
             the hours part can range from -167 to 167.  */
          if ('0' <= *time_ && *time_ <= '9')
            {
              *offset *= 10;
              *offset += 60 * 60 * (*time_++ - '0');
            }
          if (*offset > 167 * 60 * 60)
            return false;
        }
      else if (*offset > 24 * 60 * 60)
        return false;

      if (*time_ == '\0')
        return true;
    }

  if (*time_ == ':')
    time_++;
  else if (rfc8536)
    return false;

  if (*time_ < '0' || '5' < *time_)
    return false;

  *offset += 10 * 60 * (*time_++ - '0');

  if (*time_ < '0' || '9' < *time_)
    return false;

  *offset += 60 * (*time_++ - '0');

  if (*time_ == '\0')
    return true;

  if (*time_ == ':')
    time_++;
  else if (rfc8536)
    return false;

  if (*time_ < '0' || '5' < *time_)
    return false;

  *offset += 10 * (*time_++ - '0');

  if (*time_ < '0' || '9' < *time_)
    return false;

  *offset += *time_++ - '0';

  return *time_ == '\0';
}

static SysBool
parse_constant_offset (const SysChar *name,
                       SysInt32      *offset,
                       SysBool    rfc8536)
{
  /* Internet RFC 8536 section 3.3.1 and POSIX 8.3 TZ together say
     that a transition time must be numeric.  */
  if (!rfc8536 && sys_str_equal (name, "UTC"))
    {
      *offset = 0;
      return true;
    }

  if (*name >= '0' && '9' >= *name)
    return parse_time (name, offset, rfc8536);

  switch (*name++)
    {
    case 'Z':
      *offset = 0;
      /* Internet RFC 8536 section 3.3.1 requires a numeric zone.  */
      return !rfc8536 && !*name;

    case '+':
      return parse_time (name, offset, rfc8536);

    case '-':
      if (parse_time (name, offset, rfc8536))
        {
          *offset = -*offset;
          return true;
        }
      else
        return false;

    default:
      return false;
    }
}

static void
zone_for_constant_offset (SysTimeZone *gtz, const SysChar *name)
{
  SysInt32 offset;
  TransitionInfo info;

  if (name == NULL || !parse_constant_offset (name, &offset, false))
    return;

  info.gmt_offset = offset;
  info.is_dst = false;
  info.abbrev =  sys_strdup (name);

  gtz->name = sys_strdup (name);
  gtz->t_info = sys_array_sized_new (false, true, sizeof (TransitionInfo), 1);
  sys_array_append_val (gtz->t_info, info);

  /* Constant offset, no transitions */
  gtz->transitions = NULL;
}

#if defined(SYS_OS_UNIX) && defined(__sun) && defined(__SVR4)
/*
 * only used by Illumos distros or Solaris < 11: parse the /etc/default/init
 * text file looking for TZ= followed by the timezone, possibly quoted
 *
 */
static SysChar *
zone_identifier_illumos (void)
{
  SysChar *resolved_identifier = NULL;
  SysChar *contents = NULL;
  const SysChar *line_start = NULL;
  SysSize tz_len = 0;

  if (!sys_file_get_contents ("/etc/default/init", &contents, NULL, NULL) )
    return NULL;

  /* is TZ= the first/only line in the file? */
  if (strncmp (contents, "TZ=", 3) == 0)
    {
      /* found TZ= on the first line, skip over the TZ= */
      line_start = contents + 3;
    }
  else 
    {
      /* find a newline followed by TZ= */
      line_start = strstr (contents, "\nTZ=");
      if (line_start != NULL)
        line_start = line_start + 4; /* skip past the \nTZ= */
    }

  /* 
   * line_start is NULL if we didn't find TZ= at the start of any line,
   * otherwise it points to what is after the '=' (possibly '\0')
   */
  if (line_start == NULL || *line_start == '\0')
    return NULL;

  /* skip past a possible opening " or ' */
  if (*line_start == '"' || *line_start == '\'')
    line_start++;

  /*
   * loop over the next few characters, building up the length of
   * the timezone identifier, ending with end of string, newline or
   * a " or ' character
   */
  while (*(line_start + tz_len) != '\0' &&
         *(line_start + tz_len) != '\n' &&
         *(line_start + tz_len) != '"'  &&
         *(line_start + tz_len) != '\'')
    tz_len++; 

  if (tz_len > 0)
    {
      /* found it */
      resolved_identifier = sys_strndup (line_start, tz_len);
      sys_strchomp (resolved_identifier);
      sys_free (contents);
      return sys_steal_pointer (&resolved_identifier);
    }
  else
    return NULL;
}
#endif /* defined(__sun) && defined(__SRVR) */

#ifdef SYS_OS_UNIX
/*
 * returns the path to the top of the Olson zoneinfo timezone hierarchy.
 */
static const SysChar * zone_info_base_dir (void) {
  if (sys_file_is_dir ("/usr/share/zoneinfo"))
    return "/usr/share/zoneinfo";     /* Most distros */
  else if (sys_file_is_dir ("/usr/share/lib/zoneinfo"))
    return "/usr/share/lib/zoneinfo"; /* Illumos distros */

  /* need a better fallback case */
  return "/usr/share/zoneinfo";
}

static SysChar *
zone_identifier_unix (void)
{
  SysChar *resolved_identifier = NULL;
  SysSize prefix_len = 0;
  SysChar *canonical_path = NULL;
  SysError *read_link_err = NULL;
  const SysChar *tzdir;
  SysBool not_a_symlink_to_zoneinfo = false;
  struct stat file_status;

  /* Resolve the actual timezone pointed to by /etc/localtime. */
  resolved_identifier = sys_file_read_link ("/etc/localtime", &read_link_err);

  if (resolved_identifier != NULL)
    {
      if (!sys_path_is_absolute (resolved_identifier))
        {
          SysChar *absolute_resolved_identifier = sys_path_build_filename ("/etc", resolved_identifier, NULL);
          sys_free (resolved_identifier);
          resolved_identifier = sys_steal_pointer (&absolute_resolved_identifier);
        }

      if (sys_lstat (resolved_identifier, &file_status) == 0)
        {
          if ((file_status.st_mode & S_IFMT) != S_IFREG)
            {
              /* Some systems (e.g. toolbox containers) make /etc/localtime be a symlink
               * to a symlink.
               *
               * Rather than try to cope with that, just ignore /etc/localtime and use
               * the fallback code to read timezone from /etc/timezone
               */
              sys_clear_pointer (&resolved_identifier, sys_free);
              not_a_symlink_to_zoneinfo = true;
            }
        }
      else
        {
          sys_clear_pointer (&resolved_identifier, sys_free);
        }
    }
  else
    {
      not_a_symlink_to_zoneinfo = true;
      // not_a_symlink_to_zoneinfo = sys_error_matches (read_link_err,
      //                                              SYS_FILE_ERROR,
      //                                              SYS_FILE_ERROR_INVAL);
      // sys_clear_error (&read_link_err);
    }

  if (resolved_identifier == NULL)
    {
      /* if /etc/localtime is not a symlink, try:
       *  - /var/db/zoneinfo : 'tzsetup' program on FreeBSD and
       *    DragonflyBSD stores the timezone chosen by the user there.
       *  - /etc/timezone : Gentoo, OpenRC, and others store
       *    the user choice there.
       *  - call zone_identifier_illumos iff __sun and __SVR4 are defined,
       *    as a last-ditch effort to parse the TZ= setting from within
       *    /etc/default/init
       */
      if (not_a_symlink_to_zoneinfo && (sys_file_get_contents ("/var/db/zoneinfo",
                                                             &resolved_identifier,
                                                             NULL, NULL) ||
                                        sys_file_get_contents ("/etc/timezone",
                                                             &resolved_identifier,
                                                             NULL, NULL)
#if defined(__sun) && defined(__SVR4)
                                        ||
                                        (resolved_identifier = zone_identifier_illumos ())
#endif
                                            ))
        sys_str_trim_end (resolved_identifier, ' ');
      else
        {
          /* Error */
          sys_assert (resolved_identifier == NULL);
          goto out;
        }
    }
  else
    {
      /* Resolve relative path */
      canonical_path = sys_path_canonicalize_filename (resolved_identifier, "/etc");
      sys_free (resolved_identifier);
      resolved_identifier = sys_steal_pointer (&canonical_path);
    }

  tzdir = sys_env_get ("TZDIR");
  if (tzdir == NULL)
    tzdir = zone_info_base_dir ();

  /* Strip the prefix and slashes if possible. */
  if (sys_str_startswith (resolved_identifier, tzdir))
    {
      prefix_len = strlen (tzdir);
      while (*(resolved_identifier + prefix_len) == '/')
        prefix_len++;
    }

  if (prefix_len > 0)
    memmove (resolved_identifier, resolved_identifier + prefix_len,
             strlen (resolved_identifier) - prefix_len + 1  /* nul terminator */);

  sys_assert (resolved_identifier != NULL);

out:
  sys_free (canonical_path);

  return resolved_identifier;
}

static SysBytes* zone_info_unix (const SysChar *identifier,
                const SysChar *resolved_identifier)
{
  SysChar *filename = NULL;
  SysMappedFile *file = NULL;
  SysBytes *zoneinfo = NULL;
  const SysChar *tzdir;

  tzdir = sys_env_get ("TZDIR");
  if (tzdir == NULL)
    tzdir = zone_info_base_dir ();

  /* identifier can be a relative or absolute path name;
     if relative, it is interpreted starting from /usr/share/zoneinfo
     while the POSIX standard says it should start with :,
     glibc allows both syntaxes, so we should too */
  if (identifier != NULL)
    {
      if (*identifier == ':')
        identifier ++;

      if (sys_path_is_absolute (identifier))
        filename = sys_strdup (identifier);
      else
        filename = sys_path_build_filename (tzdir, identifier, NULL);
    }
  else
    {
      if (resolved_identifier == NULL)
        goto out;

      filename = sys_strdup ("/etc/localtime");
    }

  file = sys_mapped_file_new (filename, false, NULL);
  if (file != NULL)
    {
      zoneinfo = sys_bytes_new_with_free_func (sys_mapped_file_get_contents (file),
                                             sys_mapped_file_get_length (file),
                                             (SysDestroyFunc)sys_mapped_file_unref,
                                             sys_mapped_file_ref (file));
      sys_mapped_file_unref (file);
    }

  sys_assert (resolved_identifier != NULL);

out:
  sys_free (filename);

  return zoneinfo;
}

static SysBool set_tz_name (SysChar **pos, SysChar *buffer, SysUInt size) {
  SysBool quoted = **pos == '<';
  SysChar *name_pos = *pos;
  SysUInt len;

  sys_assert (size != 0);

  if (quoted)
    {
      name_pos++;
      do
        ++(*pos);
      while (isalnum (**pos) || **pos == '-' || **pos == '+');
      if (**pos != '>')
        return false;
    }
  else
    while (isalpha (**pos))
      ++(*pos);

  /* Name should be three or more characters */
  /* FIXME: Should return false if the name is too long.
     This should simplify code later in this function.  */
  if (*pos - name_pos < 3)
    return false;

  memset (buffer, 0, size);
  /* name_pos isn't 0-terminated, so we have to limit the length expressly */
  len = (SysUInt) (*pos - name_pos) > size - 1 ? size - 1 : (SysUInt) (*pos - name_pos);
  strncpy (buffer, name_pos, len);
  *pos += quoted;
  return true;
}

static SysBool
parse_offset (SysChar **pos, SysInt32 *target)
{
  SysChar *buffer;
  SysChar *target_pos = *pos;
  SysBool ret;

  while (**pos == '+' || **pos == '-' || **pos == ':' ||
         (**pos >= '0' && '9' >= **pos))
    ++(*pos);

  buffer = sys_strndup (target_pos, *pos - target_pos);
  ret = parse_constant_offset (buffer, target, false);
  sys_free (buffer);

  return ret;
}

static SysUInt
create_ruleset_from_rule (TimeZoneRule **rules, TimeZoneRule *rule)
{
  *rules = sys_new0 (TimeZoneRule, 2);

  (*rules)[0].start_year = min_TZYEAR;
  (*rules)[1].start_year = max_TZYEAR;

  (*rules)[0].std_offset = -rule->std_offset;
  (*rules)[0].dlt_offset = -rule->dlt_offset;
  (*rules)[0].dlt_start  = rule->dlt_start;
  (*rules)[0].dlt_end = rule->dlt_end;
  strcpy ((*rules)[0].std_name, rule->std_name);
  strcpy ((*rules)[0].dlt_name, rule->dlt_name);
  return 2;
}

static SysBool
parse_identifier_boundary (SysChar **pos, TimeZoneDate *target)
{
  SysChar *buffer;
  SysChar *target_pos = *pos;
  SysBool ret;

  while (**pos != ',' && **pos != '\0')
    ++(*pos);
  buffer = sys_strndup (target_pos, *pos - target_pos);
  ret = parse_tz_boundary (buffer, target);
  sys_free (buffer);

  return ret;
}


static SysBool
parse_identifier_boundaries (SysChar **pos, TimeZoneRule *tzr)
{
  if (*(*pos)++ != ',')
    return false;

  /* Start date */
  if (!parse_identifier_boundary (pos, &(tzr->dlt_start)) || *(*pos)++ != ',')
    return false;

  /* End date */
  if (!parse_identifier_boundary (pos, &(tzr->dlt_end)))
    return false;
  return true;
}

/*
 * Creates an array of TimeZoneRule from a TZ environment variable
 * type of identifier.  Should free rules afterwards
 */
static SysUInt
rules_from_identifier (const SysChar   *identifier,
                       TimeZoneRule **rules)
{
  SysChar *pos;
  TimeZoneRule tzr;

  sys_assert (rules != NULL);

  *rules = NULL;

  if (!identifier)
    return 0;

  pos = (SysChar*)identifier;
  memset (&tzr, 0, sizeof (tzr));
  /* Standard offset */
  if (!(set_tz_name (&pos, tzr.std_name, SYS_TIME_ZONE_NAME_SIZE)) ||
      !parse_offset (&pos, &(tzr.std_offset)))
    return 0;

  if (*pos == 0)
    {
      return create_ruleset_from_rule (rules, &tzr);
    }

  /* Format 2 */
  if (!(set_tz_name (&pos, tzr.dlt_name, SYS_TIME_ZONE_NAME_SIZE)))
    return 0;
  parse_offset (&pos, &(tzr.dlt_offset));
  if (tzr.dlt_offset == 0) /* No daylight offset given, assume it's 1
                              hour earlier that standard */
    tzr.dlt_offset = tzr.std_offset - 3600;
  if (*pos == '\0')
#ifdef SYS_OS_WIN32
    /* Windows allows us to use the US DST boundaries if they're not given */
    {
      SysUInt i, rules_num = 0;

      /* Use US rules, Windows' default is Pacific Standard Time */
      if ((rules_num = rules_from_windows_time_zone ("Pacific Standard Time",
                                                     NULL,
                                                     rules)))
        {
          for (i = 0; i < rules_num - 1; i++)
            {
              (*rules)[i].std_offset = - tzr.std_offset;
              (*rules)[i].dlt_offset = - tzr.dlt_offset;
              strcpy ((*rules)[i].std_name, tzr.std_name);
              strcpy ((*rules)[i].dlt_name, tzr.dlt_name);
            }

          return rules_num;
        }
      else
        return 0;
    }
#else
  return 0;
#endif
  /* Start and end required (format 2) */
  if (!parse_identifier_boundaries (&pos, &tzr))
    return 0;

  return create_ruleset_from_rule (rules, &tzr);
}

static void
fill_transition_info_from_rule (TransitionInfo *info,
                                TimeZoneRule   *rule,
                                SysBool        is_dst)
{
  SysInt offset = is_dst ? rule->dlt_offset : rule->std_offset;
  SysChar *name = is_dst ? rule->dlt_name : rule->std_name;

  info->gmt_offset = offset;
  info->is_dst = is_dst;

  if (name)
    info->abbrev = sys_strdup (name);

  else
    info->abbrev = sys_strdup_printf ("%+03d%02d",
                                      (int) offset / 3600,
                                      (int) abs (offset / 60) % 60);
}

/* Offset is previous offset of local time. Returns 0 if month is 0 */
static SysInt64
boundary_for_year (TimeZoneDate *boundary,
                   SysInt          year,
                   SysInt32        offset)
{
  TimeZoneDate buffer;
  SysDate date;
  const SysUInt64 unix_epoch_start = 719163L;
  const SysUInt64 seconds_per_day = 86400L;

  if (!boundary->mon)
    return 0;
  buffer = *boundary;

  if (boundary->year == 0)
    {
      buffer.year = year;

      if (buffer.wday)
      {
        find_relative_date (&buffer);
      }
    }

  sys_assert (buffer.year == year);
  sys_date_clear (&date, 1);
  sys_date_set_dmy (&date, buffer.mday, buffer.mon, buffer.year);
  return ((sys_date_get_julian (&date) - unix_epoch_start) * seconds_per_day +
          buffer.offset - offset);
}



static void
init_zone_from_rules (SysTimeZone    *gtz,
                      TimeZoneRule *rules,
                      SysUInt         rules_num,
                      SysChar        *identifier  /* (transfer full) */)
{
  SysUInt type_count = 0, trans_count = 0, info_index = 0;
  SysUInt ri; /* rule index */
  SysBool skip_first_std_trans = true;
  SysInt32 last_offset;

  type_count = 0;
  trans_count = 0;

  /* Last rule only contains max year */
  for (ri = 0; ri < rules_num - 1; ri++)
    {
      if (rules[ri].dlt_start.mon || rules[ri].dlt_end.mon)
        {
          SysUInt rulespan = (rules[ri + 1].start_year - rules[ri].start_year);
          SysUInt transitions = rules[ri].dlt_start.mon > 0 ? 1 : 0;
          transitions += rules[ri].dlt_end.mon > 0 ? 1 : 0;
          type_count += rules[ri].dlt_start.mon > 0 ? 2 : 1;
          trans_count += transitions * rulespan;
        }
      else
        type_count++;
    }

  gtz->name = sys_steal_pointer (&identifier);
  gtz->t_info = sys_array_sized_new (false, true, sizeof (TransitionInfo), type_count);
  gtz->transitions = sys_array_sized_new (false, true, sizeof (Transition), trans_count);

  last_offset = rules[0].std_offset;

  for (ri = 0; ri < rules_num - 1; ri++)
    {
      if ((rules[ri].std_offset || rules[ri].dlt_offset) &&
          rules[ri].dlt_start.mon == 0 && rules[ri].dlt_end.mon == 0)
        {
          TransitionInfo std_info;
          /* Standard */
          fill_transition_info_from_rule (&std_info, &(rules[ri]), false);
          sys_array_append_val (gtz->t_info, std_info);

          if (ri > 0 &&
              ((rules[ri - 1].dlt_start.mon > 12 &&
                rules[ri - 1].dlt_start.wday > rules[ri - 1].dlt_end.wday) ||
                rules[ri - 1].dlt_start.mon > rules[ri - 1].dlt_end.mon))
            {
              /* The previous rule was a southern hemisphere rule that
                 starts the year with DST, so we need to add a
                 transition to return to standard time */
              SysUInt year = rules[ri].start_year;
              SysInt64 std_time =  boundary_for_year (&rules[ri].dlt_end,
                                                    year, last_offset);
              Transition std_trans = {std_time, info_index};
              sys_array_append_val (gtz->transitions, std_trans);

            }
          last_offset = rules[ri].std_offset;
          ++info_index;
          skip_first_std_trans = true;
         }
      else
        {
          const SysUInt start_year = rules[ri].start_year;
          const SysUInt end_year = rules[ri + 1].start_year;
          SysBool dlt_first;
          SysUInt year;
          TransitionInfo std_info, dlt_info;
          if (rules[ri].dlt_start.mon > 12)
            dlt_first = rules[ri].dlt_start.wday > rules[ri].dlt_end.wday;
          else
            dlt_first = rules[ri].dlt_start.mon > rules[ri].dlt_end.mon;
          /* Standard rules are always even, because before the first
             transition is always standard time, and 0 is even. */
          fill_transition_info_from_rule (&std_info, &(rules[ri]), false);
          fill_transition_info_from_rule (&dlt_info, &(rules[ri]), true);

          sys_array_append_val (gtz->t_info, std_info);
          sys_array_append_val (gtz->t_info, dlt_info);

          /* Transition dates. We hope that a year which ends daylight
             time in a southern-hemisphere country (i.e., one that
             begins the year in daylight time) will include a rule
             which has only a dlt_end. */
          for (year = start_year; year < end_year; year++)
            {
              SysInt32 dlt_offset = (dlt_first ? last_offset :
                                   rules[ri].dlt_offset);
              SysInt32 std_offset = (dlt_first ? rules[ri].std_offset :
                                   last_offset);
              /* NB: boundary_for_year returns 0 if mon == 0 */
              SysInt64 std_time =  boundary_for_year (&rules[ri].dlt_end,
                                                    year, dlt_offset);
              SysInt64 dlt_time = boundary_for_year (&rules[ri].dlt_start,
                                                   year, std_offset);
              Transition std_trans = {std_time, info_index};
              Transition dlt_trans = {dlt_time, info_index + 1};
              last_offset = (dlt_first ? rules[ri].dlt_offset :
                             rules[ri].std_offset);
              if (dlt_first)
                {
                  if (skip_first_std_trans)
                    skip_first_std_trans = false;
                  else if (std_time)
                    sys_array_append_val (gtz->transitions, std_trans);
                  if (dlt_time)
                    sys_array_append_val (gtz->transitions, dlt_trans);
                }
              else
                {
                  if (dlt_time)
                    sys_array_append_val (gtz->transitions, dlt_trans);
                  if (std_time)
                    sys_array_append_val (gtz->transitions, std_trans);
                }
            }

          info_index += 2;
        }
    }
  if (ri > 0 &&
      ((rules[ri - 1].dlt_start.mon > 12 &&
        rules[ri - 1].dlt_start.wday > rules[ri - 1].dlt_end.wday) ||
       rules[ri - 1].dlt_start.mon > rules[ri - 1].dlt_end.mon))
    {
      /* The previous rule was a southern hemisphere rule that
         starts the year with DST, so we need to add a
         transition to return to standard time */
      TransitionInfo info;
      SysUInt year = rules[ri].start_year;
      Transition trans;
      fill_transition_info_from_rule (&info, &(rules[ri - 1]), false);
      sys_array_append_val (gtz->t_info, info);
      trans.time = boundary_for_year (&rules[ri - 1].dlt_end,
                                      year, last_offset);
      trans.info_index = info_index;
      sys_array_append_val (gtz->transitions, trans);
     }
}


#ifdef SYS_OS_UNIX
static SysTimeZone *
parse_footertz (const SysChar *footer, size_t footerlen)
{
  SysChar *tzstring = sys_strndup (footer + 1, footerlen - 2);
  SysTimeZone *footertz = NULL;

  /* FIXME: The allocation for tzstring could be avoided by
     passing a SysSize identifier_len argument to rules_from_identifier
     and changing the code in that function to stop assuming that
     identifier is nul-terminated.  */
  TimeZoneRule *rules;
  SysUInt rules_num = rules_from_identifier (tzstring, &rules);

  sys_free (tzstring);
  if (rules_num > 1)
    {
      footertz = sys_slice_new0 (SysTimeZone);
      init_zone_from_rules (footertz, rules, rules_num, NULL);
      footertz->ref_count++;
    }
  sys_free (rules);
  return footertz;
}
#endif

static void
init_zone_from_iana_info (SysTimeZone *gtz,
                          SysBytes    *zoneinfo,
                          SysChar     *identifier  /* (transfer full) */)
{
  SysSize size;
  SysUInt index;
  SysUInt32 time_count, type_count;
  SysUInt8 *tz_transitions, *tz_type_index, *tz_ttinfo;
  SysUInt8 *tz_abbrs;
  SysSize timesize = sizeof (SysInt32);
  const SysPointer header_data = sys_bytes_get_data (zoneinfo, &size);
  const SysChar *data = header_data;
  const struct tzhead *header = header_data;
  SysTimeZone *footertz = NULL;
  SysUInt extra_time_count = 0, extra_type_count = 0;
  SysInt64 last_explicit_transition_time = 0;

  sys_return_if_fail (size >= sizeof (struct tzhead) &&
                    memcmp (header, "TZif", 4) == 0);

  /* FIXME: Handle invalid TZif files better (Issue#1088).  */

  if (header->tzh_version >= '2')
      {
        /* Skip ahead to the newer 64-bit data if it's available. */
        header = (const struct tzhead *)
          (((const SysChar *) (header + 1)) +
           SysUInt32_from_be(header->tzh_ttisgmtcnt) +
           SysUInt32_from_be(header->tzh_ttisstdcnt) +
           8 * SysUInt32_from_be(header->tzh_leapcnt) +
           5 * SysUInt32_from_be(header->tzh_timecnt) +
           6 * SysUInt32_from_be(header->tzh_typecnt) +
           SysUInt32_from_be(header->tzh_charcnt));
        timesize = sizeof (SysInt64);
      }
  time_count = SysUInt32_from_be(header->tzh_timecnt);
  type_count = SysUInt32_from_be(header->tzh_typecnt);

  if (header->tzh_version >= '2')
    {
      const SysChar *footer = (((const SysChar *) (header + 1))
                             + SysUInt32_from_be(header->tzh_ttisgmtcnt)
                             + SysUInt32_from_be(header->tzh_ttisstdcnt)
                             + 12 * SysUInt32_from_be(header->tzh_leapcnt)
                             + 9 * time_count
                             + 6 * type_count
                             + SysUInt32_from_be(header->tzh_charcnt));
      const SysChar *footerlast;
      size_t footerlen;
      sys_return_if_fail (footer <= data + size - 2 && footer[0] == '\n');
      footerlast = memchr (footer + 1, '\n', data + size - (footer + 1));
      sys_return_if_fail (footerlast);
      footerlen = footerlast + 1 - footer;
      if (footerlen != 2)
        {
          footertz = parse_footertz (footer, footerlen);
          sys_return_if_fail (footertz);
          extra_type_count = footertz->t_info->len;
          extra_time_count = footertz->transitions->len;
        }
    }

  tz_transitions = ((SysUInt8 *) (header) + sizeof (*header));
  tz_type_index = tz_transitions + timesize * time_count;
  tz_ttinfo = tz_type_index + time_count;
  tz_abbrs = tz_ttinfo + sizeof (struct ttinfo) * type_count;

  gtz->name = sys_steal_pointer (&identifier);
  gtz->t_info = sys_array_sized_new (false, true, sizeof (TransitionInfo),
                                   type_count + extra_type_count);
  gtz->transitions = sys_array_sized_new (false, true, sizeof (Transition),
                                        time_count + extra_time_count);

  for (index = 0; index < type_count; index++)
    {
      TransitionInfo t_info;
      struct ttinfo info = ((struct ttinfo*)tz_ttinfo)[index];
      t_info.gmt_offset = SysInt32_from_be (info.tt_gmtoff);
      t_info.is_dst = info.tt_isdst ? true : false;
      t_info.abbrev = sys_strdup ((SysChar *) &tz_abbrs[info.tt_abbrind]);
      sys_array_append_val (gtz->t_info, t_info);
    }

  for (index = 0; index < time_count; index++)
    {
      Transition trans;
      if (header->tzh_version >= '2')
        trans.time = SysInt64_from_be (((SysInt64_be*)tz_transitions)[index]);
      else
        trans.time = SysInt32_from_be (((SysInt32_be*)tz_transitions)[index]);
      last_explicit_transition_time = trans.time;
      trans.info_index = tz_type_index[index];
      sys_assert (trans.info_index >= 0);
      sys_assert ((SysUInt) trans.info_index < gtz->t_info->len);
      sys_array_append_val (gtz->transitions, trans);
    }

  if (footertz)
    {
      /* Append footer time types.  Don't bother to coalesce
         duplicates with existing time types.  */
      for (index = 0; index < extra_type_count; index++)
        {
          TransitionInfo t_info;
          TransitionInfo *footer_t_info
            = &sys_array_index (footertz->t_info, TransitionInfo, index);
          t_info.gmt_offset = footer_t_info->gmt_offset;
          t_info.is_dst = footer_t_info->is_dst;
          t_info.abbrev = sys_steal_pointer (&footer_t_info->abbrev);
          sys_array_append_val (gtz->t_info, t_info);
        }

      /* Append footer transitions that follow the last explicit
         transition.  */
      for (index = 0; index < extra_time_count; index++)
        {
          Transition *footer_transition
            = &sys_array_index (footertz->transitions, Transition, index);
          if (time_count <= 0
              || last_explicit_transition_time < footer_transition->time)
            {
              Transition trans;
              trans.time = footer_transition->time;
              trans.info_index = type_count + footer_transition->info_index;
              sys_array_append_val (gtz->transitions, trans);
            }
        }

      sys_time_zone_unref (footertz);
    }
}

#elif defined (SYS_OS_WIN32)

static void
copy_windows_systemtime (SYSTEMTIME *s_time, TimeZoneDate *tzdate)
{
  tzdate->offset
    = s_time->wHour * 3600 + s_time->wMinute * 60 + s_time->wSecond;
  tzdate->mon = s_time->wMonth;
  tzdate->year = s_time->wYear;
  tzdate->wday = s_time->wDayOfWeek ? s_time->wDayOfWeek : 7;

  if (s_time->wYear)
    {
      tzdate->mday = s_time->wDay;
      tzdate->wday = 0;
    }
  else
    tzdate->week = s_time->wDay;
}

/* UTC = local time + bias while local time = UTC + offset */
static SysBool
rule_from_windows_time_zone_info (TimeZoneRule *rule,
                                  TIME_ZONE_INFORMATION *tzi)
{
  SysChar *std_name, *dlt_name;

  std_name =  sys_wchar_to_mbyte ((SysUniChar2 *)tzi->StandardName, NULL);
  if (std_name == NULL)
    return false;

  dlt_name =  sys_wchar_to_mbyte ((SysUniChar2 *)tzi->DaylightName, NULL);
  if (dlt_name == NULL)
    {
      sys_free (std_name);
      return false;
    }

  /* Set offset */
  if (tzi->StandardDate.wMonth)
    {
      rule->std_offset = -(tzi->Bias + tzi->StandardBias) * 60;
      rule->dlt_offset = -(tzi->Bias + tzi->DaylightBias) * 60;
      copy_windows_systemtime (&(tzi->DaylightDate), &(rule->dlt_start));

      copy_windows_systemtime (&(tzi->StandardDate), &(rule->dlt_end));
    }

  else
    {
      rule->std_offset = -tzi->Bias * 60;
      rule->dlt_start.mon = 0;
    }
  sys_strncpy (rule->std_name, std_name, SYS_TIME_ZONE_NAME_SIZE - 1);
  sys_strncpy (rule->dlt_name, dlt_name, SYS_TIME_ZONE_NAME_SIZE - 1);

  sys_free (std_name);
  sys_free (dlt_name);

  return true;
}

static SysChar*
windows_default_tzname (void)
{
  const SysUniChar2 *subkey =
    L"SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation";
  HKEY key;
  SysChar *key_name = NULL;
  SysUniChar2 *key_name_w = NULL;
  if (RegOpenKeyExW (HKEY_LOCAL_MACHINE, subkey, 0,
                     KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
    {
      DWORD size = 0;
      if (RegQueryValueExW (key, L"TimeZoneKeyName", NULL, NULL,
                            NULL, &size) == ERROR_SUCCESS)
        {
          key_name_w = sys_malloc ((SysInt)size);

          if (key_name_w == NULL ||
              RegQueryValueExW (key, L"TimeZoneKeyName", NULL, NULL,
                                (LPBYTE)key_name_w, &size) != ERROR_SUCCESS)
            {
              sys_free (key_name_w);
              key_name = NULL;
            }
          else
            key_name = sys_utf16_to_utf8 (key_name_w, NULL);
        }
      RegCloseKey (key);
    }
  return key_name;
}

typedef   struct
{
  LONG Bias;
  LONG StandardBias;
  LONG DaylightBias;
  SYSTEMTIME StandardDate;
  SYSTEMTIME DaylightDate;
} RegTZI;

static void
system_time_copy (SYSTEMTIME *orig, SYSTEMTIME *target)
{
  sys_return_if_fail (orig != NULL);
  sys_return_if_fail (target != NULL);

  target->wYear = orig->wYear;
  target->wMonth = orig->wMonth;
  target->wDayOfWeek = orig->wDayOfWeek;
  target->wDay = orig->wDay;
  target->wHour = orig->wHour;
  target->wMinute = orig->wMinute;
  target->wSecond = orig->wSecond;
  target->wMilliseconds = orig->wMilliseconds;
}

static void
register_tzi_to_tzi (RegTZI *reg, TIME_ZONE_INFORMATION *tzi)
{
  sys_return_if_fail (reg != NULL);
  sys_return_if_fail (tzi != NULL);
  tzi->Bias = reg->Bias;
  system_time_copy (&(reg->StandardDate), &(tzi->StandardDate));
  tzi->StandardBias = reg->StandardBias;
  system_time_copy (&(reg->DaylightDate), &(tzi->DaylightDate));
  tzi->DaylightBias = reg->DaylightBias;
}

static SysUInt
rules_from_windows_time_zone (const SysChar   *identifier,
                              const SysChar   *resolved_identifier,
                              TimeZoneRule **rules)
{
  HKEY key;
  SysChar *subkey = NULL;
  SysChar *subkey_dynamic = NULL;
  const SysChar *key_name;
  SysChar resys_key[1024] = 
    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones\\";
  TIME_ZONE_INFORMATION tzi;
  DWORD size;
  SysUInt rules_num = 0;
  RegTZI regtzi = { 0 }, regtzi_prev;
  WCHAR winsyspath[MAX_PATH];
  SysUniChar2 *subkey_w, *subkey_dynamic_w;

  subkey_dynamic_w = NULL;

  if (GetSystemDirectoryW (winsyspath, MAX_PATH) == 0)
    return 0;

  sys_assert (rules != NULL);

  *rules = NULL;
  key_name = NULL;

  if (!identifier)
    key_name = resolved_identifier;
  else
    key_name = identifier;

  if (!key_name)
    return 0;

  subkey = sys_strlcat (resys_key, sizeof(resys_key), key_name);
  subkey_w = sys_utf8_to_utf16 (subkey, NULL);
  if (subkey_w == NULL)
    goto utf16_conv_failed;

  subkey_dynamic = sys_strlcat (subkey, sizeof(resys_key), "\\Dynamic DST");
  subkey_dynamic_w = sys_utf8_to_utf16 (subkey_dynamic, NULL);
  if (subkey_dynamic_w == NULL)
    goto utf16_conv_failed;

  if (RegOpenKeyExW (HKEY_LOCAL_MACHINE, subkey_w, 0,
                     KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
      goto utf16_conv_failed;

  size = sizeof tzi.StandardName;

  /* use RegLoadMUIStringW() to query MUI_Std from the registry if possible, otherwise
     fallback to querying Std */
  if (RegLoadMUIStringW (key, L"MUI_Std", tzi.StandardName,
                         size, &size, 0, winsyspath) != ERROR_SUCCESS)
    {
      size = sizeof tzi.StandardName;
      if (RegQueryValueExW (key, L"Std", NULL, NULL,
                            (LPBYTE)&(tzi.StandardName), &size) != ERROR_SUCCESS)
        goto registry_failed;
    }

  size = sizeof tzi.DaylightName;

  /* use RegLoadMUIStringW() to query MUI_Dlt from the registry if possible, otherwise
     fallback to querying Dlt */
  if (RegLoadMUIStringW (key, L"MUI_Dlt", tzi.DaylightName,
                         size, &size, 0, winsyspath) != ERROR_SUCCESS)
    {
      size = sizeof tzi.DaylightName;
      if (RegQueryValueExW (key, L"Dlt", NULL, NULL,
                            (LPBYTE)&(tzi.DaylightName), &size) != ERROR_SUCCESS)
        goto registry_failed;
    }

  RegCloseKey (key);
  if (RegOpenKeyExW (HKEY_LOCAL_MACHINE, subkey_dynamic_w, 0,
                     KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
    {
      DWORD i, first, last, year;
      wchar_t s[12];

      size = sizeof first;
      if (RegQueryValueExW (key, L"FirstEntry", NULL, NULL,
                            (LPBYTE) &first, &size) != ERROR_SUCCESS)
        goto registry_failed;

      size = sizeof last;
      if (RegQueryValueExW (key, L"LastEntry", NULL, NULL,
                            (LPBYTE) &last, &size) != ERROR_SUCCESS)
        goto registry_failed;

      rules_num = last - first + 2;
      *rules = sys_new0 (TimeZoneRule, rules_num);

      for (year = first, i = 0; *rules != NULL && year <= last; year++)
        {
          SysBool failed = false;
          swprintf_s (s, 11, L"%d", year);

          if (!failed)
            {
              size = sizeof regtzi;
              if (RegQueryValueExW (key, s, NULL, NULL,
                                    (LPBYTE) &regtzi, &size) != ERROR_SUCCESS)
                failed = true;
            }

          if (failed)
            {
              sys_free (*rules);
              *rules = NULL;
              break;
            }

          if (year > first && memcmp (&regtzi_prev, &regtzi, sizeof regtzi) == 0)
              continue;
          else
            memcpy (&regtzi_prev, &regtzi, sizeof regtzi);

          register_tzi_to_tzi (&regtzi, &tzi);

          if (!rule_from_windows_time_zone_info (&(*rules)[i], &tzi))
            {
              sys_free (*rules);
              *rules = NULL;
              break;
            }

          (*rules)[i++].start_year = year;
        }

      rules_num = i + 1;

registry_failed:
      RegCloseKey (key);
    }
  else if (RegOpenKeyExW (HKEY_LOCAL_MACHINE, subkey_w, 0,
                          KEY_QUERY_VALUE, &key) == ERROR_SUCCESS)
    {
      size = sizeof regtzi;
      if (RegQueryValueExW (key, L"TZI", NULL, NULL,
                            (LPBYTE) &regtzi, &size) == ERROR_SUCCESS)
        {
          rules_num = 2;
          *rules = sys_new0 (TimeZoneRule, 2);
          register_tzi_to_tzi (&regtzi, &tzi);

          if (!rule_from_windows_time_zone_info (&(*rules)[0], &tzi))
            {
              sys_free (*rules);
              *rules = NULL;
            }
        }

      RegCloseKey (key);
    }

utf16_conv_failed:
  sys_free (subkey_dynamic_w);
  sys_free (subkey_dynamic);
  sys_free (subkey_w);
  sys_free (subkey);

  if (*rules)
    {
      (*rules)[0].start_year = min_TZYEAR;
      if ((*rules)[rules_num - 2].start_year < max_TZYEAR)
        (*rules)[rules_num - 1].start_year = max_TZYEAR;
      else
        (*rules)[rules_num - 1].start_year = (*rules)[rules_num - 2].start_year + 1;

      return rules_num;
    }

  return 0;
}

#endif

static void find_relative_date (TimeZoneDate *buffer) {
  SysUInt wday;
  SysDate date;
  sys_date_clear (&date, 1);
  wday = buffer->wday;

  /* Get last day if last is needed, first day otherwise */
  if (buffer->mon == 13 || buffer->mon == 14) /* Julian Date */
    {
      sys_date_set_dmy (&date, 1, 1, buffer->year);
      if (wday >= 59 && buffer->mon == 13 && sys_date_is_leap_year (buffer->year))
        sys_date_add_days (&date, wday);
      else
        sys_date_add_days (&date, wday - 1);
      buffer->mon = (int) sys_date_get_month (&date);
      buffer->mday = (int) sys_date_get_day (&date);
      buffer->wday = 0;
    }
  else /* M.W.D */
    {
      SysUInt days;
      SysUInt days_in_month = sys_date_get_days_in_month (buffer->mon, buffer->year);
      SysDateWeekday first_wday;

      sys_date_set_dmy (&date, 1, buffer->mon, buffer->year);
      first_wday = sys_date_get_weekday (&date);

      if ((SysUInt) first_wday > wday)
        ++(buffer->week);
      /* week is 1 <= w <= 5, we need 0-based */
      days = 7 * (buffer->week - 1) + wday - first_wday;

      /* "days" is a 0-based offset from the 1st of the month.
       * Adding days == days_in_month would bring us into the next month,
       * hence the ">=" instead of just ">".
       */
      while (days >= days_in_month)
        days -= 7;

      sys_date_add_days (&date, days);

      buffer->mday = sys_date_get_day (&date);
    }
}

/*
 * parses date[/time] for parsing TZ environment variable
 *
 * date is either Mm.w.d, Jn or N
 * - m is 1 to 12
 * - w is 1 to 5
 * - d is 0 to 6
 * - n is 1 to 365
 * - N is 0 to 365
 *
 * time is either h or hh[[:]mm[[[:]ss]]]
 *  - h[h] is 0 to 24
 *  - mm is 00 to 59
 *  - ss is 00 to 59
 */
static SysBool
parse_mwd_boundary (SysChar **pos, TimeZoneDate *boundary)
{
  SysInt month, week, day;

  if (**pos == '\0' || **pos < '0' || '9' < **pos)
    return false;

  month = *(*pos)++ - '0';

  if ((month == 1 && **pos >= '0' && '2' >= **pos) ||
      (month == 0 && **pos >= '0' && '9' >= **pos))
    {
      month *= 10;
      month += *(*pos)++ - '0';
    }

  if (*(*pos)++ != '.' || month == 0)
    return false;

  if (**pos == '\0' || **pos < '1' || '5' < **pos)
    return false;

  week = *(*pos)++ - '0';

  if (*(*pos)++ != '.')
    return false;

  if (**pos == '\0' || **pos < '0' || '6' < **pos)
    return false;

  day = *(*pos)++ - '0';

  if (!day)
    day += 7;

  boundary->year = 0;
  boundary->mon = month;
  boundary->week = week;
  boundary->wday = day;
  return true;
}

/*
 * This parses two slightly different ways of specifying
 * the Julian day:
 *
 * - ignore_leap == true
 *
 *   Jn   This specifies the Julian day with n between 1 and 365. Leap days
 *        are not counted. In this format, February 29 can't be represented;
 *        February 28 is day 59, and March 1 is always day 60.
 *
 * - ignore_leap == false
 *
 *   n   This specifies the zero-based Julian day with n between 0 and 365.
 *       February 29 is counted in leap years.
 */
static SysBool
parse_julian_boundary (SysChar** pos, TimeZoneDate *boundary,
                       SysBool ignore_leap)
{
  SysInt day = 0;
  SysDate date;

  while (**pos >= '0' && '9' >= **pos)
    {
      day *= 10;
      day += *(*pos)++ - '0';
    }

  if (ignore_leap)
    {
      if (day < 1 || 365 < day)
        return false;
      if (day >= 59)
        day++;
    }
  else
    {
      if (day < 0 || 365 < day)
        return false;
      /* SysDate wants day in range 1->366 */
      day++;
    }

  sys_date_clear (&date, 1);
  sys_date_set_julian (&date, day);
  boundary->year = 0;
  boundary->mon = (int) sys_date_get_month (&date);
  boundary->mday = (int) sys_date_get_day (&date);
  boundary->wday = 0;

  return true;
}

static SysBool
parse_tz_boundary (const SysChar  *identifier,
                   TimeZoneDate *boundary)
{
  SysChar *pos;

  pos = (SysChar*)identifier;
  /* Month-week-weekday */
  if (*pos == 'M')
    {
      ++pos;
      if (!parse_mwd_boundary (&pos, boundary))
        return false;
    }
  /* Julian date which ignores Feb 29 in leap years */
  else if (*pos == 'J')
    {
      ++pos;
      if (!parse_julian_boundary (&pos, boundary, true))
        return false ;
    }
  /* Julian date which counts Feb 29 in leap years */
  else if (*pos >= '0' && '9' >= *pos)
    {
      if (!parse_julian_boundary (&pos, boundary, false))
        return false;
    }
  else
    return false;

  /* Time */

  if (*pos == '/')
    return parse_constant_offset (pos + 1, &boundary->offset, true);
  else
    {
      boundary->offset = 2 * 60 * 60;
      return *pos == '\0';
    }
}

/* Construction {{{1 */
/**
 * sys_time_zone_new:
 * @identifier: (nullable): a timezone identifier
 *
 * A version of sys_time_zone_new_identifier() which returns the UTC time zone
 * if @identifier could not be parsed or loaded.
 *
 * If you need to check whether @identifier was loaded successfully, use
 * sys_time_zone_new_identifier().
 *
 * Returns: (transfer full) (not nullable): the requested timezone
 * Deprecated: 2.68: Use sys_time_zone_new_identifier() instead, as it provides
 *     error reporting. Change your code to handle a potentially %NULL return
 *     value.
 *
 * Since: 2.26
 **/
SysTimeZone *
sys_time_zone_new (const SysChar *identifier)
{
  SysTimeZone *tz = sys_time_zone_new_identifier (identifier);

  /* Always fall back to UTC. */
  if (tz == NULL)
    tz = sys_time_zone_new_utc ();

  sys_assert (tz != NULL);

  return sys_steal_pointer (&tz);
}

/**
 * sys_time_zone_new_identifier:
 * @identifier: (nullable): a timezone identifier
 *
 * Creates a #SysTimeZone corresponding to @identifier. If @identifier cannot be
 * parsed or loaded, %NULL is returned.
 *
 * @identifier can either be an RFC3339/ISO 8601 time offset or
 * something that would pass as a valid value for the `TZ` environment
 * variable (including %NULL).
 *
 * In Windows, @identifier can also be the unlocalized name of a time
 * zone for standard time, for example "Pacific Standard Time".
 *
 * Valid RFC3339 time offsets are `"Z"` (for UTC) or
 * `"±hh:mm"`.  ISO 8601 additionally specifies
 * `"±hhmm"` and `"±hh"`.  Offsets are
 * time values to be added to Coordinated Universal Time (UTC) to get
 * the local time.
 *
 * In UNIX, the `TZ` environment variable typically corresponds
 * to the name of a file in the zoneinfo database, an absolute path to a file
 * somewhere else, or a string in
 * "std offset [dst [offset],start[/time],end[/time]]" (POSIX) format.
 * There  are  no spaces in the specification. The name of standard
 * and daylight savings time zone must be three or more alphabetic
 * characters. Offsets are time values to be added to local time to
 * get Coordinated Universal Time (UTC) and should be
 * `"[±]hh[[:]mm[:ss]]"`.  Dates are either
 * `"Jn"` (Julian day with n between 1 and 365, leap
 * years not counted), `"n"` (zero-based Julian day
 * with n between 0 and 365) or `"Mm.w.d"` (day d
 * (0 <= d <= 6) of week w (1 <= w <= 5) of month m (1 <= m <= 12), day
 * 0 is a Sunday).  Times are in local wall clock time, the default is
 * 02:00:00.
 *
 * In Windows, the "tzn[+|–]hh[:mm[:ss]][dzn]" format is used, but also
 * accepts POSIX format.  The Windows format uses US rules for all time
 * zones; daylight savings time is 60 minutes behind the standard time
 * with date and time of change taken from Pacific Standard Time.
 * Offsets are time values to be added to the local time to get
 * Coordinated Universal Time (UTC).
 *
 * sys_time_zone_new_local() calls this function with the value of the
 * `TZ` environment variable. This function itself is independent of
 * the value of `TZ`, but if @identifier is %NULL then `/etc/localtime`
 * will be consulted to discover the correct time zone on UNIX and the
 * registry will be consulted or GetTimeZoneInformation() will be used
 * to get the local time zone on Windows.
 *
 * If intervals are not available, only time zone rules from `TZ`
 * environment variable or other means, then they will be computed
 * from year 1900 to 2037.  If the maximum year for the rules is
 * available and it is greater than 2037, then it will followed
 * instead.
 *
 * See
 * [RFC3339 §5.6](http://tools.ietf.org/html/rfc3339#section-5.6)
 * for a precise definition of valid RFC3339 time offsets
 * (the `time-offset` expansion) and ISO 8601 for the
 * full list of valid time offsets.  See
 * [The GNU C Library manual](http://www.gnu.org/s/libc/manual/html_node/TZ-Variable.html)
 * for an explanation of the possible
 * values of the `TZ` environment variable. See
 * [Microsoft Time Zone Index Values](http://msdn.microsoft.com/en-us/library/ms912391%28v=winembedded.11%29.aspx)
 * for the list of time zones on Windows.
 *
 * You should release the return value by calling sys_time_zone_unref()
 * when you are done with it.
 *
 * Returns: (transfer full) (nullable): the requested timezone, or %NULL on
 *     failure
 * Since: 2.68
 */
SysTimeZone *
sys_time_zone_new_identifier (const SysChar *identifier)
{
  SysTimeZone *tz = NULL;
  TimeZoneRule *rules;
  SysInt rules_num;
  SysChar *resolved_identifier = NULL;

  if (identifier)
    {
      SYS_LOCK (time_zones);
      if (time_zones == NULL)
        time_zones = sys_hash_table_new (sys_str_hash, (SysEqualFunc)sys_str_equal);

      tz = sys_hash_table_lookup (time_zones, (const SysPointer)identifier);
      if (tz)
        {
          sys_atomic_int_inc (&tz->ref_count);
          SYS_UNLOCK (time_zones);
          return tz;
        }
      else
        resolved_identifier = sys_strdup (identifier);
    }
  else
    {
      SYS_LOCK (tz_default);
#ifdef SYS_OS_UNIX
      resolved_identifier = zone_identifier_unix ();
#elif defined (SYS_OS_WIN32)
      resolved_identifier = windows_default_tzname ();
#endif
      if (tz_default)
        {
          /* Flush default if changed. If the identifier couldn’t be resolved,
           * we’re going to fall back to UTC eventually, so don’t clear out the
           * cache if it’s already UTC. */
          if (!(resolved_identifier == NULL && sys_str_equal (tz_default->name, "UTC")) &&
              sys_str_equal (tz_default->name, resolved_identifier))
            {
              sys_clear_pointer (&tz_default, sys_time_zone_unref);
            }
          else
            {
              tz = sys_time_zone_ref (tz_default);
              SYS_UNLOCK (tz_default);

              sys_free (resolved_identifier);
              return tz;
            }
        }
    }

  tz = sys_slice_new0 (SysTimeZone);
  tz->ref_count = 0;

  zone_for_constant_offset (tz, identifier);

  if (tz->t_info == NULL &&
      (rules_num = rules_from_identifier (identifier, &rules)))
    {
      init_zone_from_rules (tz, rules, rules_num, sys_steal_pointer (&resolved_identifier));
      sys_free (rules);
    }

  if (tz->t_info == NULL)
    {
#ifdef SYS_OS_UNIX
      SysBytes *zoneinfo = zone_info_unix (identifier, resolved_identifier);
      if (zoneinfo != NULL)
        {
          init_zone_from_iana_info (tz, zoneinfo, sys_steal_pointer (&resolved_identifier));
          sys_bytes_unref (zoneinfo);
        }
#elif defined (SYS_OS_WIN32)
      if ((rules_num = rules_from_windows_time_zone (identifier,
                                                     resolved_identifier,
                                                     &rules)))
        {
          init_zone_from_rules (tz, rules, rules_num, sys_steal_pointer (&resolved_identifier));
          sys_free (rules);
        }
#endif
    }

#if defined (SYS_OS_WIN32)
  if (tz->t_info == NULL)
    {
      if (identifier == NULL)
        {
          TIME_ZONE_INFORMATION tzi;

          if (GetTimeZoneInformation (&tzi) != TIME_ZONE_ID_INVALID)
            {
              rules = sys_new0 (TimeZoneRule, 2);

              if (rule_from_windows_time_zone_info (&rules[0], &tzi))
                {
                  memset (rules[0].std_name, 0, SYS_TIME_ZONE_NAME_SIZE);
                  memset (rules[0].dlt_name, 0, SYS_TIME_ZONE_NAME_SIZE);

                  rules[0].start_year = min_TZYEAR;
                  rules[1].start_year = max_TZYEAR;

                  init_zone_from_rules (tz, rules, 2, sys_steal_pointer (&resolved_identifier));
                }

              sys_free (rules);
            }
        }
    }
#endif

  sys_free (resolved_identifier);

  /* Failed to load the timezone. */
  if (tz->t_info == NULL)
    {
      sys_slice_free (SysTimeZone, tz);

      if (identifier)
        SYS_UNLOCK (time_zones);
      else
        SYS_UNLOCK (tz_default);

      return NULL;
    }

  sys_assert (tz->name != NULL);
  sys_assert (tz->t_info != NULL);

  if (identifier)
    sys_hash_table_insert (time_zones, tz->name, tz);
  else if (tz->name)
    {
      /* Caching reference */
      sys_atomic_int_inc (&tz->ref_count);
      tz_default = tz;
    }

  sys_atomic_int_inc (&tz->ref_count);

  if (identifier)
    SYS_UNLOCK (time_zones);
  else
    SYS_UNLOCK (tz_default);

  return tz;
}

/**
 * sys_time_zone_new_utc:
 *
 * Creates a #SysTimeZone corresponding to UTC.
 *
 * This is equivalent to calling sys_time_zone_new() with a value like
 * "Z", "UTC", "+00", etc.
 *
 * You should release the return value by calling sys_time_zone_unref()
 * when you are done with it.
 *
 * Returns: the universal timezone
 *
 * Since: 2.26
 **/
SysTimeZone * sys_time_zone_new_utc (void)
{
  static SysTimeZone *utc = NULL;
  static SysSize initialised;

  if (sys_once_init_enter (&initialised))
    {
      utc = sys_time_zone_new_identifier ("UTC");
      sys_assert (utc != NULL);
      sys_once_init_leave (&initialised, true);
    }

  return sys_time_zone_ref (utc);
}

/**
 * sys_time_zone_new_local:
 *
 * Creates a #SysTimeZone corresponding to local time.  The local time
 * zone may change between invocations to this function; for example,
 * if the system administrator changes it.
 *
 * This is equivalent to calling sys_time_zone_new() with the value of
 * the `TZ` environment variable (including the possibility of %NULL).
 *
 * You should release the return value by calling sys_time_zone_unref()
 * when you are done with it.
 *
 * Returns: the local timezone
 *
 * Since: 2.26
 **/
SysTimeZone *
sys_time_zone_new_local (void)
{
  const SysChar *tzenv = sys_env_get ("TZ");
  SysTimeZone *tz;

  SYS_LOCK (tz_local);

  /* Is time zone changed and must be flushed? */
  if (tz_local && sys_str_equal (sys_time_zone_get_identifier (tz_local), tzenv))
    sys_clear_pointer (&tz_local, sys_time_zone_unref);

  if (tz_local == NULL)
    tz_local = sys_time_zone_new_identifier (tzenv);
  if (tz_local == NULL)
    tz_local = sys_time_zone_new_utc ();

  tz = sys_time_zone_ref (tz_local);

  SYS_UNLOCK (tz_local);

  return tz;
}

/**
 * sys_time_zone_new_offset:
 * @seconds: offset to UTC, in seconds
 *
 * Creates a #SysTimeZone corresponding to the given constant offset from UTC,
 * in seconds.
 *
 * This is equivalent to calling sys_time_zone_new() with a string in the form
 * `[+|-]hh[:mm[:ss]]`.
 *
 * It is possible for this function to fail if @seconds is too big (greater than
 * 24 hours), in which case this function will return the UTC timezone for
 * backwards compatibility. To detect failures like this, use
 * sys_time_zone_new_identifier() directly.
 *
 * Returns: (transfer full): a timezone at the given offset from UTC, or UTC on
 *   failure
 * Since: 2.58
 */
SysTimeZone *
sys_time_zone_new_offset (SysInt32 seconds)
{
  SysTimeZone *tz = NULL;
  SysChar *identifier = NULL;

  /* Seemingly, we should be using @seconds directly to set the
   * #TransitionInfo.gmt_offset to avoid all this string building and parsing.
   * However, we always need to set the #SysTimeZone.name to a constructed
   * string anyway, so we might as well reuse its code.
   * sys_time_zone_new_identifier() should never fail in this situation. */
  identifier = sys_strdup_printf ("%c%02u:%02u:%02u",
                                (seconds >= 0) ? '+' : '-',
                                (abs (seconds) / 60) / 60,
                                (abs (seconds) / 60) % 60,
                                abs (seconds) % 60);
  tz = sys_time_zone_new_identifier (identifier);

  if (tz == NULL)
    tz = sys_time_zone_new_utc ();
  else
    sys_assert (sys_time_zone_get_offset (tz, 0) == seconds);

  sys_assert (tz != NULL);
  sys_free (identifier);

  return tz;
}

#define TRANSITION(n)         sys_array_index (tz->transitions, Transition, n)
#define TRANSITION_INFO(n)    sys_array_index (tz->t_info, TransitionInfo, n)

/* Internal helpers {{{1 */
/* NB: Interval 0 is before the first transition, so there's no
 * transition structure to point to which TransitionInfo to
 * use. Rule-based zones are set up so that TI 0 is always standard
 * time (which is what's in effect before Daylight time got started
 * in the early 20th century), but IANA tzfiles don't follow that
 * convention. The tzfile documentation says to use the first
 * standard-time (i.e., non-DST) tinfo, so that's what we do.
 */
inline static const TransitionInfo*
interval_info (SysTimeZone *tz,
               SysUInt      interval)
{
  SysUInt index;
  sys_return_val_if_fail (tz->t_info != NULL, NULL);
  if (interval && tz->transitions && interval <= tz->transitions->len)
    index = (TRANSITION(interval - 1)).info_index;
  else
    {
      for (index = 0; index < tz->t_info->len; index++)
        {
          TransitionInfo *tzinfo = &(TRANSITION_INFO(index));
          if (!tzinfo->is_dst)
            return tzinfo;
        }
      index = 0;
    }

  return &(TRANSITION_INFO(index));
}

inline static SysInt64
interval_start (SysTimeZone *tz,
                SysUInt      interval)
{
  if (!interval || tz->transitions == NULL || tz->transitions->len == 0)
    return SYS_MININT64;
  if (interval > tz->transitions->len)
    interval = tz->transitions->len;
  return (TRANSITION(interval - 1)).time;
}

inline static SysInt64
interval_end (SysTimeZone *tz,
              SysUInt      interval)
{
  if (tz->transitions && interval < tz->transitions->len)
  {
    SysInt64 lim = (TRANSITION(interval)).time;
    return lim - (lim != SYS_MININT64);
  }
  return SYS_MAXINT64;
}

inline static SysInt32
interval_offset (SysTimeZone *tz,
                 SysUInt      interval)
{
  sys_return_val_if_fail (tz->t_info != NULL, 0);
  return interval_info (tz, interval)->gmt_offset;
}

inline static SysBool
interval_isdst (SysTimeZone *tz,
                SysUInt      interval)
{
  sys_return_val_if_fail (tz->t_info != NULL, 0);
  return interval_info (tz, interval)->is_dst;
}


inline static SysChar*
interval_abbrev (SysTimeZone *tz,
                  SysUInt      interval)
{
  sys_return_val_if_fail (tz->t_info != NULL, 0);
  return interval_info (tz, interval)->abbrev;
}

inline static SysInt64
interval_local_start (SysTimeZone *tz,
                      SysUInt      interval)
{
  if (interval)
    return interval_start (tz, interval) + interval_offset (tz, interval);

  return SYS_MININT64;
}

inline static SysInt64
interval_local_end (SysTimeZone *tz,
                    SysUInt      interval)
{
  if (tz->transitions && interval < tz->transitions->len)
    return interval_end (tz, interval) + interval_offset (tz, interval);

  return SYS_MAXINT64;
}

static SysBool
interval_valid (SysTimeZone *tz,
                SysUInt      interval)
{
  if ( tz->transitions == NULL)
    return interval == 0;
  return interval <= tz->transitions->len;
}

/* sys_time_zone_find_interval() {{{1 */

/**
 * sys_time_zone_adjust_time:
 * @tz: a #SysTimeZone
 * @type: the #SysTimeType of @time_
 * @time_: (inout): a pointer to a number of seconds since January 1, 1970
 *
 * Finds an interval within @tz that corresponds to the given @time_,
 * possibly adjusting @time_ if required to fit into an interval.
 * The meaning of @time_ depends on @type.
 *
 * This function is similar to sys_time_zone_find_interval(), with the
 * difference that it always succeeds (by making the adjustments
 * described below).
 *
 * In any of the cases where sys_time_zone_find_interval() succeeds then
 * this function returns the same value, without modifying @time_.
 *
 * This function may, however, modify @time_ in order to deal with
 * non-existent times.  If the non-existent local @time_ of 02:30 were
 * requested on March 14th 2010 in Toronto then this function would
 * adjust @time_ to be 03:00 and return the interval containing the
 * adjusted time.
 *
 * Returns: the interval containing @time_, never -1
 *
 * Since: 2.26
 **/
SysInt
sys_time_zone_adjust_time (SysTimeZone *tz,
                         SysTimeType  type,
                         SysInt64    *time_)
{
  SysUInt i, intervals;
  SysBool interval_is_dst;

  if (tz->transitions == NULL)
    return 0;

  intervals = tz->transitions->len;

  /* find the interval containing *time UTC
   * TODO: this could be binary searched (or better) */
  for (i = 0; i <= intervals; i++)
    if (*time_ <= interval_end (tz, i))
      break;

  sys_assert (interval_start (tz, i) <= *time_ && *time_ <= interval_end (tz, i));

  if (type != SYS_TIME_TYPE_UNIVERSAL)
    {
      if (*time_ < interval_local_start (tz, i))
        /* if time came before the start of this interval... */
        {
          i--;

          /* if it's not in the previous interval... */
          if (*time_ > interval_local_end (tz, i))
            {
              /* it doesn't exist.  fast-forward it. */
              i++;
              *time_ = interval_local_start (tz, i);
            }
        }

      else if (*time_ > interval_local_end (tz, i))
        /* if time came after the end of this interval... */
        {
          i++;

          /* if it's not in the next interval... */
          if (*time_ < interval_local_start (tz, i))
            /* it doesn't exist.  fast-forward it. */
            *time_ = interval_local_start (tz, i);
        }

      else
        {
          interval_is_dst = interval_isdst (tz, i);
          if ((interval_is_dst && type != SYS_TIME_TYPE_DAYLIGHT) ||
              (!interval_is_dst && type == SYS_TIME_TYPE_DAYLIGHT))
            {
              /* it's in this interval, but dst flag doesn't match.
               * check neighbours for a better fit. */
              if (i && *time_ <= interval_local_end (tz, i - 1))
                i--;

              else if (i < intervals &&
                       *time_ >= interval_local_start (tz, i + 1))
                i++;
            }
        }
    }

  return i;
}

/**
 * sys_time_zone_find_interval:
 * @tz: a #SysTimeZone
 * @type: the #SysTimeType of @time_
 * @time_: a number of seconds since January 1, 1970
 *
 * Finds an interval within @tz that corresponds to the given @time_.
 * The meaning of @time_ depends on @type.
 *
 * If @type is %SYS_TIME_TYPE_UNIVERSAL then this function will always
 * succeed (since universal time is monotonic and continuous).
 *
 * Otherwise @time_ is treated as local time.  The distinction between
 * %SYS_TIME_TYPE_STANDARD and %SYS_TIME_TYPE_DAYLIGHT is ignored except in
 * the case that the given @time_ is ambiguous.  In Toronto, for example,
 * 01:30 on November 7th 2010 occurred twice (once inside of daylight
 * savings time and the next, an hour later, outside of daylight savings
 * time).  In this case, the different value of @type would result in a
 * different interval being returned.
 *
 * It is still possible for this function to fail.  In Toronto, for
 * example, 02:00 on March 14th 2010 does not exist (due to the leap
 * forward to begin daylight savings time).  -1 is returned in that
 * case.
 *
 * Returns: the interval containing @time_, or -1 in case of failure
 *
 * Since: 2.26
 */
SysInt
sys_time_zone_find_interval (SysTimeZone *tz,
                           SysTimeType  type,
                           SysInt64     time_)
{
  SysUInt i, intervals;
  SysBool interval_is_dst;

  if (tz->transitions == NULL)
    return 0;
  intervals = tz->transitions->len;
  for (i = 0; i <= intervals; i++)
    if (time_ <= interval_end (tz, i))
      break;

  if (type == SYS_TIME_TYPE_UNIVERSAL)
    return i;

  if (time_ < interval_local_start (tz, i))
    {
      if (time_ > interval_local_end (tz, --i))
        return -1;
    }

  else if (time_ > interval_local_end (tz, i))
    {
      if (time_ < interval_local_start (tz, ++i))
        return -1;
    }

  else
    {
      interval_is_dst = interval_isdst (tz, i);
      if  ((interval_is_dst && type != SYS_TIME_TYPE_DAYLIGHT) ||
           (!interval_is_dst && type == SYS_TIME_TYPE_DAYLIGHT))
        {
          if (i && time_ <= interval_local_end (tz, i - 1))
            i--;

          else if (i < intervals && time_ >= interval_local_start (tz, i + 1))
            i++;
        }
    }

  return i;
}

/* Public API accessors {{{1 */

/**
 * sys_time_zone_get_abbreviation:
 * @tz: a #SysTimeZone
 * @interval: an interval within the timezone
 *
 * Determines the time zone abbreviation to be used during a particular
 * @interval of time in the time zone @tz.
 *
 * For example, in Toronto this is currently "EST" during the winter
 * months and "EDT" during the summer months when daylight savings time
 * is in effect.
 *
 * Returns: the time zone abbreviation, which belongs to @tz
 *
 * Since: 2.26
 **/
const SysChar *
sys_time_zone_get_abbreviation (SysTimeZone *tz,
                              SysInt       interval)
{
  sys_return_val_if_fail (interval_valid (tz, (SysUInt)interval), NULL);

  return interval_abbrev (tz, (SysUInt)interval);
}

/**
 * sys_time_zone_get_offset:
 * @tz: a #SysTimeZone
 * @interval: an interval within the timezone
 *
 * Determines the offset to UTC in effect during a particular @interval
 * of time in the time zone @tz.
 *
 * The offset is the number of seconds that you add to UTC time to
 * arrive at local time for @tz (ie: negative numbers for time zones
 * west of GMT, positive numbers for east).
 *
 * Returns: the number of seconds that should be added to UTC to get the
 *          local time in @tz
 *
 * Since: 2.26
 **/
SysInt32
sys_time_zone_get_offset (SysTimeZone *tz,
                        SysInt       interval)
{
  sys_return_val_if_fail (interval_valid (tz, (SysUInt)interval), 0);

  return interval_offset (tz, (SysUInt)interval);
}

/**
 * sys_time_zone_is_dst:
 * @tz: a #SysTimeZone
 * @interval: an interval within the timezone
 *
 * Determines if daylight savings time is in effect during a particular
 * @interval of time in the time zone @tz.
 *
 * Returns: %true if daylight savings time is in effect
 *
 * Since: 2.26
 **/
SysBool
sys_time_zone_is_dst (SysTimeZone *tz,
                    SysInt       interval)
{
  sys_return_val_if_fail (interval_valid (tz, interval), false);

  if (tz->transitions == NULL)
    return false;

  return interval_isdst (tz, (SysUInt)interval);
}

/**
 * sys_time_zone_get_identifier:
 * @tz: a #SysTimeZone
 *
 * Get the identifier of this #SysTimeZone, as passed to sys_time_zone_new().
 * If the identifier passed at construction time was not recognised, `UTC` will
 * be returned. If it was %NULL, the identifier of the local timezone at
 * construction time will be returned.
 *
 * The identifier will be returned in the same format as provided at
 * construction time: if provided as a time offset, that will be returned by
 * this function.
 *
 * Returns: identifier for this timezone
 * Since: 2.58
 */
const SysChar *
sys_time_zone_get_identifier (SysTimeZone *tz)
{
  sys_return_val_if_fail (tz != NULL, NULL);

  return tz->name;
}
