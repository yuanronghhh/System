#include <System/Platform/Common/SysDateTime.h>
#include <System/Platform/Common/SysOs.h>
#include <System/Platform/Common/SysTimeZone.h>
#include <System/Platform/Common/SysThreadPrivate.h>
#include <System/DataTypes/SysString.h>
#include <System/Utils/SysUtf8.h>
#include <System/Utils/SysCharSetPrivate.h>
#include <System/Utils/SysConvert.h>
#include <System/Utils/SysStr.h>
#include <System/Utils/SysConvertPrivate.h>

struct _SysDateTime
{
  /* Microsecond timekeeping within Day */
  SysUInt64 usec;

  /* TimeZone information */
  SysTimeZone *tz;
  SysInt interval;

  /* 1 is 0001-01-01 in Proleptic Gregorian */
  SysInt32 days;

  SysInt ref_count;  /* (atomic) */
};

/* Time conversion {{{1 */

#define UNIX_EPOCH_START     719163
#define INSTANT_TO_UNIX(instant) \
  ((instant)/USEC_PER_SECOND - UNIX_EPOCH_START * SEC_PER_DAY)
#define INSTANT_TO_UNIX_USECS(instant) \
  ((instant) - UNIX_EPOCH_START * SEC_PER_DAY * USEC_PER_SECOND)
#define UNIX_TO_INSTANT(unix) \
  (((SysInt64) (unix) + UNIX_EPOCH_START * SEC_PER_DAY) * USEC_PER_SECOND)
#define UNIX_USECS_TO_INSTANT(unix_usecs) \
  ((SysInt64) (unix_usecs) + UNIX_EPOCH_START * SEC_PER_DAY * USEC_PER_SECOND)
#define UNIX_TO_INSTANT_IS_VALID(unix) \
  ((SysInt64) (unix) <= INSTANT_TO_UNIX (SYS_MAXINT64))
#define UNIX_USECS_TO_INSTANT_IS_VALID(unix_usecs) \
  ((SysInt64) (unix_usecs) <= INSTANT_TO_UNIX_USECS (SYS_MAXINT64))

#define DAYS_IN_4YEARS    1461    /* days in 4 years */
#define DAYS_IN_100YEARS  36524   /* days in 100 years */
#define DAYS_IN_400YEARS  146097  /* days in 400 years  */

#define USEC_PER_SECOND      (SYS_INT64_CONSTANT (1000000))
#define USEC_PER_MINUTE      (SYS_INT64_CONSTANT (60000000))
#define USEC_PER_HOUR        (SYS_INT64_CONSTANT (3600000000))
#define USEC_PER_MILLISECOND (SYS_INT64_CONSTANT (1000))
#define USEC_PER_DAY         (SYS_INT64_CONSTANT (86400000000))
#define SEC_PER_DAY          (SYS_INT64_CONSTANT (86400))

#define SECS_PER_MINUTE (60)
#define SECS_PER_HOUR   (60 * SECS_PER_MINUTE)
#define SECS_PER_DAY    (24 * SECS_PER_HOUR)
#define SECS_PER_YEAR   (365 * SECS_PER_DAY)
#define SECS_PER_JULIAN (DAYS_PER_PERIOD * SECS_PER_DAY)

#define GREGORIAN_LEAP(y)    ((((y) % 4) == 0) && (!((((y) % 100) == 0) && (((y) % 400) != 0))))
#define JULIAN_YEAR(d)       ((d)->julian / 365.25)
#define DAYS_PER_PERIOD      (SYS_INT64_CONSTANT (2914695))

static const SysUInt16 days_in_months[2][13] =
{
  { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
  { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const SysUInt16 days_in_year[2][13] =
{
  {  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
  {  0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

#ifdef HAVE_LANGINFO_TIME

#define GET_AMPM(d) ((sys_date_time_get_hour (d) < 12) ? \
                     nl_langinfo (AM_STR) : \
                     nl_langinfo (PM_STR))
#define GET_AMPM_IS_LOCALE true

#define PREFERRED_DATE_TIME_FMT nl_langinfo (D_T_FMT)
#define PREFERRED_DATE_FMT nl_langinfo (D_FMT)
#define PREFERRED_TIME_FMT nl_langinfo (T_FMT)
#define PREFERRED_12HR_TIME_FMT nl_langinfo (T_FMT_AMPM)

static const SysInt weekday_item[2][7] =
{
  { ABDAY_2, ABDAY_3, ABDAY_4, ABDAY_5, ABDAY_6, ABDAY_7, ABDAY_1 },
  { DAY_2, DAY_3, DAY_4, DAY_5, DAY_6, DAY_7, DAY_1 }
};

static const SysInt month_item[2][12] =
{
  { ABMON_1, ABMON_2, ABMON_3, ABMON_4, ABMON_5, ABMON_6, ABMON_7, ABMON_8, ABMON_9, ABMON_10, ABMON_11, ABMON_12 },
  { MON_1, MON_2, MON_3, MON_4, MON_5, MON_6, MON_7, MON_8, MON_9, MON_10, MON_11, MON_12 },
};

#define WEEKDAY_ABBR(d) nl_langinfo (weekday_item[0][sys_date_time_get_day_of_week (d) - 1])
#define WEEKDAY_ABBR_IS_LOCALE true
#define WEEKDAY_FULL(d) nl_langinfo (weekday_item[1][sys_date_time_get_day_of_week (d) - 1])
#define WEEKDAY_FULL_IS_LOCALE true
#define MONTH_ABBR(d) nl_langinfo (month_item[0][sys_date_time_get_month (d) - 1])
#define MONTH_ABBR_IS_LOCALE true
#define MONTH_FULL(d) nl_langinfo (month_item[1][sys_date_time_get_month (d) - 1])
#define MONTH_FULL_IS_LOCALE true

#else

#define GET_AMPM(d)          (get_fallback_ampm (sys_date_time_get_hour (d)))
#define GET_AMPM_IS_LOCALE   false

/* Translators: this is the preferred format for expressing the date and the time */
#define PREFERRED_DATE_TIME_FMT SYS_C_("SysDateTime", "%a %b %e %H:%M:%S %Y")

/* Translators: this is the preferred format for expressing the date */
#define PREFERRED_DATE_FMT SYS_C_("SysDateTime", "%m/%d/%y")

/* Translators: this is the preferred format for expressing the time */
#define PREFERRED_TIME_FMT SYS_C_("SysDateTime", "%H:%M:%S")

/* Translators: this is the preferred format for expressing 12 hour time */
#define PREFERRED_12HR_TIME_FMT SYS_C_("SysDateTime", "%I:%M:%S %p")

#define WEEKDAY_ABBR(d)       (get_weekday_name_abbr (sys_date_time_get_day_of_week (d)))
#define WEEKDAY_ABBR_IS_LOCALE false
#define WEEKDAY_FULL(d)       (get_weekday_name (sys_date_time_get_day_of_week (d)))
#define WEEKDAY_FULL_IS_LOCALE false
/* We don't yet know if nl_langinfo (MON_n) returns standalone or complete-date
 * format forms but if nl_langinfo (ALTMON_n) is not supported then we will
 * have to use MONTH_FULL as standalone.  The same if nl_langinfo () does not
 * exist at all.  MONTH_ABBR is similar: if nl_langinfo (_NL_ABALTMON_n) is not
 * supported then we will use MONTH_ABBR as standalone.
 */
#define MONTH_ABBR(d)         (get_month_name_abbr_standalone (sys_date_time_get_month (d)))
#define MONTH_ABBR_IS_LOCALE  false
#define MONTH_FULL(d)         (get_month_name_standalone (sys_date_time_get_month (d)))
#define MONTH_FULL_IS_LOCALE  false


static void format_number (SysString     *str,
               SysBool     use_alt_digits,
               const SysChar *pad,
               SysInt         width,
               SysUInt32      number);

static SysBool format_ampm (SysDateTime *datetime,
             SysString   *outstr,
             SysBool   locale_is_utf8,
             SysBool   uppercase);

static SysBool format_z (SysString *outstr,
          SysInt     offset,
          SysUInt    colons);

static const SysChar *
get_month_name_standalone (SysInt month)
{
  switch (month)
    {
    case 1:
      /* Translators: Some languages (Baltic, Slavic, Greek, and some more)
       * need different grammatical forms of month names depending on whether
       * they are standalone or in a complete date context, with the day
       * number.  Some other languages may prefer starting with uppercase when
       * they are standalone and with lowercase when they are in a complete
       * date context.  Here are full month names in a form appropriate when
       * they are used standalone.  If your system is Linux with the glibc
       * version 2.27 (released Feb 1, 2018) or newer or if it is from the BSD
       * family (which includes OS X) then you can refer to the date command
       * line utility and see what the command `date +%OB' produces.  Also in
       * the latest Linux the command `locale alt_mon' in your native locale
       * produces a complete list of month names almost ready to copy and
       * paste here.  Note that in most of the languages (western European,
       * non-European) there is no difference between the standalone and
       * complete date form.
       */
      return SYS_C_("full month name", "January");
    case 2:
      return SYS_C_("full month name", "February");
    case 3:
      return SYS_C_("full month name", "March");
    case 4:
      return SYS_C_("full month name", "April");
    case 5:
      return SYS_C_("full month name", "May");
    case 6:
      return SYS_C_("full month name", "June");
    case 7:
      return SYS_C_("full month name", "July");
    case 8:
      return SYS_C_("full month name", "August");
    case 9:
      return SYS_C_("full month name", "September");
    case 10:
      return SYS_C_("full month name", "October");
    case 11:
      return SYS_C_("full month name", "November");
    case 12:
      return SYS_C_("full month name", "December");

    default:

      sys_warning_N ("Invalid month number %d", month);
    }

  return NULL;
}

static const SysChar *
get_month_name_abbr_standalone (SysInt month)
{
  switch (month)
    {
    case 1:
      /* Translators: Some languages need different grammatical forms of
       * month names depending on whether they are standalone or in a complete
       * date context, with the day number.  Some may prefer starting with
       * uppercase when they are standalone and with lowercase when they are
       * in a full date context.  However, as these names are abbreviated
       * the grammatical difference is visible probably only in Belarusian
       * and Russian.  In other languages there is no difference between
       * the standalone and complete date form when they are abbreviated.
       * If your system is Linux with the glibc version 2.27 (released
       * Feb 1, 2018) or newer then you can refer to the date command line
       * utility and see what the command `date +%Ob' produces.  Also in
       * the latest Linux the command `locale ab_alt_mon' in your native
       * locale produces a complete list of month names almost ready to copy
       * and paste here.  Note that this feature is not yet supported by any
       * other platform.  Here are abbreviated month names in a form
       * appropriate when they are used standalone.
       */
      return SYS_C_("abbreviated month name", "Jan");
    case 2:
      return SYS_C_("abbreviated month name", "Feb");
    case 3:
      return SYS_C_("abbreviated month name", "Mar");
    case 4:
      return SYS_C_("abbreviated month name", "Apr");
    case 5:
      return SYS_C_("abbreviated month name", "May");
    case 6:
      return SYS_C_("abbreviated month name", "Jun");
    case 7:
      return SYS_C_("abbreviated month name", "Jul");
    case 8:
      return SYS_C_("abbreviated month name", "Aug");
    case 9:
      return SYS_C_("abbreviated month name", "Sep");
    case 10:
      return SYS_C_("abbreviated month name", "Oct");
    case 11:
      return SYS_C_("abbreviated month name", "Nov");
    case 12:
      return SYS_C_("abbreviated month name", "Dec");

    default:
      sys_warning_N ("Invalid month number %d", month);
    }

  return NULL;
}

static const SysChar *
get_weekday_name (SysInt day)
{
  switch (day)
    {
    case 1:
      return SYS_C_("full weekday name", "Monday");
    case 2:
      return SYS_C_("full weekday name", "Tuesday");
    case 3:
      return SYS_C_("full weekday name", "Wednesday");
    case 4:
      return SYS_C_("full weekday name", "Thursday");
    case 5:
      return SYS_C_("full weekday name", "Friday");
    case 6:
      return SYS_C_("full weekday name", "Saturday");
    case 7:
      return SYS_C_("full weekday name", "Sunday");

    default:
      sys_warning_N ("Invalid week day number %d", day);
    }

  return NULL;
}

static const SysChar *
get_weekday_name_abbr (SysInt day)
{
  switch (day)
    {
    case 1:
      return SYS_C_("abbreviated weekday name", "Mon");
    case 2:
      return SYS_C_("abbreviated weekday name", "Tue");
    case 3:
      return SYS_C_("abbreviated weekday name", "Wed");
    case 4:
      return SYS_C_("abbreviated weekday name", "Thu");
    case 5:
      return SYS_C_("abbreviated weekday name", "Fri");
    case 6:
      return SYS_C_("abbreviated weekday name", "Sat");
    case 7:
      return SYS_C_("abbreviated weekday name", "Sun");

    default:
      sys_warning_N ("Invalid week day number %d", day);
    }

  return NULL;
}

#endif  /* HAVE_LANGINFO_TIME */

#ifdef HAVE_LANGINFO_ALTMON

/* If nl_langinfo () supports ALTMON_n then MON_n returns full date format
 * forms and ALTMON_n returns standalone forms.
 */

#define MONTH_FULL_WITH_DAY(d) MONTH_FULL(d)
#define MONTH_FULL_WITH_DAY_IS_LOCALE MONTH_FULL_IS_LOCALE

static const SysInt alt_month_item[12] =
{
  ALTMON_1, ALTMON_2, ALTMON_3, ALTMON_4, ALTMON_5, ALTMON_6,
  ALTMON_7, ALTMON_8, ALTMON_9, ALTMON_10, ALTMON_11, ALTMON_12
};

#define MONTH_FULL_STANDALONE(d) nl_langinfo (alt_month_item[sys_date_time_get_month (d) - 1])
#define MONTH_FULL_STANDALONE_IS_LOCALE true

#else

/* If nl_langinfo () does not support ALTMON_n then either MON_n returns
 * standalone forms or nl_langinfo (MON_n) does not work so we have defined
 * it as standalone form.
 */

#define MONTH_FULL_STANDALONE(d) MONTH_FULL(d)
#define MONTH_FULL_STANDALONE_IS_LOCALE MONTH_FULL_IS_LOCALE
#define MONTH_FULL_WITH_DAY(d) (get_month_name_with_day (sys_date_time_get_month (d)))
#define MONTH_FULL_WITH_DAY_IS_LOCALE false

static const SysChar *
get_month_name_with_day (SysInt month)
{
  switch (month)
    {
    case 1:
      /* Translators: Some languages need different grammatical forms of
       * month names depending on whether they are standalone or in a full
       * date context, with the day number.  Some may prefer starting with
       * uppercase when they are standalone and with lowercase when they are
       * in a full date context.  Here are full month names in a form
       * appropriate when they are used in a full date context, with the
       * day number.  If your system is Linux with the glibc version 2.27
       * (released Feb 1, 2018) or newer or if it is from the BSD family
       * (which includes OS X) then you can refer to the date command line
       * utility and see what the command `date +%B' produces.  Also in
       * the latest Linux the command `locale mon' in your native locale
       * produces a complete list of month names almost ready to copy and
       * paste here.  In older Linux systems due to a bug the result is
       * incorrect in some languages.  Note that in most of the languages
       * (western European, non-European) there is no difference between the
       * standalone and complete date form.
       */
      return SYS_C_("full month name with day", "January");
    case 2:
      return SYS_C_("full month name with day", "February");
    case 3:
      return SYS_C_("full month name with day", "March");
    case 4:
      return SYS_C_("full month name with day", "April");
    case 5:
      return SYS_C_("full month name with day", "May");
    case 6:
      return SYS_C_("full month name with day", "June");
    case 7:
      return SYS_C_("full month name with day", "July");
    case 8:
      return SYS_C_("full month name with day", "August");
    case 9:
      return SYS_C_("full month name with day", "September");
    case 10:
      return SYS_C_("full month name with day", "October");
    case 11:
      return SYS_C_("full month name with day", "November");
    case 12:
      return SYS_C_("full month name with day", "December");

    default:
      sys_warning_N ("Invalid month number %d", month);
    }

  return NULL;
}

#endif  /* HAVE_LANGINFO_ALTMON */

#ifdef HAVE_LANGINFO_ABALTMON

/* If nl_langinfo () supports _NL_ABALTMON_n then ABMON_n returns full
 * date format forms and _NL_ABALTMON_n returns standalone forms.
 */

#define MONTH_ABBR_WITH_DAY(d) MONTH_ABBR(d)
#define MONTH_ABBR_WITH_DAY_IS_LOCALE MONTH_ABBR_IS_LOCALE

static const SysInt ab_alt_month_item[12] =
{
  _NL_ABALTMON_1, _NL_ABALTMON_2, _NL_ABALTMON_3, _NL_ABALTMON_4,
  _NL_ABALTMON_5, _NL_ABALTMON_6, _NL_ABALTMON_7, _NL_ABALTMON_8,
  _NL_ABALTMON_9, _NL_ABALTMON_10, _NL_ABALTMON_11, _NL_ABALTMON_12
};

#define MONTH_ABBR_STANDALONE(d) nl_langinfo (ab_alt_month_item[sys_date_time_get_month (d) - 1])
#define MONTH_ABBR_STANDALONE_IS_LOCALE true

#else

/* If nl_langinfo () does not support _NL_ABALTMON_n then either ABMON_n
 * returns standalone forms or nl_langinfo (ABMON_n) does not work so we
 * have defined it as standalone form. Now it's time to swap.
 */

#define MONTH_ABBR_STANDALONE(d) MONTH_ABBR(d)
#define MONTH_ABBR_STANDALONE_IS_LOCALE MONTH_ABBR_IS_LOCALE
#define MONTH_ABBR_WITH_DAY(d) (get_month_name_abbr_with_day (sys_date_time_get_month (d)))
#define MONTH_ABBR_WITH_DAY_IS_LOCALE false

static const SysChar *
get_month_name_abbr_with_day (SysInt month)
{
  switch (month)
    {
    case 1:
      /* Translators: Some languages need different grammatical forms of
       * month names depending on whether they are standalone or in a full
       * date context, with the day number.  Some may prefer starting with
       * uppercase when they are standalone and with lowercase when they are
       * in a full date context.  Here are abbreviated month names in a form
       * appropriate when they are used in a full date context, with the
       * day number.  However, as these names are abbreviated the grammatical
       * difference is visible probably only in Belarusian and Russian.
       * In other languages there is no difference between the standalone
       * and complete date form when they are abbreviated.  If your system
       * is Linux with the glibc version 2.27 (released Feb 1, 2018) or newer
       * then you can refer to the date command line utility and see what the
       * command `date +%b' produces.  Also in the latest Linux the command
       * `locale abmon' in your native locale produces a complete list of
       * month names almost ready to copy and paste here.  In other systems
       * due to a bug the result is incorrect in some languages.
       */
      return SYS_C_("abbreviated month name with day", "Jan");
    case 2:
      return SYS_C_("abbreviated month name with day", "Feb");
    case 3:
      return SYS_C_("abbreviated month name with day", "Mar");
    case 4:
      return SYS_C_("abbreviated month name with day", "Apr");
    case 5:
      return SYS_C_("abbreviated month name with day", "May");
    case 6:
      return SYS_C_("abbreviated month name with day", "Jun");
    case 7:
      return SYS_C_("abbreviated month name with day", "Jul");
    case 8:
      return SYS_C_("abbreviated month name with day", "Aug");
    case 9:
      return SYS_C_("abbreviated month name with day", "Sep");
    case 10:
      return SYS_C_("abbreviated month name with day", "Oct");
    case 11:
      return SYS_C_("abbreviated month name with day", "Nov");
    case 12:
      return SYS_C_("abbreviated month name with day", "Dec");

    default:

      sys_warning_N ("Invalid month number %d", month);
    }

  return NULL;
}

#endif  /* HAVE_LANGINFO_ABALTMON */

/* Format AM/PM indicator if the locale does not have a localized version. */
static const SysChar *
get_fallback_ampm (SysInt hour)
{
  if (hour < 12)
    /* Translators: 'before midday' indicator */
    return SYS_C_("SysDateTime", "AM");
  else
    /* Translators: 'after midday' indicator */
    return SYS_C_("SysDateTime", "PM");
}

static inline SysInt
ymd_to_days (SysInt year,
             SysInt month,
             SysInt day)
{
  SysInt64 days;

  days = ((SysInt64) year - 1) * 365 + ((year - 1) / 4) - ((year - 1) / 100)
      + ((year - 1) / 400);

  days += days_in_year[0][month - 1];
  if (GREGORIAN_LEAP (year) && month > 2)
    day++;

  days += day;

  return (SysInt)days;
}

static void
sys_date_time_get_week_number (SysDateTime *datetime,
                             SysInt      *week_number,
                             SysInt      *day_of_week,
                             SysInt      *day_of_year)
{
  SysInt a, b, c, d, e, f, g, n, s, month = -1, day = -1, year = -1;

  sys_date_time_get_ymd (datetime, &year, &month, &day);

  if (month <= 2)
    {
      a = sys_date_time_get_year (datetime) - 1;
      b = (a / 4) - (a / 100) + (a / 400);
      c = ((a - 1) / 4) - ((a - 1) / 100) + ((a - 1) / 400);
      s = b - c;
      e = 0;
      f = day - 1 + (31 * (month - 1));
    }
  else
    {
      a = year;
      b = (a / 4) - (a / 100) + (a / 400);
      c = ((a - 1) / 4) - ((a - 1) / 100) + ((a - 1) / 400);
      s = b - c;
      e = s + 1;
      f = day + (((153 * (month - 3)) + 2) / 5) + 58 + s;
    }

  g = (a + b) % 7;
  d = (f + g - e) % 7;
  n = f + 3 - d;

  if (week_number)
    {
      if (n < 0)
        *week_number = 53 - ((g - s) / 5);
      else if (n > 364 + s)
        *week_number = 1;
      else
        *week_number = (n / 7) + 1;
    }

  if (day_of_week)
    *day_of_week = d + 1;

  if (day_of_year)
    *day_of_year = f + 1;
}

/* Lifecycle {{{1 */

static SysDateTime *
sys_date_time_alloc (SysTimeZone *tz)
{
  SysDateTime *datetime;

  datetime = sys_slice_new0 (SysDateTime);
  datetime->tz = sys_time_zone_ref (tz);
  datetime->ref_count = 1;

  return datetime;
}

/**
 * sys_date_time_ref:
 * @datetime: a #SysDateTime
 *
 * Atomically increments the reference count of @datetime by one.
 *
 * Returns: the #SysDateTime with the reference count increased
 *
 * Since: 2.26
 */
SysDateTime *
sys_date_time_ref (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, NULL);
  sys_return_val_if_fail (datetime->ref_count > 0, NULL);

  sys_atomic_int_inc (&datetime->ref_count);

  return datetime;
}

/**
 * sys_date_time_unref:
 * @datetime: a #SysDateTime
 *
 * Atomically decrements the reference count of @datetime by one.
 *
 * When the reference count reaches zero, the resources allocated by
 * @datetime are freed
 *
 * Since: 2.26
 */
void
sys_date_time_unref (SysDateTime *datetime)
{
  sys_return_if_fail (datetime != NULL);
  sys_return_if_fail (datetime->ref_count > 0);

  if (sys_atomic_int_dec_and_test (&datetime->ref_count))
    {
      sys_time_zone_unref (datetime->tz);
      sys_slice_free (SysDateTime, datetime);
    }
}

/* Internal state transformers {{{1 */
/*< internal >
 * sys_date_time_to_instant:
 * @datetime: a #SysDateTime
 *
 * Convert a @datetime into an instant.
 *
 * An instant is a number that uniquely describes a particular
 * microsecond in time, taking time zone considerations into account.
 * (ie: "03:00 -0400" is the same instant as "02:00 -0500").
 *
 * An instant is always positive but we use a signed return value to
 * avoid troubles with C.
 */
static SysInt64
sys_date_time_to_instant (SysDateTime *datetime)
{
  SysInt64 offset;

  offset = sys_time_zone_get_offset (datetime->tz, datetime->interval);
  offset *= USEC_PER_SECOND;

  return datetime->days * USEC_PER_DAY + datetime->usec - offset;
}

/*< internal >
 * sys_date_time_from_instant:
 * @tz: a #SysTimeZone
 * @instant: an instant in time
 *
 * Creates a #SysDateTime from a time zone and an instant.
 *
 * This might fail if the time ends up being out of range.
 */
static SysDateTime *
sys_date_time_from_instant (SysTimeZone *tz,
                          SysInt64     instant)
{
  SysDateTime *datetime;
  SysInt64 offset;

  if (instant < 0 || instant > SYS_INT64_CONSTANT (1000000000000000000))
    return NULL;

  datetime = sys_date_time_alloc (tz);
  datetime->interval = sys_time_zone_find_interval (tz,
                                                  SYS_TIME_TYPE_UNIVERSAL,
                                                  INSTANT_TO_UNIX (instant));
  offset = sys_time_zone_get_offset (datetime->tz, datetime->interval);
  offset *= USEC_PER_SECOND;

  instant += offset;

  datetime->days = (SysInt32)(instant / USEC_PER_DAY);
  datetime->usec = instant % USEC_PER_DAY;

  if (datetime->days < 1 || 3652059 < datetime->days)
    {
      sys_date_time_unref (datetime);
      datetime = NULL;
    }

  return datetime;
}


/*< internal >
 * sys_date_time_deal_with_date_change:
 * @datetime: a #SysDateTime
 *
 * This function should be called whenever the date changes by adding
 * days, months or years.  It does three things.
 *
 * First, we ensure that the date falls between 0001-01-01 and
 * 9999-12-31 and return %false if it does not.
 *
 * Next we update the ->interval field.
 *
 * Finally, we ensure that the resulting date and time pair exists (by
 * ensuring that our time zone has an interval containing it) and
 * adjusting as required.  For example, if we have the time 02:30:00 on
 * March 13 2010 in Toronto and we add 1 day to it, we would end up with
 * 2:30am on March 14th, which doesn't exist.  In that case, we bump the
 * time up to 3:00am.
 */
static SysBool sys_date_time_deal_with_date_change (SysDateTime *datetime) {
  SysTimeType was_dst;
  SysInt64 full_time;
  SysInt64 usec;

  if (datetime->days < 1 || datetime->days > 3652059)
    return false;

  was_dst = sys_time_zone_is_dst (datetime->tz, datetime->interval);

  full_time = datetime->days * USEC_PER_DAY + datetime->usec;


  usec = full_time % USEC_PER_SECOND;
  full_time /= USEC_PER_SECOND;
  full_time -= UNIX_EPOCH_START * SEC_PER_DAY;

  datetime->interval = sys_time_zone_adjust_time (datetime->tz,
                                                was_dst,
                                                &full_time);
  full_time += UNIX_EPOCH_START * SEC_PER_DAY;
  full_time *= USEC_PER_SECOND;
  full_time += usec;

  datetime->days = (SysInt32)(full_time / USEC_PER_DAY);
  datetime->usec = full_time % USEC_PER_DAY;

  /* maybe daylight time caused us to shift to a different day,
   * but it definitely didn't push us into a different year */
  return true;
}

static SysDateTime *
sys_date_time_replace_days (SysDateTime *datetime,
                          SysInt       days)
{
  SysDateTime *new;

  new = sys_date_time_alloc (datetime->tz);
  new->interval = datetime->interval;
  new->usec = datetime->usec;
  new->days = days;

  if (!sys_date_time_deal_with_date_change (new))
    {
      sys_date_time_unref (new);
      new = NULL;
    }

  return new;
}

/* now/unix/timeval Constructors {{{1 */

/*< internal >
 * sys_date_time_new_from_unix:
 * @tz: a #SysTimeZone
 * @usecs: the Unix time, in microseconds since the epoch
 *
 * Creates a #SysDateTime corresponding to the given Unix time @t_us in the
 * given time zone @tz.
 *
 * Unix time is the number of seconds that have elapsed since 1970-01-01
 * 00:00:00 UTC, regardless of the time zone given.
 *
 * This call can fail (returning %NULL) if @t represents a time outside
 * of the supported range of #SysDateTime.
 *
 * You should release the return value by calling sys_date_time_unref()
 * when you are done with it.
 *
 * Returns: a new #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
static SysDateTime *
sys_date_time_new_from_unix (SysTimeZone *tz,
                           SysInt64     usecs)
{
  if (!UNIX_USECS_TO_INSTANT_IS_VALID (usecs))
    return NULL;

  return sys_date_time_from_instant (tz, UNIX_USECS_TO_INSTANT (usecs));
}

/**
 * sys_date_time_new_now: (constructor)
 * @tz: a #SysTimeZone
 *
 * Creates a #SysDateTime corresponding to this exact instant in the given
 * time zone @tz.  The time is as accurate as the system allows, to a
 * maximum accuracy of 1 microsecond.
 *
 * This function will always succeed unless GLib is still being used after the
 * year 9999.
 *
 * You should release the return value by calling sys_date_time_unref()
 * when you are done with it.
 *
 * Returns: (transfer full) (nullable): a new #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_new_now (SysTimeZone *tz)
{
  SysInt64 now_us;

  sys_return_val_if_fail (tz != NULL, NULL);

  now_us = sys_get_real_time ();

  return sys_date_time_new_from_unix (tz, now_us);
}

/**
 * sys_date_time_new_now_local: (constructor)
 *
 * Creates a #SysDateTime corresponding to this exact instant in the local
 * time zone.
 *
 * This is equivalent to calling sys_date_time_new_now() with the time
 * zone returned by sys_time_zone_new_local().
 *
 * Returns: (transfer full) (nullable): a new #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_new_now_local (void)
{
  SysDateTime *datetime;
  SysTimeZone *local;

  local = sys_time_zone_new_local ();
  datetime = sys_date_time_new_now (local);
  sys_time_zone_unref (local);

  return datetime;
}

/**
 * sys_date_time_new_now_utc: (constructor)
 *
 * Creates a #SysDateTime corresponding to this exact instant in UTC.
 *
 * This is equivalent to calling sys_date_time_new_now() with the time
 * zone returned by sys_time_zone_new_utc().
 *
 * Returns: (transfer full) (nullable): a new #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_new_now_utc (void)
{
  SysDateTime *datetime;
  SysTimeZone *utc;

  utc = sys_time_zone_new_utc ();
  datetime = sys_date_time_new_now (utc);
  sys_time_zone_unref (utc);

  return datetime;
}

/**
 * sys_date_time_new_from_unix_local: (constructor)
 * @t: the Unix time
 *
 * Creates a #SysDateTime corresponding to the given Unix time @t in the
 * local time zone.
 *
 * Unix time is the number of seconds that have elapsed since 1970-01-01
 * 00:00:00 UTC, regardless of the local time offset.
 *
 * This call can fail (returning %NULL) if @t represents a time outside
 * of the supported range of #SysDateTime.
 *
 * You should release the return value by calling sys_date_time_unref()
 * when you are done with it.
 *
 * Returns: (transfer full) (nullable): a new #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_new_from_unix_local (SysInt64 t)
{
  SysDateTime *datetime;
  SysTimeZone *local;

  if (t > SYS_MAXINT64 / USEC_PER_SECOND ||
      t < SYS_MININT64 / USEC_PER_SECOND)
    return NULL;

  local = sys_time_zone_new_local ();
  datetime = sys_date_time_new_from_unix (local, t * USEC_PER_SECOND);
  sys_time_zone_unref (local);

  return datetime;
}

/**
 * sys_date_time_new_from_unix_utc: (constructor)
 * @t: the Unix time
 *
 * Creates a #SysDateTime corresponding to the given Unix time @t in UTC.
 *
 * Unix time is the number of seconds that have elapsed since 1970-01-01
 * 00:00:00 UTC.
 *
 * This call can fail (returning %NULL) if @t represents a time outside
 * of the supported range of #SysDateTime.
 *
 * You should release the return value by calling sys_date_time_unref()
 * when you are done with it.
 *
 * Returns: (transfer full) (nullable): a new #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
SysDateTime * sys_date_time_new_from_unix_utc (SysInt64 t) {
  SysDateTime *datetime;
  SysTimeZone *utc;

  if (t > SYS_MAXINT64 / USEC_PER_SECOND ||
      t < SYS_MININT64 / USEC_PER_SECOND)
    return NULL;

  utc = sys_time_zone_new_utc ();
  datetime = sys_date_time_new_from_unix (utc, t * USEC_PER_SECOND);
  sys_time_zone_unref (utc);

  return datetime;
}


/* Parse integers in the form d (week days), dd (hours etc), ddd (ordinal days) or dddd (years) */
static SysBool get_iso8601_int (const SysChar *text, SysSSize length, SysInt *value) {
  SysSSize i;
  SysUInt v = 0;

  if (length < 1 || length > 4)
    return false;

  for (i = 0; i < length; i++)
    {
      const SysChar c = text[i];
      if (c < '0' || c > '9')
        return false;
      v = v * 10 + (c - '0');
    }

  *value = v;
  return true;
}

/* Parse seconds in the form ss or ss.sss (variable length decimal) */
static SysBool get_iso8601_seconds (const SysChar *text, SysSSize length, SysDouble *value) {
  SysSSize i;
  SysUInt64 divisor = 1, v = 0;

  if (length < 2)
    return false;

  for (i = 0; i < 2; i++)
    {
      const SysChar c = text[i];
      if (c < '0' || c > '9')
        return false;
      v = v * 10 + (c - '0');
    }

  if (length > 2 && !(text[i] == '.' || text[i] == ','))
    return false;

  /* Ignore leap seconds, see sys_date_time_new_from_iso8601() */
  if (v >= (SysUInt64)60.0 && v <= (SysUInt64)61.0)
    v = (SysUInt64)59.0;

  i++;
  if (i == length)
    return false;

  for (; i < length; i++)
    {
      const SysChar c = text[i];
      if (c < '0' || c > '9' ||
          v > (SYS_MAXUINT64 - (c - '0')) / 10 ||
          divisor > SYS_MAXUINT64 / 10)
        return false;
      v = v * 10 + (c - '0');
      divisor *= 10;
    }

  *value = (SysDouble)(v / divisor);
  return true;
}

static SysDateTime *
sys_date_time_new_ordinal (SysTimeZone *tz, SysInt year, SysInt ordinal_day, SysInt hour, SysInt minute, SysDouble seconds)
{
  SysDateTime *dt;

  if (ordinal_day < 1 || ordinal_day > (GREGORIAN_LEAP (year) ? 366 : 365))
    return NULL;

  dt = sys_date_time_new (tz, year, 1, 1, hour, minute, seconds);
  if (dt == NULL)
    return NULL;
  dt->days += ordinal_day - 1;

  return dt;
}

static SysDateTime *
sys_date_time_new_week (SysTimeZone *tz, SysInt year, SysInt week, SysInt week_day, SysInt hour, SysInt minute, SysDouble seconds)
{
  SysInt64 p;
  SysInt max_week, jan4_week_day, ordinal_day;
  SysDateTime *dt;

  p = (year * 365 + (year / 4) - (year / 100) + (year / 400)) % 7;
  max_week = p == 4 ? 53 : 52;

  if (week < 1 || week > max_week || week_day < 1 || week_day > 7)
    return NULL;

  dt = sys_date_time_new (tz, year, 1, 4, 0, 0, 0);
  if (dt == NULL)
    return NULL;
  sys_date_time_get_week_number (dt, NULL, &jan4_week_day, NULL);
  sys_date_time_unref (dt);

  ordinal_day = (week * 7) + week_day - (jan4_week_day + 3);
  if (ordinal_day < 0)
    {
      year--;
      ordinal_day += GREGORIAN_LEAP (year) ? 366 : 365;
    }
  else if (ordinal_day > (GREGORIAN_LEAP (year) ? 366 : 365))
    {
      ordinal_day -= (GREGORIAN_LEAP (year) ? 366 : 365);
      year++;
    }

  return sys_date_time_new_ordinal (tz, year, ordinal_day, hour, minute, seconds);
}

static SysDateTime *
parse_iso8601_date (const SysChar *text, SysSSize length,
                    SysInt hour, SysInt minute, SysDouble seconds, SysTimeZone *tz)
{
  /* YYYY-MM-DD */
  if (length == 10 && text[4] == '-' && text[7] == '-')
    {
      int year, month, day;
      if (!get_iso8601_int (text, 4, &year) ||
          !get_iso8601_int (text + 5, 2, &month) ||
          !get_iso8601_int (text + 8, 2, &day))
        return NULL;
      return sys_date_time_new (tz, year, month, day, hour, minute, seconds);
    }
  /* YYYY-DDD */
  else if (length == 8 && text[4] == '-')
    {
      SysInt year, ordinal_day;
      if (!get_iso8601_int (text, 4, &year) ||
          !get_iso8601_int (text + 5, 3, &ordinal_day))
        return NULL;
      return sys_date_time_new_ordinal (tz, year, ordinal_day, hour, minute, seconds);
    }
  /* YYYY-Www-D */
  else if (length == 10 && text[4] == '-' && text[5] == 'W' && text[8] == '-')
    {
      SysInt year, week, week_day;
      if (!get_iso8601_int (text, 4, &year) ||
          !get_iso8601_int (text + 6, 2, &week) ||
          !get_iso8601_int (text + 9, 1, &week_day))
        return NULL;
      return sys_date_time_new_week (tz, year, week, week_day, hour, minute, seconds);
    }
  /* YYYYWwwD */
  else if (length == 8 && text[4] == 'W')
    {
      SysInt year, week, week_day;
      if (!get_iso8601_int (text, 4, &year) ||
          !get_iso8601_int (text + 5, 2, &week) ||
          !get_iso8601_int (text + 7, 1, &week_day))
        return NULL;
      return sys_date_time_new_week (tz, year, week, week_day, hour, minute, seconds);
    }
  /* YYYYMMDD */
  else if (length == 8)
    {
      int year, month, day;
      if (!get_iso8601_int (text, 4, &year) ||
          !get_iso8601_int (text + 4, 2, &month) ||
          !get_iso8601_int (text + 6, 2, &day))
        return NULL;
      return sys_date_time_new (tz, year, month, day, hour, minute, seconds);
    }
  /* YYYYDDD */
  else if (length == 7)
    {
      SysInt year, ordinal_day;
      if (!get_iso8601_int (text, 4, &year) ||
          !get_iso8601_int (text + 4, 3, &ordinal_day))
        return NULL;
      return sys_date_time_new_ordinal (tz, year, ordinal_day, hour, minute, seconds);
    }
  else
    return false;
}

static SysTimeZone *
parse_iso8601_timezone (const SysChar *text, SysSSize length, SysSSize *tz_offset)
{
  SysInt i, tz_length, offset_hours, offset_minutes;
  SysInt offset_sign = 1;
  SysTimeZone *tz;

  /* UTC uses Z suffix  */
  if (length > 0 && text[length - 1] == 'Z')
    {
      *tz_offset = length - 1;
      return sys_time_zone_new_utc ();
    }

  /* Look for '+' or '-' of offset */
  for (i = (SysInt)(length - 1); i >= 0; i--)
    if (text[i] == '+' || text[i] == '-')
      {
        offset_sign = text[i] == '-' ? -1 : 1;
        break;
      }
  if (i < 0)
    return NULL;
  tz_length = (SysInt)(length - i);

  /* +hh:mm or -hh:mm */
  if (tz_length == 6 && text[i+3] == ':')
    {
      if (!get_iso8601_int (text + i + 1, 2, &offset_hours) ||
          !get_iso8601_int (text + i + 4, 2, &offset_minutes))
        return NULL;
    }
  /* +hhmm or -hhmm */
  else if (tz_length == 5)
    {
      if (!get_iso8601_int (text + i + 1, 2, &offset_hours) ||
          !get_iso8601_int (text + i + 3, 2, &offset_minutes))
        return NULL;
    }
  /* +hh or -hh */
  else if (tz_length == 3)
    {
      if (!get_iso8601_int (text + i + 1, 2, &offset_hours))
        return NULL;
      offset_minutes = 0;
    }
  else
    return NULL;

  *tz_offset = i;
  tz = sys_time_zone_new_identifier (text + i);

  /* Double-check that the SysTimeZone matches our interpretation of the timezone.
   * This can fail because our interpretation is less strict than (for example)
   * parse_time() in gtimezone.c, which restricts the range of the parsed
   * integers. */
  if (tz == NULL || sys_time_zone_get_offset (tz, 0) != offset_sign * (offset_hours * 3600 + offset_minutes * 60))
    {
      sys_clear_pointer (&tz, sys_time_zone_unref);
      return NULL;
    }

  return tz;
}

static SysBool
parse_iso8601_time (const SysChar *text, SysSSize length,
                    SysInt *hour, SysInt *minute, SysDouble *seconds, SysTimeZone **tz)
{
  SysSSize tz_offset = -1;

  /* Check for timezone suffix */
  *tz = parse_iso8601_timezone (text, length, &tz_offset);
  if (tz_offset >= 0)
    length = tz_offset;

  /* hh:mm:ss(.sss) */
  if (length >= 8 && text[2] == ':' && text[5] == ':')
    {
      return get_iso8601_int (text, 2, hour) &&
             get_iso8601_int (text + 3, 2, minute) &&
             get_iso8601_seconds (text + 6, length - 6, seconds);
    }
  /* hhmmss(.sss) */
  else if (length >= 6)
    {
      return get_iso8601_int (text, 2, hour) &&
             get_iso8601_int (text + 2, 2, minute) &&
             get_iso8601_seconds (text + 4, length - 4, seconds);
    }
  else
    return false;
}

/**
 * sys_date_time_new_from_iso8601: (constructor)
 * @text: an ISO 8601 formatted time string.
 * @default_tz: (nullable): a #SysTimeZone to use if the text doesn't contain a
 *                          timezone, or %NULL.
 *
 * Creates a #SysDateTime corresponding to the given
 * [ISO 8601 formatted string](https://en.wikipedia.org/wiki/ISO_8601)
 * @text. ISO 8601 strings of the form <date><sep><time><tz> are supported, with
 * some extensions from [RFC 3339](https://tools.ietf.org/html/rfc3339) as
 * mentioned below.
 *
 * Note that as #SysDateTime "is oblivious to leap seconds", leap seconds information
 * in an ISO-8601 string will be ignored, so a `23:59:60` time would be parsed as
 * `23:59:59`.
 *
 * <sep> is the separator and can be either 'T', 't' or ' '. The latter two
 * separators are an extension from
 * [RFC 3339](https://tools.ietf.org/html/rfc3339#section-5.6).
 *
 * <date> is in the form:
 *
 * - `YYYY-MM-DD` - Year/month/day, e.g. 2016-08-24.
 * - `YYYYMMDD` - Same as above without dividers.
 * - `YYYY-DDD` - Ordinal day where DDD is from 001 to 366, e.g. 2016-237.
 * - `YYYYDDD` - Same as above without dividers.
 * - `YYYY-Www-D` - Week day where ww is from 01 to 52 and D from 1-7,
 *   e.g. 2016-W34-3.
 * - `YYYYWwwD` - Same as above without dividers.
 *
 * <time> is in the form:
 *
 * - `hh:mm:ss(.sss)` - Hours, minutes, seconds (subseconds), e.g. 22:10:42.123.
 * - `hhmmss(.sss)` - Same as above without dividers.
 *
 * <tz> is an optional timezone suffix of the form:
 *
 * - `Z` - UTC.
 * - `+hh:mm` or `-hh:mm` - Offset from UTC in hours and minutes, e.g. +12:00.
 * - `+hh` or `-hh` - Offset from UTC in hours, e.g. +12.
 *
 * If the timezone is not provided in @text it must be provided in @default_tz
 * (this field is otherwise ignored).
 *
 * This call can fail (returning %NULL) if @text is not a valid ISO 8601
 * formatted string.
 *
 * You should release the return value by calling sys_date_time_unref()
 * when you are done with it.
 *
 * Returns: (transfer full) (nullable): a new #SysDateTime, or %NULL
 *
 * Since: 2.56
 */
SysDateTime *
sys_date_time_new_from_iso8601 (const SysChar *text, SysTimeZone *default_tz)
{
  SysInt length, date_length = -1;
  SysInt hour = 0, minute = 0;
  SysDouble seconds = 0.0;
  SysTimeZone *tz = NULL;
  SysDateTime *datetime = NULL;

  sys_return_val_if_fail (text != NULL, NULL);

  /* Count length of string and find date / time separator ('T', 't', or ' ') */
  for (length = 0; text[length] != '\0'; length++)
    {
      if (date_length < 0 && (text[length] == 'T' || text[length] == 't' || text[length] == ' '))
        date_length = length;
    }

  if (date_length < 0)
    return NULL;

  if (!parse_iso8601_time (text + date_length + 1, length - (date_length + 1),
                           &hour, &minute, &seconds, &tz))
    goto out;
  if (tz == NULL && default_tz == NULL)
    return NULL;

  datetime = parse_iso8601_date (text, date_length, hour, minute, seconds, tz ? tz : default_tz);

out:
    if (tz != NULL)
      sys_time_zone_unref (tz);
    return datetime;
}

/* full new functions {{{1 */

/**
 * sys_date_time_new: (constructor)
 * @tz: a #SysTimeZone
 * @year: the year component of the date
 * @month: the month component of the date
 * @day: the day component of the date
 * @hour: the hour component of the date
 * @minute: the minute component of the date
 * @seconds: the number of seconds past the minute
 *
 * Creates a new #SysDateTime corresponding to the given date and time in
 * the time zone @tz.
 *
 * The @year must be between 1 and 9999, @month between 1 and 12 and @day
 * between 1 and 28, 29, 30 or 31 depending on the month and the year.
 *
 * @hour must be between 0 and 23 and @minute must be between 0 and 59.
 *
 * @seconds must be at least 0.0 and must be strictly less than 60.0.
 * It will be rounded down to the nearest microsecond.
 *
 * If the given time is not representable in the given time zone (for
 * example, 02:30 on March 14th 2010 in Toronto, due to daylight savings
 * time) then the time will be rounded up to the nearest existing time
 * (in this case, 03:00).  If this matters to you then you should verify
 * the return value for containing the same as the numbers you gave.
 *
 * In the case that the given time is ambiguous in the given time zone
 * (for example, 01:30 on November 7th 2010 in Toronto, due to daylight
 * savings time) then the time falling within standard (ie:
 * non-daylight) time is taken.
 *
 * It not considered a programmer error for the values to this function
 * to be out of range, but in the case that they are, the function will
 * return %NULL.
 *
 * You should release the return value by calling sys_date_time_unref()
 * when you are done with it.
 *
 * Returns: (transfer full) (nullable): a new #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_new (SysTimeZone *tz,
                 SysInt       year,
                 SysInt       month,
                 SysInt       day,
                 SysInt       hour,
                 SysInt       minute,
                 SysDouble    seconds)
{
  SysDateTime *datetime;
  SysInt64 full_time;
  /* keep these variables as volatile. We do not want them ending up in
   * registers - them doing so may cause us to hit precision problems on i386.
   * See: https://bugzilla.gnome.org/show_bug.cgi?id=792410 */
  volatile SysInt64 usec;
  volatile SysDouble usecd;

  sys_return_val_if_fail (tz != NULL, NULL);

  if (year < 1 || year > 9999 ||
      month < 1 || month > 12 ||
      day < 1 || day > days_in_months[GREGORIAN_LEAP (year)][month] ||
      hour < 0 || hour > 23 ||
      minute < 0 || minute > 59 ||
      isnan (seconds) ||
      seconds < 0.0 || seconds >= 60.0)
    return NULL;

  datetime = sys_date_time_alloc (tz);
  datetime->days = ymd_to_days (year, month, day);
  datetime->usec = (hour   * USEC_PER_HOUR)
                 + (minute * USEC_PER_MINUTE)
                 + (SysInt64) (seconds * USEC_PER_SECOND);

  full_time = SEC_PER_DAY *
                (ymd_to_days (year, month, day) - UNIX_EPOCH_START) +
              SECS_PER_HOUR * hour +
              SECS_PER_MINUTE * minute +
              (int) seconds;

  datetime->interval = sys_time_zone_adjust_time (datetime->tz,
                                                SYS_TIME_TYPE_STANDARD,
                                                &full_time);

  /* This is the correct way to convert a scaled FP value to integer.
   * If this surprises you, please observe that (int)(1.000001 * 1e6)
   * is 1000000.  This is not a problem with precision, it's just how
   * FP numbers work.
   * See https://bugzilla.gnome.org/show_bug.cgi?id=697715. */
  usec = (SysInt64)(seconds * USEC_PER_SECOND);
  usecd = (SysDouble)(usec + 1) * 1e-6;
  if (usecd <= seconds) {
    usec++;
  }

  full_time += UNIX_EPOCH_START * SEC_PER_DAY;
  datetime->days = (SysInt32)(full_time / SEC_PER_DAY);
  datetime->usec = (full_time % SEC_PER_DAY) * USEC_PER_SECOND;
  datetime->usec += usec % USEC_PER_SECOND;

  return datetime;
}

/**
 * sys_date_time_new_local: (constructor)
 * @year: the year component of the date
 * @month: the month component of the date
 * @day: the day component of the date
 * @hour: the hour component of the date
 * @minute: the minute component of the date
 * @seconds: the number of seconds past the minute
 *
 * Creates a new #SysDateTime corresponding to the given date and time in
 * the local time zone.
 *
 * This call is equivalent to calling sys_date_time_new() with the time
 * zone returned by sys_time_zone_new_local().
 *
 * Returns: (transfer full) (nullable): a #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_new_local (SysInt    year,
                       SysInt    month,
                       SysInt    day,
                       SysInt    hour,
                       SysInt    minute,
                       SysDouble seconds)
{
  SysDateTime *datetime;
  SysTimeZone *local;

  local = sys_time_zone_new_local ();
  datetime = sys_date_time_new (local, year, month, day, hour, minute, seconds);
  sys_time_zone_unref (local);

  return datetime;
}

/**
 * sys_date_time_new_utc: (constructor)
 * @year: the year component of the date
 * @month: the month component of the date
 * @day: the day component of the date
 * @hour: the hour component of the date
 * @minute: the minute component of the date
 * @seconds: the number of seconds past the minute
 *
 * Creates a new #SysDateTime corresponding to the given date and time in
 * UTC.
 *
 * This call is equivalent to calling sys_date_time_new() with the time
 * zone returned by sys_time_zone_new_utc().
 *
 * Returns: (transfer full) (nullable): a #SysDateTime, or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_new_utc (SysInt    year,
                     SysInt    month,
                     SysInt    day,
                     SysInt    hour,
                     SysInt    minute,
                     SysDouble seconds)
{
  SysDateTime *datetime;
  SysTimeZone *utc;

  utc = sys_time_zone_new_utc ();
  datetime = sys_date_time_new (utc, year, month, day, hour, minute, seconds);
  sys_time_zone_unref (utc);

  return datetime;
}

/* Adders {{{1 */

/**
 * sys_date_time_add:
 * @datetime: a #SysDateTime
 * @timespan: a #SysTimeSpan
 *
 * Creates a copy of @datetime and adds the specified timespan to the copy.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime*
sys_date_time_add (SysDateTime *datetime,
                 SysTimeSpan  timespan)
{
  sys_return_val_if_fail (datetime != NULL, NULL);

  return sys_date_time_from_instant (datetime->tz, timespan +
                                   sys_date_time_to_instant (datetime));
}

/**
 * sys_date_time_add_years:
 * @datetime: a #SysDateTime
 * @years: the number of years
 *
 * Creates a copy of @datetime and adds the specified number of years to the
 * copy. Add negative values to subtract years.
 *
 * As with sys_date_time_add_months(), if the resulting date would be 29th
 * February on a non-leap year, the day will be clamped to 28th February.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime *
sys_date_time_add_years (SysDateTime *datetime,
                       SysInt       years)
{
  SysInt year, month, day;

  sys_return_val_if_fail (datetime != NULL, NULL);

  if (years < -10000 || years > 10000)
    return NULL;

  sys_date_time_get_ymd (datetime, &year, &month, &day);
  year += years;

  /* only possible issue is if we've entered a year with no February 29
   */
  if (month == 2 && day == 29 && !GREGORIAN_LEAP (year))
    day = 28;

  return sys_date_time_replace_days (datetime, ymd_to_days (year, month, day));
}

/**
 * sys_date_time_add_months:
 * @datetime: a #SysDateTime
 * @months: the number of months
 *
 * Creates a copy of @datetime and adds the specified number of months to the
 * copy. Add negative values to subtract months.
 *
 * The day of the month of the resulting #SysDateTime is clamped to the number
 * of days in the updated calendar month. For example, if adding 1 month to
 * 31st January 2018, the result would be 28th February 2018. In 2020 (a leap
 * year), the result would be 29th February.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime*
sys_date_time_add_months (SysDateTime *datetime,
                        SysInt       months)
{
  SysInt year, month, day;

  sys_return_val_if_fail (datetime != NULL, NULL);
  sys_date_time_get_ymd (datetime, &year, &month, &day);

  if (months < -120000 || months > 120000)
    return NULL;

  year += months / 12;
  month += months % 12;
  if (month < 1)
    {
      month += 12;
      year--;
    }
  else if (month > 12)
    {
      month -= 12;
      year++;
    }

  day = min (day, days_in_months[GREGORIAN_LEAP (year)][month]);

  return sys_date_time_replace_days (datetime, ymd_to_days (year, month, day));
}

/**
 * sys_date_time_add_weeks:
 * @datetime: a #SysDateTime
 * @weeks: the number of weeks
 *
 * Creates a copy of @datetime and adds the specified number of weeks to the
 * copy. Add negative values to subtract weeks.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime*
sys_date_time_add_weeks (SysDateTime *datetime,
                       SysInt             weeks)
{
  sys_return_val_if_fail (datetime != NULL, NULL);

  return sys_date_time_add_days (datetime, weeks * 7);
}

/**
 * sys_date_time_add_days:
 * @datetime: a #SysDateTime
 * @days: the number of days
 *
 * Creates a copy of @datetime and adds the specified number of days to the
 * copy. Add negative values to subtract days.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime*
sys_date_time_add_days (SysDateTime *datetime,
                      SysInt       days)
{
  sys_return_val_if_fail (datetime != NULL, NULL);

  if (days < -3660000 || days > 3660000)
    return NULL;

  return sys_date_time_replace_days (datetime, datetime->days + days);
}

/**
 * sys_date_time_add_hours:
 * @datetime: a #SysDateTime
 * @hours: the number of hours to add
 *
 * Creates a copy of @datetime and adds the specified number of hours.
 * Add negative values to subtract hours.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime*
sys_date_time_add_hours (SysDateTime *datetime,
                       SysInt       hours)
{
  return sys_date_time_add (datetime, hours * USEC_PER_HOUR);
}

/**
 * sys_date_time_add_minutes:
 * @datetime: a #SysDateTime
 * @minutes: the number of minutes to add
 *
 * Creates a copy of @datetime adding the specified number of minutes.
 * Add negative values to subtract minutes.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime*
sys_date_time_add_minutes (SysDateTime *datetime,
                         SysInt             minutes)
{
  return sys_date_time_add (datetime, minutes * USEC_PER_MINUTE);
}


/**
 * sys_date_time_add_seconds:
 * @datetime: a #SysDateTime
 * @seconds: the number of seconds to add
 *
 * Creates a copy of @datetime and adds the specified number of seconds.
 * Add negative values to subtract seconds.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime*
sys_date_time_add_seconds (SysDateTime *datetime,
                         SysDouble    seconds)
{
  return sys_date_time_add (datetime, (SysTimeSpan)(seconds * USEC_PER_SECOND));
}

/**
 * sys_date_time_add_full:
 * @datetime: a #SysDateTime
 * @years: the number of years to add
 * @months: the number of months to add
 * @days: the number of days to add
 * @hours: the number of hours to add
 * @minutes: the number of minutes to add
 * @seconds: the number of seconds to add
 *
 * Creates a new #SysDateTime adding the specified values to the current date and
 * time in @datetime. Add negative values to subtract.
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 */
SysDateTime *
sys_date_time_add_full (SysDateTime *datetime,
                      SysInt       years,
                      SysInt       months,
                      SysInt       days,
                      SysInt       hours,
                      SysInt       minutes,
                      SysDouble    seconds)
{
  SysInt year, month, day;
  SysInt64 full_time;
  SysDateTime *new;
  SysInt interval;

  sys_return_val_if_fail (datetime != NULL, NULL);
  sys_date_time_get_ymd (datetime, &year, &month, &day);

  months += years * 12;

  if (months < -120000 || months > 120000)
    return NULL;

  if (days < -3660000 || days > 3660000)
    return NULL;

  year += months / 12;
  month += months % 12;
  if (month < 1)
    {
      month += 12;
      year--;
    }
  else if (month > 12)
    {
      month -= 12;
      year++;
    }

  day = min (day, days_in_months[GREGORIAN_LEAP (year)][month]);

  /* full_time is now in unix (local) time */
  full_time = datetime->usec / USEC_PER_SECOND + SEC_PER_DAY *
    (ymd_to_days (year, month, day) + days - UNIX_EPOCH_START);

  interval = sys_time_zone_adjust_time (datetime->tz,
                                      sys_time_zone_is_dst (datetime->tz,
                                                          datetime->interval),
                                      &full_time);

  /* move to UTC unix time */
  full_time -= sys_time_zone_get_offset (datetime->tz, interval);

  /* convert back to an instant, add back fractional seconds */
  full_time += UNIX_EPOCH_START * SEC_PER_DAY;
  full_time = full_time * USEC_PER_SECOND +
              datetime->usec % USEC_PER_SECOND;

  /* do the actual addition now */
  full_time += (hours * USEC_PER_HOUR) +
               (minutes * USEC_PER_MINUTE) +
               (SysInt64) (seconds * USEC_PER_SECOND);

  /* find the new interval */
  interval = sys_time_zone_find_interval (datetime->tz,
                                        SYS_TIME_TYPE_UNIVERSAL,
                                        INSTANT_TO_UNIX (full_time));

  /* convert back into local time */
  full_time += USEC_PER_SECOND *
               sys_time_zone_get_offset (datetime->tz, interval);

  /* split into days and usec of a new datetime */
  new = sys_date_time_alloc (datetime->tz);
  new->interval = interval;
  new->days = (SysInt32)(full_time / USEC_PER_DAY);
  new->usec = full_time % USEC_PER_DAY;

  /* XXX validate */

  return new;
}

/* Compare, difference, hash, equal {{{1 */
/**
 * sys_date_time_compare:
 * @dt1: (type SysDateTime) (not nullable): first #SysDateTime to compare
 * @dt2: (type SysDateTime) (not nullable): second #SysDateTime to compare
 *
 * A comparison function for #SysDateTimes that is suitable
 * as a #GCompareFunc. Both #SysDateTimes must be non-%NULL.
 *
 * Returns: -1, 0 or 1 if @dt1 is less than, equal to or greater
 *   than @dt2.
 *
 * Since: 2.26
 */
SysInt
sys_date_time_compare (const SysPointer dt1,
                     const SysPointer dt2)
{
  SysInt64 difference;

  difference = sys_date_time_difference ((SysDateTime *) dt1, (SysDateTime *) dt2);

  if (difference < 0)
    return -1;

  else if (difference > 0)
    return 1;

  else
    return 0;
}

/**
 * sys_date_time_difference:
 * @end: a #SysDateTime
 * @begin: a #SysDateTime
 *
 * Calculates the difference in time between @end and @begin.  The
 * #SysTimeSpan that is returned is effectively @end - @begin (ie:
 * positive if the first parameter is larger).
 *
 * Returns: the difference between the two #SysDateTime, as a time
 *   span expressed in microseconds.
 *
 * Since: 2.26
 */
SysTimeSpan
sys_date_time_difference (SysDateTime *end,
                        SysDateTime *begin)
{
  sys_return_val_if_fail (begin != NULL, 0);
  sys_return_val_if_fail (end != NULL, 0);

  return sys_date_time_to_instant (end) -
         sys_date_time_to_instant (begin);
}

/**
 * sys_date_time_hash:
 * @datetime: (type SysDateTime) (not nullable): a #SysDateTime
 *
 * Hashes @datetime into a #SysUInt, suitable for use within #SysHashTable.
 *
 * Returns: a #SysUInt containing the hash
 *
 * Since: 2.26
 */
SysUInt
sys_date_time_hash (const SysPointer datetime)
{
  sys_return_val_if_fail (datetime != NULL, 0);

  return (SysUInt)sys_date_time_to_instant ((SysDateTime *) datetime);
}

/**
 * sys_date_time_equal:
 * @dt1: (type SysDateTime) (not nullable): a #SysDateTime
 * @dt2: (type SysDateTime) (not nullable): a #SysDateTime
 *
 * Checks to see if @dt1 and @dt2 are equal.
 *
 * Equal here means that they represent the same moment after converting
 * them to the same time zone.
 *
 * Returns: %true if @dt1 and @dt2 are equal
 *
 * Since: 2.26
 */
SysBool
sys_date_time_equal (const SysPointer dt1,
                   const SysPointer dt2)
{
  return sys_date_time_difference ((SysDateTime *) dt1, (SysDateTime *) dt2) == 0;
}

/* Year, Month, Day Getters {{{1 */
/**
 * sys_date_time_get_ymd:
 * @datetime: a #SysDateTime.
 * @year: (out) (optional): the return location for the gregorian year, or %NULL.
 * @month: (out) (optional): the return location for the month of the year, or %NULL.
 * @day: (out) (optional): the return location for the day of the month, or %NULL.
 *
 * Retrieves the Gregorian day, month, and year of a given #SysDateTime.
 *
 * Since: 2.26
 **/
void
sys_date_time_get_ymd (SysDateTime *datetime,
                     SysInt      *year,
                     SysInt      *month,
                     SysInt      *day)
{
  SysInt the_year;
  SysInt the_month;
  SysInt the_day;
  SysInt remaining_days;
  SysInt y100_cycles;
  SysInt y4_cycles;
  SysInt y1_cycles;
  SysInt preceding;
  SysBool leap;

  sys_return_if_fail (datetime != NULL);

  remaining_days = datetime->days;

  /*
   * We need to convert an offset in days to its year/month/day representation.
   * Leap years makes this a little trickier than it should be, so we use
   * 400, 100 and 4 years cycles here to get to the correct year.
   */

  /* Our days offset starts sets 0001-01-01 as day 1, if it was day 0 our
   * math would be simpler, so let's do it */
  remaining_days--;

  the_year = (remaining_days / DAYS_IN_400YEARS) * 400 + 1;
  remaining_days = remaining_days % DAYS_IN_400YEARS;

  y100_cycles = remaining_days / DAYS_IN_100YEARS;
  remaining_days = remaining_days % DAYS_IN_100YEARS;
  the_year += y100_cycles * 100;

  y4_cycles = remaining_days / DAYS_IN_4YEARS;
  remaining_days = remaining_days % DAYS_IN_4YEARS;
  the_year += y4_cycles * 4;

  y1_cycles = remaining_days / 365;
  the_year += y1_cycles;
  remaining_days = remaining_days % 365;

  if (y1_cycles == 4 || y100_cycles == 4) {
    sys_assert (remaining_days == 0);

    /* special case that indicates that the date is actually one year before,
     * in the 31th of December */
    the_year--;
    the_month = 12;
    the_day = 31;
    goto end;
  }

  /* now get the month and the day */
  leap = y1_cycles == 3 && (y4_cycles != 24 || y100_cycles == 3);

  sys_assert (leap == GREGORIAN_LEAP(the_year));

  the_month = (remaining_days + 50) >> 5;
  preceding = (days_in_year[0][the_month - 1] + (the_month > 2 && leap));
  if (preceding > remaining_days)
    {
      /* estimate is too large */
      the_month -= 1;
      preceding -= leap ? days_in_months[1][the_month]
                        : days_in_months[0][the_month];
    }

  remaining_days -= preceding;
  sys_assert(0 <= remaining_days);

  the_day = remaining_days + 1;

end:
  if (year)
    *year = the_year;
  if (month)
    *month = the_month;
  if (day)
    *day = the_day;
}

/**
 * sys_date_time_get_year:
 * @datetime: A #SysDateTime
 *
 * Retrieves the year represented by @datetime in the Gregorian calendar.
 *
 * Returns: the year represented by @datetime
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_year (SysDateTime *datetime)
{
  SysInt year;

  sys_return_val_if_fail (datetime != NULL, 0);

  sys_date_time_get_ymd (datetime, &year, NULL, NULL);

  return year;
}

/**
 * sys_date_time_get_month:
 * @datetime: a #SysDateTime
 *
 * Retrieves the month of the year represented by @datetime in the Gregorian
 * calendar.
 *
 * Returns: the month represented by @datetime
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_month (SysDateTime *datetime)
{
  SysInt month;

  sys_return_val_if_fail (datetime != NULL, 0);

  sys_date_time_get_ymd (datetime, NULL, &month, NULL);

  return month;
}

/**
 * sys_date_time_get_day_of_month:
 * @datetime: a #SysDateTime
 *
 * Retrieves the day of the month represented by @datetime in the gregorian
 * calendar.
 *
 * Returns: the day of the month
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_day_of_month (SysDateTime *datetime)
{
  SysInt           day_of_year,
                 i;
  SysUInt          is_leap;
  SysUInt16        last = 0;

  sys_return_val_if_fail (datetime != NULL, 0);

  is_leap = GREGORIAN_LEAP (sys_date_time_get_year (datetime)) ? 1 : 0;
  sys_date_time_get_week_number (datetime, NULL, NULL, &day_of_year);

  for (i = 1; i <= 12; i++)
    {
      if (days_in_year[is_leap][i] >= day_of_year)
        return day_of_year - last;
      last = days_in_year[is_leap][i];
    }

  sys_warn_if_reached ();
  return 0;
}

/* Week of year / day of week getters {{{1 */
/**
 * sys_date_time_get_week_numbering_year:
 * @datetime: a #SysDateTime
 *
 * Returns the ISO 8601 week-numbering year in which the week containing
 * @datetime falls.
 *
 * This function, taken together with sys_date_time_get_week_of_year() and
 * sys_date_time_get_day_of_week() can be used to determine the full ISO
 * week date on which @datetime falls.
 *
 * This is usually equal to the normal Gregorian year (as returned by
 * sys_date_time_get_year()), except as detailed below:
 *
 * For Thursday, the week-numbering year is always equal to the usual
 * calendar year.  For other days, the number is such that every day
 * within a complete week (Monday to Sunday) is contained within the
 * same week-numbering year.
 *
 * For Monday, Tuesday and Wednesday occurring near the end of the year,
 * this may mean that the week-numbering year is one greater than the
 * calendar year (so that these days have the same week-numbering year
 * as the Thursday occurring early in the next year).
 *
 * For Friday, Saturday and Sunday occurring near the start of the year,
 * this may mean that the week-numbering year is one less than the
 * calendar year (so that these days have the same week-numbering year
 * as the Thursday occurring late in the previous year).
 *
 * An equivalent description is that the week-numbering year is equal to
 * the calendar year containing the majority of the days in the current
 * week (Monday to Sunday).
 *
 * Note that January 1 0001 in the proleptic Gregorian calendar is a
 * Monday, so this function never returns 0.
 *
 * Returns: the ISO 8601 week-numbering year for @datetime
 *
 * Since: 2.26
 **/
SysInt
sys_date_time_get_week_numbering_year (SysDateTime *datetime)
{
  SysInt year = -1, month = -1, day = -1, weekday;

  sys_date_time_get_ymd (datetime, &year, &month, &day);
  weekday = sys_date_time_get_day_of_week (datetime);

  /* January 1, 2, 3 might be in the previous year if they occur after
   * Thursday.
   *
   *   Jan 1:  Friday, Saturday, Sunday    =>  day 1:  weekday 5, 6, 7
   *   Jan 2:  Saturday, Sunday            =>  day 2:  weekday 6, 7
   *   Jan 3:  Sunday                      =>  day 3:  weekday 7
   *
   * So we have a special case if (day - weekday) <= -4
   */
  if (month == 1 && (day - weekday) <= -4)
    return year - 1;

  /* December 29, 30, 31 might be in the next year if they occur before
   * Thursday.
   *
   *   Dec 31: Monday, Tuesday, Wednesday  =>  day 31: weekday 1, 2, 3
   *   Dec 30: Monday, Tuesday             =>  day 30: weekday 1, 2
   *   Dec 29: Monday                      =>  day 29: weekday 1
   *
   * So we have a special case if (day - weekday) >= 28
   */
  else if (month == 12 && (day - weekday) >= 28)
    return year + 1;

  else
    return year;
}

/**
 * sys_date_time_get_week_of_year:
 * @datetime: a #SysDateTime
 *
 * Returns the ISO 8601 week number for the week containing @datetime.
 * The ISO 8601 week number is the same for every day of the week (from
 * Moday through Sunday).  That can produce some unusual results
 * (described below).
 *
 * The first week of the year is week 1.  This is the week that contains
 * the first Thursday of the year.  Equivalently, this is the first week
 * that has more than 4 of its days falling within the calendar year.
 *
 * The value 0 is never returned by this function.  Days contained
 * within a year but occurring before the first ISO 8601 week of that
 * year are considered as being contained in the last week of the
 * previous year.  Similarly, the final days of a calendar year may be
 * considered as being part of the first ISO 8601 week of the next year
 * if 4 or more days of that week are contained within the new year.
 *
 * Returns: the ISO 8601 week number for @datetime.
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_week_of_year (SysDateTime *datetime)
{
  SysInt weeknum;

  sys_return_val_if_fail (datetime != NULL, 0);

  sys_date_time_get_week_number (datetime, &weeknum, NULL, NULL);

  return weeknum;
}

/**
 * sys_date_time_get_day_of_week:
 * @datetime: a #SysDateTime
 *
 * Retrieves the ISO 8601 day of the week on which @datetime falls (1 is
 * Monday, 2 is Tuesday... 7 is Sunday).
 *
 * Returns: the day of the week
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_day_of_week (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, 0);

  return (datetime->days - 1) % 7 + 1;
}

/* Day of year getter {{{1 */
/**
 * sys_date_time_get_day_of_year:
 * @datetime: a #SysDateTime
 *
 * Retrieves the day of the year represented by @datetime in the Gregorian
 * calendar.
 *
 * Returns: the day of the year
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_day_of_year (SysDateTime *datetime)
{
  SysInt doy = 0;

  sys_return_val_if_fail (datetime != NULL, 0);

  sys_date_time_get_week_number (datetime, NULL, NULL, &doy);
  return doy;
}

/* Time component getters {{{1 */

/**
 * sys_date_time_get_hour:
 * @datetime: a #SysDateTime
 *
 * Retrieves the hour of the day represented by @datetime
 *
 * Returns: the hour of the day
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_hour (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, 0);

  return (SysInt)(datetime->usec / USEC_PER_HOUR);
}

/**
 * sys_date_time_get_minute:
 * @datetime: a #SysDateTime
 *
 * Retrieves the minute of the hour represented by @datetime
 *
 * Returns: the minute of the hour
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_minute (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, 0);

  return (datetime->usec % USEC_PER_HOUR) / USEC_PER_MINUTE;
}

/**
 * sys_date_time_get_second:
 * @datetime: a #SysDateTime
 *
 * Retrieves the second of the minute represented by @datetime
 *
 * Returns: the second represented by @datetime
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_second (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, 0);

  return (datetime->usec % USEC_PER_MINUTE) / USEC_PER_SECOND;
}

/**
 * sys_date_time_get_microsecond:
 * @datetime: a #SysDateTime
 *
 * Retrieves the microsecond of the date represented by @datetime
 *
 * Returns: the microsecond of the second
 *
 * Since: 2.26
 */
SysInt
sys_date_time_get_microsecond (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, 0);

  return (datetime->usec % USEC_PER_SECOND);
}

/**
 * sys_date_time_get_seconds:
 * @datetime: a #SysDateTime
 *
 * Retrieves the number of seconds since the start of the last minute,
 * including the fractional part.
 *
 * Returns: the number of seconds
 *
 * Since: 2.26
 **/
SysDouble
sys_date_time_get_seconds (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, 0);

  return (datetime->usec % USEC_PER_MINUTE) / 1000000.0;
}

/* Exporters {{{1 */
/**
 * sys_date_time_to_unix:
 * @datetime: a #SysDateTime
 *
 * Gives the Unix time corresponding to @datetime, rounding down to the
 * nearest second.
 *
 * Unix time is the number of seconds that have elapsed since 1970-01-01
 * 00:00:00 UTC, regardless of the time zone associated with @datetime.
 *
 * Returns: the Unix time corresponding to @datetime
 *
 * Since: 2.26
 **/
SysInt64
sys_date_time_to_unix (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, 0);

  return INSTANT_TO_UNIX (sys_date_time_to_instant (datetime));
}

/* Timezone queries {{{1 */
/**
 * sys_date_time_get_utc_offset:
 * @datetime: a #SysDateTime
 *
 * Determines the offset to UTC in effect at the time and in the time
 * zone of @datetime.
 *
 * The offset is the number of microseconds that you add to UTC time to
 * arrive at local time for the time zone (ie: negative numbers for time
 * zones west of GMT, positive numbers for east).
 *
 * If @datetime represents UTC time, then the offset is always zero.
 *
 * Returns: the number of microseconds that should be added to UTC to
 *          get the local time
 *
 * Since: 2.26
 **/
SysTimeSpan
sys_date_time_get_utc_offset (SysDateTime *datetime)
{
  SysInt offset;

  sys_return_val_if_fail (datetime != NULL, 0);

  offset = sys_time_zone_get_offset (datetime->tz, datetime->interval);

  return (SysInt64) offset * USEC_PER_SECOND;
}

/**
 * sys_date_time_get_timezone:
 * @datetime: a #SysDateTime
 *
 * Get the time zone for this @datetime.
 *
 * Returns: (transfer none): the time zone
 * Since: 2.58
 */
SysTimeZone *
sys_date_time_get_timezone (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, NULL);

  sys_assert (datetime->tz != NULL);
  return datetime->tz;
}

/**
 * sys_date_time_get_timezone_abbreviation:
 * @datetime: a #SysDateTime
 *
 * Determines the time zone abbreviation to be used at the time and in
 * the time zone of @datetime.
 *
 * For example, in Toronto this is currently "EST" during the winter
 * months and "EDT" during the summer months when daylight savings
 * time is in effect.
 *
 * Returns: (transfer none): the time zone abbreviation. The returned
 *          string is owned by the #SysDateTime and it should not be
 *          modified or freed
 *
 * Since: 2.26
 **/
const SysChar *
sys_date_time_get_timezone_abbreviation (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, NULL);

  return sys_time_zone_get_abbreviation (datetime->tz, datetime->interval);
}

/**
 * sys_date_time_is_daylight_savings:
 * @datetime: a #SysDateTime
 *
 * Determines if daylight savings time is in effect at the time and in
 * the time zone of @datetime.
 *
 * Returns: %true if daylight savings time is in effect
 *
 * Since: 2.26
 **/
SysBool
sys_date_time_is_daylight_savings (SysDateTime *datetime)
{
  sys_return_val_if_fail (datetime != NULL, false);

  return sys_time_zone_is_dst (datetime->tz, datetime->interval);
}

/* Timezone convert {{{1 */
/**
 * sys_date_time_to_timezone:
 * @datetime: a #SysDateTime
 * @tz: the new #SysTimeZone
 *
 * Create a new #SysDateTime corresponding to the same instant in time as
 * @datetime, but in the time zone @tz.
 *
 * This call can fail in the case that the time goes out of bounds.  For
 * example, converting 0001-01-01 00:00:00 UTC to a time zone west of
 * Greenwich will fail (due to the year 0 being out of range).
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_to_timezone (SysDateTime *datetime,
                         SysTimeZone *tz)
{
  sys_return_val_if_fail (datetime != NULL, NULL);
  sys_return_val_if_fail (tz != NULL, NULL);

  return sys_date_time_from_instant (tz, sys_date_time_to_instant (datetime));
}

/**
 * sys_date_time_to_local:
 * @datetime: a #SysDateTime
 *
 * Creates a new #SysDateTime corresponding to the same instant in time as
 * @datetime, but in the local time zone.
 *
 * This call is equivalent to calling sys_date_time_to_timezone() with the
 * time zone returned by sys_time_zone_new_local().
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_to_local (SysDateTime *datetime)
{
  SysDateTime *new;
  SysTimeZone *local;

  local = sys_time_zone_new_local ();
  new = sys_date_time_to_timezone (datetime, local);
  sys_time_zone_unref (local);

  return new;
}

/**
 * sys_date_time_to_utc:
 * @datetime: a #SysDateTime
 *
 * Creates a new #SysDateTime corresponding to the same instant in time as
 * @datetime, but in UTC.
 *
 * This call is equivalent to calling sys_date_time_to_timezone() with the
 * time zone returned by sys_time_zone_new_utc().
 *
 * Returns: (transfer full) (nullable): the newly created #SysDateTime which
 *   should be freed with sys_date_time_unref(), or %NULL
 *
 * Since: 2.26
 **/
SysDateTime *
sys_date_time_to_utc (SysDateTime *datetime)
{
  SysDateTime *new;
  SysTimeZone *utc;

  utc = sys_time_zone_new_utc ();
  new = sys_date_time_to_timezone (datetime, utc);
  sys_time_zone_unref (utc);

  return new;
}

/* Format {{{1 */

static SysBool format_z (SysString *outstr,
          SysInt     offset,
          SysUInt    colons) {
  SysInt hours;
  SysInt minutes;
  SysInt seconds;
  SysChar sign = offset >= 0 ? '+' : '-';

  offset = abs (offset);
  hours = offset / 3600;
  minutes = offset / 60 % 60;
  seconds = offset % 60;

  switch (colons)
    {
    case 0:
      sys_string_append_printf (outstr, "%c%02d%02d",
                              sign,
                              hours,
                              minutes);
      break;

    case 1:
      sys_string_append_printf (outstr, "%c%02d:%02d",
                              sign,
                              hours,
                              minutes);
      break;

    case 2:
      sys_string_append_printf (outstr, "%c%02d:%02d:%02d",
                              sign,
                              hours,
                              minutes,
                              seconds);
      break;

    case 3:
      sys_string_append_printf (outstr, "%c%02d", sign, hours);

      if (minutes != 0 || seconds != 0)
        {
          sys_string_append_printf (outstr, ":%02d", minutes);

          if (seconds != 0)
            sys_string_append_printf (outstr, ":%02d", seconds);
        }
      break;

    default:
      return false;
    }

  return true;
}

#ifdef HAVE_LANGINFO_OUTDIGIT
/* Initializes the array with UTF-8 encoded alternate digits suitable for use
 * in current locale. Returns NULL when current locale does not use alternate
 * digits or there was an error converting them to UTF-8.
 */
static const SysChar * const *
initialize_alt_digits (void)
{
  SysUInt i;
  SysSSize digit_len;
  SysChar *digit;
  const SysChar *locale_digit;
#define N_DIGITS 10
#define max_UTF8_ENCODING_LEN 4
  static SysChar buffer[N_DIGITS * (MAX_UTF8_ENCODING_LEN + 1 /* null separator */)];
#undef N_DIGITS
#undef max_UTF8_ENCODING_LEN
  SysChar *buffer_end = buffer;
  static const SysChar *alt_digits[10];

  for (i = 0; i != 10; ++i)
    {
      locale_digit = nl_langinfo (_NL_CTYPE_OUTDIGIT0_MB + i);

      if (sys_str_equal (locale_digit, "") == 0)
        return NULL;

      digit = _g_ctype_locale_to_utf8 (locale_digit, -1, NULL, &digit_len, NULL);
      if (digit == NULL)
        return NULL;

      sys_assert (digit_len < (SysSSize) (buffer + sizeof (buffer) - buffer_end));

      alt_digits[i] = buffer_end;
      buffer_end = sys_stpcpy (buffer_end, digit);
      /* skip trailing null byte */
      buffer_end += 1;

      sys_free (digit);
    }

  return alt_digits;
}
#endif /* HAVE_LANGINFO_OUTDIGIT */

static void format_number (SysString     *str,
               SysBool     use_alt_digits,
               const SysChar *pad,
               SysInt         width,
               SysUInt32      number) {
  const SysChar *ascii_digits[10] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"
  };
  const SysChar * const *digits = ascii_digits;
  const SysChar *tmp[10];
  SysInt i = 0;

  sys_return_if_fail (width <= 10);

#ifdef HAVE_LANGINFO_OUTDIGIT
  if (use_alt_digits)
    {
      static const SysChar * const *alt_digits = NULL;
      static SysSSize initialised;

      if SYS_UNLIKELY (sys_once_init_enter (&initialised))
        {
          alt_digits = initialize_alt_digits ();

          if (alt_digits == NULL)
            alt_digits = ascii_digits;

          sys_once_init_leave (&initialised, true);
        }

      digits = alt_digits;
    }
#endif /* HAVE_LANGINFO_OUTDIGIT */

  do
    {
      tmp[i++] = digits[number % 10];
      number /= 10;
    }
  while (number);

  while (pad && i < width)
    tmp[i++] = *pad == '0' ? digits[0] : pad;

  /* should really be impossible */
  sys_assert (i <= 10);

  while (i)
    sys_string_append (str, tmp[--i]);
}

static SysBool format_ampm (SysDateTime *datetime,
             SysString   *outstr,
             SysBool   locale_is_utf8,
             SysBool   uppercase) {
  const SysChar *ampm;
  SysChar       *tmp = NULL, *ampm_dup;

  ampm = GET_AMPM (datetime);

  if (!ampm || ampm[0] == '\0')
    ampm = get_fallback_ampm (sys_date_time_get_hour (datetime));

  if (!locale_is_utf8 && GET_AMPM_IS_LOCALE)
    {
      /* This assumes that locale encoding can't have embedded NULs */
      ampm = tmp = sys_locale_to_utf8 (ampm, -1, NULL, NULL, NULL);
      if (tmp == NULL)
        return false;
    }
  if (uppercase)
    ampm_dup = sys_utf8_strup (ampm, -1);
  else
    ampm_dup = sys_utf8_strdown (ampm, -1);
  sys_free (tmp);

  sys_string_append (outstr, ampm_dup);
  sys_free (ampm_dup);

  return true;
}

static SysBool sys_date_time_format_utf8 (SysDateTime   *datetime,
    const SysChar *utf8_format,
    SysString     *outstr,
    SysBool     locale_is_utf8);


/* sys_date_time_format() subroutine that takes a locale-encoded format
 * string and produces a UTF-8 encoded date/time string.
 */
static SysBool
sys_date_time_format_locale (SysDateTime   *datetime,
			   const SysChar *locale_format,
			   SysString     *outstr,
			   SysBool     locale_is_utf8)
{
  SysChar *utf8_format;
  SysBool success;

  if (locale_is_utf8)
    return sys_date_time_format_utf8 (datetime, locale_format, outstr, locale_is_utf8);

  utf8_format = _sys_time_locale_to_utf8 (locale_format, -1, NULL, NULL, NULL);
  if (utf8_format == NULL)
    return false;

  success = sys_date_time_format_utf8 (datetime, utf8_format, outstr,
                                     locale_is_utf8);
  sys_free (utf8_format);
  return success;
}

static inline SysBool
string_append (SysString     *string,
               const SysChar *s,
               SysBool     s_is_utf8)
{
  SysChar *utf8;
  SysSSize  utf8_len;

  if (s_is_utf8)
    {
      sys_string_append (string, s);
    }
  else
    {
      utf8 = _sys_time_locale_to_utf8 (s, -1, NULL, &utf8_len, NULL);
      if (utf8 == NULL)
        return false;
      sys_string_append_len (string, utf8, utf8_len);
      sys_free (utf8);
    }

  return true;
}

/* sys_date_time_format() subroutine that takes a UTF-8 encoded format
 * string and produces a UTF-8 encoded date/time string.
 */
static SysBool sys_date_time_format_utf8 (SysDateTime   *datetime,
    const SysChar *utf8_format,
    SysString     *outstr,
    SysBool     locale_is_utf8) {
  SysUInt     len;
  SysUInt     colons;
  SysUniChar  c;
  SysBool  alt_digits = false;
  SysBool  pad_set = false;
  SysBool  name_is_utf8;
  const SysChar *pad = "";
  const SysChar *name;
  const SysChar *tz;

  while (*utf8_format)
    {
      len = (SysUInt)strcspn (utf8_format, "%");
      if (len)
        sys_string_append_len (outstr, utf8_format, len);

      utf8_format += len;
      if (!*utf8_format)
        break;

      sys_assert (*utf8_format == '%');
      utf8_format++;
      if (!*utf8_format)
        break;

      colons = 0;
      alt_digits = false;
      pad_set = false;

next_mod:
      c = sys_utf8_get_char (utf8_format);
      utf8_format = sys_utf8_next_char (utf8_format);
      switch (c)
      {
        case 'a':
          name = WEEKDAY_ABBR (datetime);
          if (sys_str_equal (name, ""))
            return false;

          name_is_utf8 = locale_is_utf8 || !WEEKDAY_ABBR_IS_LOCALE;

          if (!string_append (outstr, name, name_is_utf8))
            return false;

          break;
        case 'A':
          name = WEEKDAY_FULL (datetime);
          if (sys_str_equal (name, "") == 0)
            return false;

          name_is_utf8 = locale_is_utf8 || !WEEKDAY_FULL_IS_LOCALE;

          if (!string_append (outstr, name, name_is_utf8))
            return false;

          break;
        case 'b':
          name = alt_digits ? MONTH_ABBR_STANDALONE (datetime)
            : MONTH_ABBR_WITH_DAY (datetime);
          if (sys_str_equal (name, "") == 0)
            return false;

          name_is_utf8 = locale_is_utf8 ||
            ((alt_digits && !MONTH_ABBR_STANDALONE_IS_LOCALE) ||
             (!alt_digits && !MONTH_ABBR_WITH_DAY_IS_LOCALE));

          if (!string_append (outstr, name, name_is_utf8))
            return false;

          break;
        case 'B':
          name = alt_digits ? MONTH_FULL_STANDALONE (datetime)
            : MONTH_FULL_WITH_DAY (datetime);
          if (sys_str_equal (name, "") == 0)
            return false;

          name_is_utf8 = locale_is_utf8 ||
            ((alt_digits && !MONTH_FULL_STANDALONE_IS_LOCALE) ||
             (!alt_digits && !MONTH_FULL_WITH_DAY_IS_LOCALE));

          if (!string_append (outstr, name, name_is_utf8))
            return false;

          break;
        case 'c':
          {
            if (sys_str_equal (PREFERRED_DATE_TIME_FMT, "") == 0)
              return false;
            if (!sys_date_time_format_locale (datetime, PREFERRED_DATE_TIME_FMT,
                  outstr, locale_is_utf8))
              return false;
          }
          break;
        case 'C':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_year (datetime) / 100);
          break;
        case 'd':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_day_of_month (datetime));
          break;
        case 'e':
          format_number (outstr, alt_digits, pad_set ? pad : "\u2007", 2,
              sys_date_time_get_day_of_month (datetime));
          break;
        case 'f':
          sys_string_append_printf (outstr, "%06" SYS_UINT64_FORMAT,
              datetime->usec % SYS_TIME_SPAN_SECOND);
          break;
        case 'F':
          sys_string_append_printf (outstr, "%d-%02d-%02d",
              sys_date_time_get_year (datetime),
              sys_date_time_get_month (datetime),
              sys_date_time_get_day_of_month (datetime));
          break;
        case 'g':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_week_numbering_year (datetime) % 100);
          break;
        case 'G':
          format_number (outstr, alt_digits, pad_set ? pad : 0, 0,
              sys_date_time_get_week_numbering_year (datetime));
          break;
        case 'h':
          name = alt_digits ? MONTH_ABBR_STANDALONE (datetime)
            : MONTH_ABBR_WITH_DAY (datetime);
          if (sys_str_equal (name, "") == 0)
            return false;

          name_is_utf8 = locale_is_utf8 ||
            ((alt_digits && !MONTH_ABBR_STANDALONE_IS_LOCALE) ||
             (!alt_digits && !MONTH_ABBR_WITH_DAY_IS_LOCALE));

          if (!string_append (outstr, name, name_is_utf8))
            return false;

          break;
        case 'H':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_hour (datetime));
          break;
        case 'I':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              (sys_date_time_get_hour (datetime) + 11) % 12 + 1);
          break;
        case 'j':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 3,
              sys_date_time_get_day_of_year (datetime));
          break;
        case 'k':
          format_number (outstr, alt_digits, pad_set ? pad : "\u2007", 2,
              sys_date_time_get_hour (datetime));
          break;
        case 'l':
          format_number (outstr, alt_digits, pad_set ? pad : "\u2007", 2,
              (sys_date_time_get_hour (datetime) + 11) % 12 + 1);
          break;
        case 'm':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_month (datetime));
          break;
        case 'M':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_minute (datetime));
          break;
        case 'n':
          sys_string_append_c (outstr, '\n');
          break;
        case 'O':
          alt_digits = true;
          goto next_mod;
        case 'p':
          if (!format_ampm (datetime, outstr, locale_is_utf8, true))
            return false;
          break;
        case 'P':
          if (!format_ampm (datetime, outstr, locale_is_utf8, false))
            return false;
          break;
        case 'r':
          {
            if (sys_str_equal (PREFERRED_12HR_TIME_FMT, "") == 0)
              return false;
            if (!sys_date_time_format_locale (datetime, PREFERRED_12HR_TIME_FMT,
                  outstr, locale_is_utf8))
              return false;
          }
          break;
        case 'R':
          sys_string_append_printf (outstr, "%02d:%02d",
              sys_date_time_get_hour (datetime),
              sys_date_time_get_minute (datetime));
          break;
        case 's':
          sys_string_append_printf (outstr, "%" SYS_INT64_FORMAT, sys_date_time_to_unix (datetime));
          break;
        case 'S':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_second (datetime));
          break;
        case 't':
          sys_string_append_c (outstr, '\t');
          break;
        case 'T':
          sys_string_append_printf (outstr, "%02d:%02d:%02d",
              sys_date_time_get_hour (datetime),
              sys_date_time_get_minute (datetime),
              sys_date_time_get_second (datetime));
          break;
        case 'u':
          format_number (outstr, alt_digits, 0, 0,
              sys_date_time_get_day_of_week (datetime));
          break;
        case 'V':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_week_of_year (datetime));
          break;
        case 'w':
          format_number (outstr, alt_digits, 0, 0,
              sys_date_time_get_day_of_week (datetime) % 7);
          break;
        case 'x':
          {
            if (sys_str_equal (PREFERRED_DATE_FMT, "") == 0)
              return false;
            if (!sys_date_time_format_locale (datetime, PREFERRED_DATE_FMT,
                  outstr, locale_is_utf8))
              return false;
          }
          break;
        case 'X':
          {
            if (sys_str_equal (PREFERRED_TIME_FMT, "") == 0)
              return false;
            if (!sys_date_time_format_locale (datetime, PREFERRED_TIME_FMT,
                  outstr, locale_is_utf8))
              return false;
          }
          break;
        case 'y':
          format_number (outstr, alt_digits, pad_set ? pad : "0", 2,
              sys_date_time_get_year (datetime) % 100);
          break;
        case 'Y':
          format_number (outstr, alt_digits, 0, 0,
              sys_date_time_get_year (datetime));
          break;
        case 'z':
          {
            SysInt64 offset;
            offset = sys_date_time_get_utc_offset (datetime) / USEC_PER_SECOND;
            if (!format_z (outstr, (int) offset, colons))
              return false;
          }
          break;
        case 'Z':
          tz = sys_date_time_get_timezone_abbreviation (datetime);
          sys_string_append (outstr, tz);
          break;
        case '%':
          sys_string_append_c (outstr, '%');
          break;
        case '-':
          pad_set = true;
          pad = "";
          goto next_mod;
        case '_':
          pad_set = true;
          pad = " ";
          goto next_mod;
        case '0':
          pad_set = true;
          pad = "0";
          goto next_mod;
        case ':':
          /* Colons are only allowed before 'z' */
          if (*utf8_format && *utf8_format != 'z' && *utf8_format != ':')
            return false;
          colons++;
          goto next_mod;
        default:
          return false;
      }
    }

  return true;
}

SysChar * sys_date_time_format (SysDateTime   *datetime,
    const SysChar *format) {
  SysString  *outstr;
  const SysChar *charset;
  /* Avoid conversions from locale (for LC_TIME and not for LC_MESSAGES unless
   * specified otherwise) charset to UTF-8 if charset is compatible
   * with UTF-8 already. Check for UTF-8 and synonymous canonical names of
   * ASCII. */
  SysBool time_is_utf8_compatible = _sys_get_time_charset (&charset) ||
    sys_str_equal ("ASCII", charset) == 0 ||
    sys_str_equal ("ANSI_X3.4-1968", charset) == 0;

  sys_return_val_if_fail (datetime != NULL, NULL);
  sys_return_val_if_fail (format != NULL, NULL);
  sys_return_val_if_fail (sys_utf8_validate (format, -1, NULL), NULL);

  outstr = sys_string_sized_new (strlen (format) * 2);

  if (!sys_date_time_format_utf8 (datetime, format, outstr,
        time_is_utf8_compatible))
  {
    sys_string_free (outstr, true);
    return NULL;
  }

  return sys_string_free (outstr, false);
}

/**
 * sys_date_time_format_iso8601:
 * @datetime: A #SysDateTime
 *
 * Format @datetime in [ISO 8601 format](https://en.wikipedia.org/wiki/ISO_8601),
 * including the date, time and time zone, and return that as a UTF-8 encoded
 * string.
 *
 * Since GLib 2.66, this will output to sub-second precision if needed.
 *
 * Returns: (transfer full) (nullable): a newly allocated string formatted in
 *   ISO 8601 format or %NULL in the case that there was an error. The string
 *   should be freed with sys_free().
 *
 * Since: 2.62
 */
SysChar * sys_date_time_format_iso8601 (SysDateTime *datetime) {
  SysString *outstr = NULL;
  SysChar *main_date = NULL;
  SysInt64 offset;
  SysChar *format = "%C%y-%m-%dT%H:%M:%S";

  sys_return_val_if_fail (datetime != NULL, NULL);

  /* if datetime has sub-second non-zero values below the second precision we
   * should print them as well */
  if (datetime->usec % SYS_TIME_SPAN_SECOND != 0)
    format = "%C%y-%m-%dT%H:%M:%S.%f";

  /* Main date and time. */
  main_date = sys_date_time_format (datetime, format);
  outstr = sys_string_new (main_date);
  sys_free (main_date);

  /* Timezone. Format it as `%:::z` unless the offset is zero, in which case
   * we can simply use `Z`. */
  offset = sys_date_time_get_utc_offset (datetime);

  if (offset == 0)
  {
    sys_string_append_c (outstr, 'Z');
  }
  else
  {
    SysChar *time_zone = sys_date_time_format (datetime, "%:::z");
    sys_string_append (outstr, time_zone);
    sys_free (time_zone);
  }

  return sys_string_free (outstr, false);
}

/* Epilogue {{{1 */
/* vim:set foldmethod=marker: */
