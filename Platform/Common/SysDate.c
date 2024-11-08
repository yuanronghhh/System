#include <System/Platform/Common/SysDate.h>
#include <System/Platform/Common/SysThread.h>
#include <System/DataTypes/SysArray.h>
#include <System/Utils/SysStr.h>
#include <System/Utils/SysConvert.h>
#include <System/Utils/SysUtf8.h>

/**
 * this code from glib Thread
 * see: ftp://ftp.gtk.org/pub/gtk/
 * license under GNU Lesser General Public
 */

/**
 * sys_date_new:
 *
 * Allocates a #SysDate and initializes
 * it to a safe state. The new date will
 * be cleared (as if you'd called sys_date_clear()) but invalid (it won't
 * represent an existing day). Free the return value with sys_date_free().
 *
 * Returns: a newly-allocated #SysDate
 */
SysDate*
sys_date_new (void)
{
  SysDate *d = sys_new0 (SysDate, 1); /* happily, 0 is the invalid flag for everything. */

  return d;
}

/**
 * sys_date_new_dmy:
 * @day: day of the month
 * @month: month of the year
 * @year: year
 *
 * Create a new #SysDate representing the given day-month-year triplet.
 *
 * The triplet you pass in must represent a valid date. Use sys_date_valid_dmy()
 * if needed to validate it. The returned #SysDate is guaranteed to be non-%NULL
 * and valid.
 *
 * Returns: (transfer full) (not nullable): a newly-allocated #SysDate
 *   initialized with @day, @month, and @year
 */
SysDate*
sys_date_new_dmy (SysDateDay   day, 
                SysDateMonth m, 
                SysDateYear  y)
{
  SysDate *d;
  sys_return_val_if_fail (sys_date_valid_dmy (day, m, y), NULL);
  
  d = sys_new (SysDate, 1);
  
  d->julian = false;
  d->dmy    = true;
  
  d->month = m;
  d->day   = day;
  d->year  = y;
  
  sys_assert (sys_date_valid (d));
  
  return d;
}

/**
 * sys_date_new_julian:
 * @julian_day: days since January 1, Year 1
 *
 * Create a new #SysDate representing the given Julian date.
 *
 * The @julian_day you pass in must be valid. Use sys_date_valid_julian() if
 * needed to validate it. The returned #SysDate is guaranteed to be non-%NULL and
 * valid.
 *
 * Returns: (transfer full) (not nullable): a newly-allocated #SysDate initialized
 *   with @julian_day
 */
SysDate*
sys_date_new_julian (SysUInt32 julian_day)
{
  SysDate *d;
  sys_return_val_if_fail (sys_date_valid_julian (julian_day), NULL);
  
  d = sys_new (SysDate, 1);
  
  d->julian = true;
  d->dmy    = false;
  
  d->julian_days = julian_day;
  
  sys_assert (sys_date_valid (d));
  
  return d;
}

/**
 * sys_date_free:
 * @date: a #SysDate to free
 *
 * Frees a #SysDate returned from sys_date_new().
 */
void
sys_date_free (SysDate *date)
{
  sys_return_if_fail (date != NULL);
  
  sys_free (date);
}

/**
 * sys_date_copy:
 * @date: a #SysDate to copy
 *
 * Copies a SysDate to a newly-allocated SysDate. If the input was invalid
 * (as determined by sys_date_valid()), the invalid state will be copied
 * as is into the new object.
 *
 * Returns: (transfer full): a newly-allocated #SysDate initialized from @date
 *
 * Since: 2.56
 */
SysDate *
sys_date_copy (const SysDate *date)
{
  SysDate *res;
  sys_return_val_if_fail (date != NULL, NULL);

  if (sys_date_valid (date))
    res = sys_date_new_julian (sys_date_get_julian (date));
  else
    {
      res = sys_date_new ();
      *res = *date;
    }

  return res;
}

/**
 * sys_date_valid:
 * @date: a #SysDate to check
 *
 * Returns %true if the #SysDate represents an existing day. The date must not
 * contain garbage; it should have been initialized with sys_date_clear()
 * if it wasn't allocated by one of the sys_date_new() variants.
 *
 * Returns: Whether the date is valid
 */
SysBool     
sys_date_valid (const SysDate *d)
{
  sys_return_val_if_fail (d != NULL, false);
  
  return (d->julian || d->dmy);
}

static const SysUInt8 days_in_months[2][13] = 
{  /* error, jan feb mar apr may jun jul aug sep oct nov dec */
  {  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }, 
  {  0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 } /* leap year */
};

static const SysUInt16 days_in_year[2][14] = 
{  /* 0, jan feb mar apr may  jun  jul  aug  sep  oct  nov  dec */
  {  0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 }, 
  {  0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

/**
 * sys_date_valid_month:
 * @month: month
 *
 * Returns %true if the month value is valid. The 12 #SysDateMonth
 * enumeration values are the only valid months.
 *
 * Returns: %true if the month is valid
 */
SysBool     
sys_date_valid_month (SysDateMonth m)
{ 
  return (((SysInt) m > SYS_DATE_BAD_MONTH) && ((SysInt) m < 13));
}

/**
 * sys_date_valid_year:
 * @year: year
 *
 * Returns %true if the year is valid. Any year greater than 0 is valid,
 * though there is a 16-bit limit to what #SysDate will understand.
 *
 * Returns: %true if the year is valid
 */
SysBool     
sys_date_valid_year (SysDateYear y)
{
  return ( y > SYS_DATE_BAD_YEAR );
}

/**
 * sys_date_valid_day:
 * @day: day to check
 *
 * Returns %true if the day of the month is valid (a day is valid if it's
 * between 1 and 31 inclusive).
 *
 * Returns: %true if the day is valid
 */

SysBool     
sys_date_valid_day (SysDateDay d)
{
  return ( (d > SYS_DATE_BAD_DAY) && (d < 32) );
}

/**
 * sys_date_valid_weekday:
 * @weekday: weekday
 *
 * Returns %true if the weekday is valid. The seven #SysDateWeekday enumeration
 * values are the only valid weekdays.
 *
 * Returns: %true if the weekday is valid
 */
SysBool     
sys_date_valid_weekday (SysDateWeekday w)
{
  return (((SysInt) w > SYS_DATE_BAD_WEEKDAY) && ((SysInt) w < 8));
}

/**
 * sys_date_valid_julian:
 * @julian_date: Julian day to check
 *
 * Returns %true if the Julian day is valid. Anything greater than zero
 * is basically a valid Julian, though there is a 32-bit limit.
 *
 * Returns: %true if the Julian day is valid
 */
SysBool     
sys_date_valid_julian (SysUInt32 j)
{
  return (j > SYS_DATE_BAD_JULIAN);
}

/**
 * sys_date_valid_dmy:
 * @day: day
 * @month: month
 * @year: year
 *
 * Returns %true if the day-month-year triplet forms a valid, existing day
 * in the range of days #SysDate understands (Year 1 or later, no more than
 * a few thousand years in the future).
 *
 * Returns: %true if the date is a valid one
 */
SysBool sys_date_valid_dmy (SysDateDay  d,
    SysDateMonth m,
    SysDateYear  y)
{
  /* No need to check the upper bound of @y, because #SysDateYear is 16 bits wide,
   * just like #SysDate.year. */
  return ( (m > SYS_DATE_BAD_MONTH) &&
           (m < 13)               && 
           (d > SYS_DATE_BAD_DAY)   && 
           (y > SYS_DATE_BAD_YEAR)  &&   /* must check before using sys_date_is_leap_year */
           (d <=  (sys_date_is_leap_year (y) ?
                   days_in_months[1][m] : days_in_months[0][m])) );
}


/* "Julian days" just means an absolute number of days, where Day 1 ==
 *   Jan 1, Year 1
 */
static void
sys_date_update_julian (const SysDate *const_d)
{
  SysDate *d = (SysDate *) const_d;
  SysDateYear year;
  SysInt idx;
  
  sys_return_if_fail (d != NULL);
  sys_return_if_fail (d->dmy != 0);
  sys_return_if_fail (!d->julian);
  sys_return_if_fail (sys_date_valid_dmy (d->day, d->month, d->year));
  
  /* What we actually do is: multiply years * 365 days in the year,
   * add the number of years divided by 4, subtract the number of
   * years divided by 100 and add the number of years divided by 400,
   * which accounts for leap year stuff. Code from Steffen Beyer's
   * DateCalc. 
   */
  
  year = d->year - 1; /* we know d->year > 0 since it's valid */
  
  d->julian_days = year * 365U;
  d->julian_days += (year >>= 2); /* divide by 4 and add */
  d->julian_days -= (year /= 25); /* divides original # years by 100 */
  d->julian_days += year >> 2;    /* divides by 4, which divides original by 400 */
  
  idx = sys_date_is_leap_year (d->year) ? 1 : 0;
  
  d->julian_days += days_in_year[idx][d->month] + d->day;
  
  sys_return_if_fail (sys_date_valid_julian (d->julian_days));
  
  d->julian = true;
}

static void 
sys_date_update_dmy (const SysDate *const_d)
{
  SysDate *d = (SysDate *) const_d;
  SysDateYear y;
  SysDateMonth m;
  SysDateDay day;
  
  SysUInt32 A, B, C, D, E, M;
  
  sys_return_if_fail (d != NULL);
  sys_return_if_fail (d->julian);
  sys_return_if_fail (!d->dmy);
  sys_return_if_fail (sys_date_valid_julian (d->julian_days));
  
  /* Formula taken from the Calendar FAQ; the formula was for the
   *  Julian Period which starts on 1 January 4713 BC, so we add
   *  1,721,425 to the number of days before doing the formula.
   *
   * I'm sure this can be simplified for our 1 January 1 AD period
   * start, but I can't figure out how to unpack the formula.  
   */
  
  A = d->julian_days + 1721425 + 32045;
  B = ( 4 *(A + 36524) )/ 146097 - 1;
  C = A - (146097 * B)/4;
  D = ( 4 * (C + 365) ) / 1461 - 1;
  E = C - ((1461*D) / 4);
  M = (5 * (E - 1) + 2)/153;
  
  m = M + 3 - (12*(M/10));
  day = E - (153*M + 2)/5;
  y = 100 * B + D - 4800 + (M/10);
  
#ifdef SYS_ENABLE_DEBUG
  if (!sys_date_valid_dmy (day, m, y)) 
    sys_warning_N ("OOPS julian: %u  computed dmy: %u %u %u",
               d->julian_days, day, m, y);
#endif
  
  d->month = m;
  d->day   = day;
  d->year  = y;
  
  d->dmy = true;
}

/**
 * sys_date_get_weekday:
 * @date: a #SysDate
 *
 * Returns the day of the week for a #SysDate. The date must be valid.
 *
 * Returns: day of the week as a #SysDateWeekday.
 */
SysDateWeekday 
sys_date_get_weekday (const SysDate *d)
{
  sys_return_val_if_fail (sys_date_valid (d), SYS_DATE_BAD_WEEKDAY);
  
  if (!d->julian) 
    sys_date_update_julian (d);

  sys_return_val_if_fail (d->julian, SYS_DATE_BAD_WEEKDAY);
  
  return ((d->julian_days - 1) % 7) + 1;
}

/**
 * sys_date_get_month:
 * @date: a #SysDate to get the month from
 *
 * Returns the month of the year. The date must be valid.
 *
 * Returns: month of the year as a #SysDateMonth
 */
SysDateMonth   
sys_date_get_month (const SysDate *d)
{
  sys_return_val_if_fail (sys_date_valid (d), SYS_DATE_BAD_MONTH);
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_val_if_fail (d->dmy, SYS_DATE_BAD_MONTH);
  
  return d->month;
}

/**
 * sys_date_get_year:
 * @date: a #SysDate
 *
 * Returns the year of a #SysDate. The date must be valid.
 *
 * Returns: year in which the date falls
 */
SysDateYear    
sys_date_get_year (const SysDate *d)
{
  sys_return_val_if_fail (sys_date_valid (d), SYS_DATE_BAD_YEAR);
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_val_if_fail (d->dmy, SYS_DATE_BAD_YEAR);  
  
  return d->year;
}

/**
 * sys_date_get_day:
 * @date: a #SysDate to extract the day of the month from
 *
 * Returns the day of the month. The date must be valid.
 *
 * Returns: day of the month
 */
SysDateDay     
sys_date_get_day (const SysDate *d)
{
  sys_return_val_if_fail (sys_date_valid (d), SYS_DATE_BAD_DAY);
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_val_if_fail (d->dmy, SYS_DATE_BAD_DAY);  
  
  return d->day;
}

/**
 * sys_date_get_julian:
 * @date: a #SysDate to extract the Julian day from
 *
 * Returns the Julian day or "serial number" of the #SysDate. The
 * Julian day is simply the number of days since January 1, Year 1; i.e.,
 * January 1, Year 1 is Julian day 1; January 2, Year 1 is Julian day 2,
 * etc. The date must be valid.
 *
 * Returns: Julian day
 */
SysUInt32      
sys_date_get_julian (const SysDate *d)
{
  sys_return_val_if_fail (sys_date_valid (d), SYS_DATE_BAD_JULIAN);
  
  if (!d->julian) 
    sys_date_update_julian (d);

  sys_return_val_if_fail (d->julian, SYS_DATE_BAD_JULIAN);  
  
  return d->julian_days;
}

/**
 * sys_date_get_day_of_year:
 * @date: a #SysDate to extract day of year from
 *
 * Returns the day of the year, where Jan 1 is the first day of the
 * year. The date must be valid.
 *
 * Returns: day of the year
 */
SysUInt        
sys_date_get_day_of_year (const SysDate *d)
{
  SysInt idx;
  
  sys_return_val_if_fail (sys_date_valid (d), 0);
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_val_if_fail (d->dmy, 0);  
  
  idx = sys_date_is_leap_year (d->year) ? 1 : 0;
  
  return (days_in_year[idx][d->month] + d->day);
}

/**
 * sys_date_get_monday_week_of_year:
 * @date: a #SysDate
 *
 * Returns the week of the year, where weeks are understood to start on
 * Monday. If the date is before the first Monday of the year, return 0.
 * The date must be valid.
 *
 * Returns: week of the year
 */
SysUInt        
sys_date_get_monday_week_of_year (const SysDate *d)
{
  SysDateWeekday wd;
  SysUInt day;
  SysDate first;
  
  sys_return_val_if_fail (sys_date_valid (d), 0);
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_val_if_fail (d->dmy, 0);  
  
  sys_date_clear (&first, 1);
  
  sys_date_set_dmy (&first, 1, 1, d->year);
  
  wd = sys_date_get_weekday (&first) - 1; /* make Monday day 0 */
  day = sys_date_get_day_of_year (d) - 1;
  
  return ((day + wd)/7U + (wd == 0 ? 1 : 0));
}

/**
 * sys_date_get_sunday_week_of_year:
 * @date: a #SysDate
 *
 * Returns the week of the year during which this date falls, if
 * weeks are understood to begin on Sunday. The date must be valid.
 * Can return 0 if the day is before the first Sunday of the year.
 *
 * Returns: week number
 */
SysUInt        
sys_date_get_sunday_week_of_year (const SysDate *d)
{
  SysDateWeekday wd;
  SysUInt day;
  SysDate first;
  
  sys_return_val_if_fail (sys_date_valid (d), 0);
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_val_if_fail (d->dmy, 0);  
  
  sys_date_clear (&first, 1);
  
  sys_date_set_dmy (&first, 1, 1, d->year);
  
  wd = sys_date_get_weekday (&first);
  if (wd == 7) wd = 0; /* make Sunday day 0 */
  day = sys_date_get_day_of_year (d) - 1;
  
  return ((day + wd)/7U + (wd == 0 ? 1 : 0));
}

/**
 * sys_date_get_iso8601_week_of_year:
 * @date: a valid #SysDate
 *
 * Returns the week of the year, where weeks are interpreted according
 * to ISO 8601. 
 * 
 * Returns: ISO 8601 week number of the year.
 *
 * Since: 2.6
 **/
SysUInt
sys_date_get_iso8601_week_of_year (const SysDate *d)
{
  SysUInt j, d4, L, d1, w;

  sys_return_val_if_fail (sys_date_valid (d), 0);
  
  if (!d->julian)
    sys_date_update_julian (d);

  sys_return_val_if_fail (d->julian, 0);

  /* Formula taken from the Calendar FAQ; the formula was for the
   * Julian Period which starts on 1 January 4713 BC, so we add
   * 1,721,425 to the number of days before doing the formula. 
   */
  j  = d->julian_days + 1721425;
  d4 = (j + 31741 - (j % 7)) % 146097 % 36524 % 1461;
  L  = d4 / 1460;
  d1 = ((d4 - L) % 365) + L;
  w  = d1 / 7 + 1;

  return w;
}

/**
 * sys_date_days_between:
 * @date1: the first date
 * @date2: the second date
 *
 * Computes the number of days between two dates.
 * If @date2 is prior to @date1, the returned value is negative.
 * Both dates must be valid.
 *
 * Returns: the number of days between @date1 and @date2
 */
SysInt
sys_date_days_between (const SysDate *d1,
                     const SysDate *d2)
{
  sys_return_val_if_fail (sys_date_valid (d1), 0);
  sys_return_val_if_fail (sys_date_valid (d2), 0);

  return (SysInt)sys_date_get_julian (d2) - (SysInt)sys_date_get_julian (d1);
}

/**
 * sys_date_clear:
 * @date: pointer to one or more dates to clear
 * @n_dates: number of dates to clear
 *
 * Initializes one or more #SysDate structs to a safe but invalid
 * state. The cleared dates will not represent an existing date, but will
 * not contain garbage. Useful to init a date declared on the stack.
 * Validity can be tested with sys_date_valid().
 */
void         
sys_date_clear (SysDate *d, SysUInt ndates)
{
  sys_return_if_fail (d != NULL);
  sys_return_if_fail (ndates != 0);
  
  memset (d, 0x0, ndates*sizeof (SysDate)); 
}

SYS_LOCK_DEFINE_STATIC (sys_date_global);

/* These are for the parser, output to the user should use *
 * sys_date_strftime () - this creates more never-freed memory to annoy
 * all those memory debugger users. :-) 
 */

static SysChar *lonsys_month_names[13] = 
{ 
  NULL,
};

static SysChar *lonsys_month_names_alternative[13] =
{
  NULL,
};

static SysChar *short_month_names[13] = 
{
  NULL, 
};

static SysChar *short_month_names_alternative[13] =
{
  NULL,
};

/* This tells us if we need to update the parse info */
static SysChar *current_locale = NULL;

/* order of these in the current locale */
static SysDateDMY dmy_order[3] = 
{
   SYS_DATE_DAY, SYS_DATE_MONTH, SYS_DATE_YEAR
};

/* Where to chop two-digit years: i.e., for the 1930 default, numbers
 * 29 and below are counted as in the year 2000, numbers 30 and above
 * are counted as in the year 1900.  
 */

static const SysDateYear twodigit_start_year = 1930;

/* It is impossible to enter a year between 1 AD and 99 AD with this
 * in effect.  
 */
static SysBool usinsys_twodigit_years = false;

/* Adjustment of locale era to AD, non-zero means using locale era
 */
static SysInt locale_era_adjust = 0;

struct _SysDateParseTokens {
  SysInt num_ints;
  SysInt n[3];
  SysUInt month;
};

typedef struct _SysDateParseTokens SysDateParseTokens;

static inline SysBool
update_month_match (SysSize *longest,
                    const SysChar *haystack,
                    const SysChar *needle)
{
  SysSize length;

  if (needle == NULL)
    return false;

  length = strlen (needle);
  if (*longest >= length)
    return false;

  if (strstr (haystack, needle) == NULL)
    return false;

  *longest = length;
  return true;
}

#define NUM_LEN 10

/* HOLDS: sys_date_global_lock */
static void
sys_date_fill_parse_tokens (const SysChar *str, SysDateParseTokens *pt)
{
  SysChar num[4][NUM_LEN+1];
  SysInt i;
  const SysUChar *s;
  
  /* We count 4, but store 3; so we can give an error
   * if there are 4.
   */
  num[0][0] = num[1][0] = num[2][0] = num[3][0] = '\0';
  
  s = (const SysUChar *) str;
  pt->num_ints = 0;
  while (*s && pt->num_ints < 4) 
    {
      
      i = 0;
      while (*s && isdigit (*s) && i < NUM_LEN)
        {
          num[pt->num_ints][i] = *s;
          ++s; 
          ++i;
        }
      
      if (i > 0) 
        {
          num[pt->num_ints][i] = '\0';
          ++(pt->num_ints);
        }
      
      if (*s == '\0') break;
      
      ++s;
    }
  
  pt->n[0] = pt->num_ints > 0 ? atoi (num[0]) : 0;
  pt->n[1] = pt->num_ints > 1 ? atoi (num[1]) : 0;
  pt->n[2] = pt->num_ints > 2 ? atoi (num[2]) : 0;
  
  pt->month = SYS_DATE_BAD_MONTH;
  
  if (pt->num_ints < 3)
    {
      SysSize longest = 0;
      SysChar *casefold;
      SysChar *normalized;
      
      casefold = sys_utf8_casefold (str, -1);
      normalized = sys_utf8_normalize (casefold, -1, SYS_NORMALIZE_ALL);
      sys_free (casefold);

      for (i = 1; i < 13; ++i)
        {
          /* Here month names may be in a genitive case if the language
           * grammatical rules require it.
           * Examples of how January may look in some languages:
           * Catalan: "de gener", Croatian: "siječnja", Polish: "stycznia",
           * Upper Sorbian: "januara".
           * Note that most of the languages can't or don't use the the
           * genitive case here so they use nominative everywhere.
           * For example, English always uses "January".
           */
          if (update_month_match (&longest, normalized, lonsys_month_names[i]))
            pt->month = i;

          /* Here month names will be in a nominative case.
           * Examples of how January may look in some languages:
           * Catalan: "gener", Croatian: "Siječanj", Polish: "styczeń",
           * Upper Sorbian: "Januar".
           */
          if (update_month_match (&longest, normalized, lonsys_month_names_alternative[i]))
            pt->month = i;

          /* Differences between abbreviated nominative and abbreviated
           * genitive month names are visible in very few languages but
           * let's handle them.
           */
          if (update_month_match (&longest, normalized, short_month_names[i]))
            pt->month = i;

          if (update_month_match (&longest, normalized, short_month_names_alternative[i]))
            pt->month = i;
        }

      sys_free (normalized);
    }
}

/* HOLDS: sys_date_global_lock */
static void
sys_date_prepare_to_parse (const SysChar      *str, 
                         SysDateParseTokens *pt)
{
  const SysChar *locale = setlocale (LC_TIME, NULL);
  SysBool recompute_localeinfo = false;
  SysDate d;
  
  sys_return_if_fail (locale != NULL); /* should not happen */
  
  sys_date_clear (&d, 1);              /* clear for scratch use */
  
  if ( (current_locale == NULL) || (strcmp (locale, current_locale) != 0) ) 
    recompute_localeinfo = true;  /* Uh, there used to be a reason for the temporary */
  
  if (recompute_localeinfo)
    {
      int i = 1;
      SysDateParseTokens testpt;
      SysChar buf[128];
      
      sys_free (current_locale); /* still works if current_locale == NULL */
      
      current_locale = sys_strdup (locale);
      
      short_month_names[0] = "Error";
      lonsys_month_names[0] = "Error";

      while (i < 13) 
        {
          SysChar *casefold;
          
          sys_date_set_dmy (&d, 1, i, 1976);
          
          sys_return_if_fail (sys_date_valid (&d));
          
          sys_date_strftime (buf, 127, "%b", &d);

          casefold = sys_utf8_casefold (buf, -1);
          sys_free (short_month_names[i]);
          short_month_names[i] = sys_utf8_normalize (casefold, -1, SYS_NORMALIZE_ALL);
          sys_free (casefold);
          
          sys_date_strftime (buf, 127, "%B", &d);
          casefold = sys_utf8_casefold (buf, -1);
          sys_free (lonsys_month_names[i]);
          lonsys_month_names[i] = sys_utf8_normalize (casefold, -1, SYS_NORMALIZE_ALL);
          sys_free (casefold);
          
          sys_date_strftime (buf, 127, "%Ob", &d);
          casefold = sys_utf8_casefold (buf, -1);
          sys_free (short_month_names_alternative[i]);
          short_month_names_alternative[i] = sys_utf8_normalize (casefold, -1, SYS_NORMALIZE_ALL);
          sys_free (casefold);

          sys_date_strftime (buf, 127, "%OB", &d);
          casefold = sys_utf8_casefold (buf, -1);
          sys_free (lonsys_month_names_alternative[i]);
          lonsys_month_names_alternative[i] = sys_utf8_normalize (casefold, -1, SYS_NORMALIZE_ALL);
          sys_free (casefold);

          ++i;
        }
      
      /* Determine DMY order */
      
      /* had to pick a random day - don't change this, some strftimes
       * are broken on some days, and this one is good so far. */
      sys_date_set_dmy (&d, 4, 7, 1976);
      
      sys_date_strftime (buf, 127, "%x", &d);
      
      sys_date_fill_parse_tokens (buf, &testpt);

      usinsys_twodigit_years = false;
      locale_era_adjust = 0;
      dmy_order[0] = SYS_DATE_DAY;
      dmy_order[1] = SYS_DATE_MONTH;
      dmy_order[2] = SYS_DATE_YEAR;
      
      i = 0;
      while (i < testpt.num_ints)
        {
          switch (testpt.n[i])
            {
            case 7:
              dmy_order[i] = SYS_DATE_MONTH;
              break;
            case 4:
              dmy_order[i] = SYS_DATE_DAY;
              break;
            case 76:
              usinsys_twodigit_years = true;
              // SYS_GNUC_FALLTHROUGH;
              break;
            case 1976:
              dmy_order[i] = SYS_DATE_YEAR;
              break;
            default:
              /* assume locale era */
              locale_era_adjust = 1976 - testpt.n[i];
              dmy_order[i] = SYS_DATE_YEAR;
              break;
            }
          ++i;
        }
      
#if defined(SYS_ENABLE_DEBUG) && 0
      DEBUSYS_MSG (("**SysDate prepared a new set of locale-specific parse rules."));
      i = 1;
      while (i < 13) 
        {
          DEBUSYS_MSG (("  %s   %s", lonsys_month_names[i], short_month_names[i]));
          ++i;
        }
      DEBUSYS_MSG (("Alternative month names:"));
      i = 1;
      while (i < 13)
        {
          DEBUSYS_MSG (("  %s   %s", lonsys_month_names_alternative[i], short_month_names_alternative[i]));
          ++i;
        }
      if (usinsys_twodigit_years)
        {
          DEBUSYS_MSG (("**Using twodigit years with cutoff year: %u", twodigit_start_year));
        }
      { 
        SysChar *strings[3];
        i = 0;
        while (i < 3)
          {
            switch (dmy_order[i])
              {
              case SYS_DATE_MONTH:
                strings[i] = "Month";
                break;
              case SYS_DATE_YEAR:
                strings[i] = "Year";
                break;
              case SYS_DATE_DAY:
                strings[i] = "Day";
                break;
              default:
                strings[i] = NULL;
                break;
              }
            ++i;
          }
        DEBUSYS_MSG (("**Order: %s, %s, %s", strings[0], strings[1], strings[2]));
        DEBUSYS_MSG (("**Sample date in this locale: '%s'", buf));
      }
#endif
    }
  
  sys_date_fill_parse_tokens (str, pt);
}

static SysUInt
convert_twodigit_year (SysUInt y)
{
  if (usinsys_twodigit_years && y < 100)
    {
      SysUInt two     =  twodigit_start_year % 100;
      SysUInt century = (twodigit_start_year / 100) * 100;

      if (y < two)
        century += 100;

      y += century;
    }
  return y;
}

/**
 * sys_date_set_parse:
 * @date: a #SysDate to fill in
 * @str: string to parse
 *
 * Parses a user-inputted string @str, and try to figure out what date it
 * represents, taking the [current locale][setlocale] into account. If the
 * string is successfully parsed, the date will be valid after the call.
 * Otherwise, it will be invalid. You should check using sys_date_valid()
 * to see whether the parsing succeeded.
 *
 * This function is not appropriate for file formats and the like; it
 * isn't very precise, and its exact behavior varies with the locale.
 * It's intended to be a heuristic routine that guesses what the user
 * means by a given string (and it does work pretty well in that
 * capacity).
 */
void         
sys_date_set_parse (SysDate       *d, 
                  const SysChar *str)
{
  SysDateParseTokens pt;
  SysUInt m = SYS_DATE_BAD_MONTH, day = SYS_DATE_BAD_DAY, y = SYS_DATE_BAD_YEAR;
  SysSize str_len;

  sys_return_if_fail (d != NULL);

  /* set invalid */
  sys_date_clear (d, 1);

  /* Anything longer than this is ridiculous and could take a while to normalize.
   * This limit is chosen arbitrarily. */
  str_len = strlen (str);
  if (str_len > 200)
    return;

  /* The input has to be valid UTF-8. */
  if (!sys_utf8_validate_len (str, str_len, NULL))
    return;

  SYS_LOCK (sys_date_global);

  sys_date_prepare_to_parse (str, &pt);

  if (pt.num_ints == 4) 
  {
    SYS_UNLOCK (sys_date_global);
    return; /* presumably a typo; bail out. */
  }

  if (pt.num_ints > 1)
  {
    int i = 0;
    int j = 0;

    sys_assert (pt.num_ints < 4); /* i.e., it is 2 or 3 */

    while (i < pt.num_ints && j < 3) 
    {
      switch (dmy_order[j])
      {
        case SYS_DATE_MONTH: {
          if (pt.num_ints == 2 && pt.month != SYS_DATE_BAD_MONTH)
          {
            m = pt.month;
            ++j;      /* skip months, but don't skip this number */
            continue;
          }
          else 
            m = pt.n[i];
        }
        break;
        case SYS_DATE_DAY: {
          if (pt.num_ints == 2 && pt.month == SYS_DATE_BAD_MONTH)
          {
            day = 1;
            ++j;      /* skip days, since we may have month/year */
            continue;
          }
          day = pt.n[i];
        }
        break;
        case SYS_DATE_YEAR: {
          y  = pt.n[i];

          if (locale_era_adjust != 0)
          {
            y += locale_era_adjust;
          }

          y = convert_twodigit_year (y);
        }
        break;
      }

      ++i;
      ++j;
    }

      if (pt.num_ints == 3 && !sys_date_valid_dmy (day, m, y))
        {
          /* Try YYYY MM DD */
          y   = pt.n[0];
          m   = pt.n[1];
          day = pt.n[2];
          
          if (usinsys_twodigit_years && y < 100) 
            y = SYS_DATE_BAD_YEAR; /* avoids ambiguity */
        }
      else if (pt.num_ints == 2)
        {
          if (m == SYS_DATE_BAD_MONTH && pt.month != SYS_DATE_BAD_MONTH)
            m = pt.month;
        }
    }
  else if (pt.num_ints == 1) 
    {
      if (pt.month != SYS_DATE_BAD_MONTH)
        {
          /* Month name and year? */
          m    = pt.month;
          day  = 1;
          y = pt.n[0];
        }
      else
        {
          /* Try yyyymmdd and yymmdd */
          
          m   = (pt.n[0]/100) % 100;
          day = pt.n[0] % 100;
          y   = pt.n[0]/10000;

          y   = convert_twodigit_year (y);
        }
    }
  
  /* See if we got anything valid out of all this. */
  /* y < 8000 is to catch 19998 style typos; the library is OK up to 65535 or so */
  if (y < 8000 && sys_date_valid_dmy (day, m, y)) 
    {
      d->month = m;
      d->day   = day;
      d->year  = y;
      d->dmy   = true;
    }
#ifdef SYS_ENABLE_DEBUG
  else 
    {
      DEBUSYS_MSG (("Rejected DMY %u %u %u", day, m, y));
    }
#endif
  SYS_UNLOCK (sys_date_global);
}

static SysBool _sys_localtime (time_t timet, struct tm *out_tm) {
  SysBool success = true;

#ifdef HAVE_LOCALTIME_R
  if (!localtime_r (&timet, out_tm))
    success = false;
#else
  {
    struct tm *ptm = localtime (&timet);

    if (ptm == NULL)
      {
        /* Happens at least in Microsoft's C library if you pass a
         * negative time_t.
         */
        success = false;
      }
    else
      memcpy (out_tm, ptm, sizeof (struct tm));
  }
#endif

  return success;
}

/**
 * sys_date_set_time_t:
 * @date: a #SysDate 
 * @timet: time_t value to set
 *
 * Sets the value of a date to the date corresponding to a time 
 * specified as a time_t. The time to date conversion is done using 
 * the user's current timezone.
 *
 * To set the value of a date to the current day, you could write:
 * |[<!-- language="C" -->
 *  time_t now = time (NULL);
 *  if (now == (time_t) -1)
 *    // handle the error
 *  sys_date_set_time_t (date, now);
 * ]|
 *
 * Since: 2.10
 */
void         
sys_date_set_time_t (SysDate *date,
                   time_t timet)
{
  struct tm tm;
  SysBool success;

  sys_return_if_fail (date != NULL);

  success = _sys_localtime (timet, &tm);
  if (!success)
    {
      /* Still set a default date, 2000-01-01.
       *
       * We may assert out below. */
      tm.tm_mon = 0;
      tm.tm_mday = 1;
      tm.tm_year = 100;
    }

  date->julian = false;
  
  date->month = tm.tm_mon + 1;
  date->day   = tm.tm_mday;
  date->year  = tm.tm_year + 1900;
  
  sys_return_if_fail (sys_date_valid_dmy (date->day, date->month, date->year));
  
  date->dmy    = true;

}

/**
 * sys_date_set_month:
 * @date: a #SysDate
 * @month: month to set
 *
 * Sets the month of the year for a #SysDate.  If the resulting
 * day-month-year triplet is invalid, the date will be invalid.
 */
void         
sys_date_set_month (SysDate     *d, 
                  SysDateMonth m)
{
  sys_return_if_fail (d != NULL);
  sys_return_if_fail (sys_date_valid_month (m));

  if (d->julian && !d->dmy) sys_date_update_dmy(d);
  d->julian = false;
  
  d->month = m;
  
  if (sys_date_valid_dmy (d->day, d->month, d->year))
    d->dmy = true;
  else 
    d->dmy = false;
}

/**
 * sys_date_set_day:
 * @date: a #SysDate
 * @day: day to set
 *
 * Sets the day of the month for a #SysDate. If the resulting
 * day-month-year triplet is invalid, the date will be invalid.
 */
void         
sys_date_set_day (SysDate    *d, 
                SysDateDay  day)
{
  sys_return_if_fail (d != NULL);
  sys_return_if_fail (sys_date_valid_day (day));
  
  if (d->julian && !d->dmy) sys_date_update_dmy(d);
  d->julian = false;
  
  d->day = day;
  
  if (sys_date_valid_dmy (d->day, d->month, d->year))
    d->dmy = true;
  else 
    d->dmy = false;
}

/**
 * sys_date_set_year:
 * @date: a #SysDate
 * @year: year to set
 *
 * Sets the year for a #SysDate. If the resulting day-month-year
 * triplet is invalid, the date will be invalid.
 */
void         
sys_date_set_year (SysDate     *d, 
                 SysDateYear  y)
{
  sys_return_if_fail (d != NULL);
  sys_return_if_fail (sys_date_valid_year (y));
  
  if (d->julian && !d->dmy) sys_date_update_dmy(d);
  d->julian = false;
  
  d->year = y;
  
  if (sys_date_valid_dmy (d->day, d->month, d->year))
    d->dmy = true;
  else 
    d->dmy = false;
}

/**
 * sys_date_set_dmy:
 * @date: a #SysDate
 * @day: day
 * @month: month
 * @y: year
 *
 * Sets the value of a #SysDate from a day, month, and year.
 * The day-month-year triplet must be valid; if you aren't
 * sure it is, call sys_date_valid_dmy() to check before you
 * set it.
 */
void         
sys_date_set_dmy (SysDate      *d, 
                SysDateDay    day, 
                SysDateMonth  m, 
                SysDateYear   y)
{
  sys_return_if_fail (d != NULL);
  sys_return_if_fail (sys_date_valid_dmy (day, m, y));
  
  d->julian = false;
  
  d->month = m;
  d->day   = day;
  d->year  = y;
  
  d->dmy = true;
}

/**
 * sys_date_set_julian:
 * @date: a #SysDate
 * @julian_date: Julian day number (days since January 1, Year 1)
 *
 * Sets the value of a #SysDate from a Julian day number.
 */
void         
sys_date_set_julian (SysDate   *d, 
                   SysUInt32  j)
{
  sys_return_if_fail (d != NULL);
  sys_return_if_fail (sys_date_valid_julian (j));
  
  d->julian_days = j;
  d->julian = true;
  d->dmy = false;
}

/**
 * sys_date_is_first_of_month:
 * @date: a #SysDate to check
 *
 * Returns %true if the date is on the first of a month.
 * The date must be valid.
 *
 * Returns: %true if the date is the first of the month
 */
SysBool     
sys_date_is_first_of_month (const SysDate *d)
{
  sys_return_val_if_fail (sys_date_valid (d), false);
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_val_if_fail (d->dmy, false);  
  
  if (d->day == 1) return true;
  else return false;
}

/**
 * sys_date_is_last_of_month:
 * @date: a #SysDate to check
 *
 * Returns %true if the date is the last day of the month.
 * The date must be valid.
 *
 * Returns: %true if the date is the last day of the month
 */
SysBool     
sys_date_is_last_of_month (const SysDate *d)
{
  SysInt idx;
  
  sys_return_val_if_fail (sys_date_valid (d), false);
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_val_if_fail (d->dmy, false);  
  
  idx = sys_date_is_leap_year (d->year) ? 1 : 0;
  
  if (d->day == days_in_months[idx][d->month]) return true;
  else return false;
}

/**
 * sys_date_add_days:
 * @date: a #SysDate to increment
 * @n_days: number of days to move the date forward
 *
 * Increments a date some number of days.
 * To move forward by weeks, add weeks*7 days.
 * The date must be valid.
 */
void         
sys_date_add_days (SysDate *d, 
                 SysUInt  ndays)
{
  sys_return_if_fail (sys_date_valid (d));
  
  if (!d->julian)
    sys_date_update_julian (d);

  sys_return_if_fail (d->julian);
  sys_return_if_fail (ndays <= SYS_MAXUINT32 - d->julian_days);
  
  d->julian_days += ndays;
  d->dmy = false;
}

/**
 * sys_date_subtract_days:
 * @date: a #SysDate to decrement
 * @n_days: number of days to move
 *
 * Moves a date some number of days into the past.
 * To move by weeks, just move by weeks*7 days.
 * The date must be valid.
 */
void         
sys_date_subtract_days (SysDate *d, 
                      SysUInt  ndays)
{
  sys_return_if_fail (sys_date_valid (d));
  
  if (!d->julian)
    sys_date_update_julian (d);

  sys_return_if_fail (d->julian);
  sys_return_if_fail (d->julian_days > ndays);
  
  d->julian_days -= ndays;
  d->dmy = false;
}

/**
 * sys_date_add_months:
 * @date: a #SysDate to increment
 * @n_months: number of months to move forward
 *
 * Increments a date by some number of months.
 * If the day of the month is greater than 28,
 * this routine may change the day of the month
 * (because the destination month may not have
 * the current day in it). The date must be valid.
 */
void         
sys_date_add_months (SysDate *d, 
                   SysUInt  nmonths)
{
  SysUInt years, months;
  SysInt idx;
  
  sys_return_if_fail (sys_date_valid (d));
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_if_fail (d->dmy != 0);
  sys_return_if_fail (nmonths <= SYS_MAXUINT - (d->month - 1));

  nmonths += d->month - 1;
  
  years  = nmonths/12;
  months = nmonths%12;

  sys_return_if_fail (years <= (SysUInt) (SYS_MAXUINT16 - d->year));

  d->month = months + 1;
  d->year  += years;
  
  idx = sys_date_is_leap_year (d->year) ? 1 : 0;
  
  if (d->day > days_in_months[idx][d->month])
    d->day = days_in_months[idx][d->month];
  
  d->julian = false;
  
  sys_return_if_fail (sys_date_valid (d));
}

/**
 * sys_date_subtract_months:
 * @date: a #SysDate to decrement
 * @n_months: number of months to move
 *
 * Moves a date some number of months into the past.
 * If the current day of the month doesn't exist in
 * the destination month, the day of the month
 * may change. The date must be valid.
 */
void         
sys_date_subtract_months (SysDate *d, 
                        SysUInt  nmonths)
{
  SysUInt years, months;
  SysInt idx;
  
  sys_return_if_fail (sys_date_valid (d));
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_if_fail (d->dmy != 0);
  
  years  = nmonths/12;
  months = nmonths%12;
  
  sys_return_if_fail (d->year > years);
  
  d->year  -= years;
  
  if (d->month > months) d->month -= months;
  else 
    {
      months -= d->month;
      d->month = 12 - months;
      d->year -= 1;
    }
  
  idx = sys_date_is_leap_year (d->year) ? 1 : 0;
  
  if (d->day > days_in_months[idx][d->month])
    d->day = days_in_months[idx][d->month];
  
  d->julian = false;
  
  sys_return_if_fail (sys_date_valid (d));
}

/**
 * sys_date_add_years:
 * @date: a #SysDate to increment
 * @n_years: number of years to move forward
 *
 * Increments a date by some number of years.
 * If the date is February 29, and the destination
 * year is not a leap year, the date will be changed
 * to February 28. The date must be valid.
 */
void         
sys_date_add_years (SysDate *d, 
                  SysUInt  nyears)
{
  sys_return_if_fail (sys_date_valid (d));
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_if_fail (d->dmy != 0);
  sys_return_if_fail (nyears <= (SysUInt) (SYS_MAXUINT16 - d->year));

  d->year += nyears;
  
  if (d->month == 2 && d->day == 29)
    {
      if (!sys_date_is_leap_year (d->year))
        d->day = 28;
    }
  
  d->julian = false;
}

/**
 * sys_date_subtract_years:
 * @date: a #SysDate to decrement
 * @n_years: number of years to move
 *
 * Moves a date some number of years into the past.
 * If the current day doesn't exist in the destination
 * year (i.e. it's February 29 and you move to a non-leap-year)
 * then the day is changed to February 29. The date
 * must be valid.
 */
void         
sys_date_subtract_years (SysDate *d, 
                       SysUInt  nyears)
{
  sys_return_if_fail (sys_date_valid (d));
  
  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_if_fail (d->dmy != 0);
  sys_return_if_fail (d->year > nyears);
  
  d->year -= nyears;
  
  if (d->month == 2 && d->day == 29)
    {
      if (!sys_date_is_leap_year (d->year))
        d->day = 28;
    }
  
  d->julian = false;
}

/**
 * sys_date_is_leap_year:
 * @year: year to check
 *
 * Returns %true if the year is a leap year.
 *
 * For the purposes of this function, leap year is every year
 * divisible by 4 unless that year is divisible by 100. If it
 * is divisible by 100 it would be a leap year only if that year
 * is also divisible by 400.
 *
 * Returns: %true if the year is a leap year
 */
SysBool     
sys_date_is_leap_year (SysDateYear year)
{
  sys_return_val_if_fail (sys_date_valid_year (year), false);
  
  return ( (((year % 4) == 0) && ((year % 100) != 0)) ||
           (year % 400) == 0 );
}

/**
 * sys_date_get_days_in_month:
 * @month: month
 * @year: year
 *
 * Returns the number of days in a month, taking leap
 * years into account.
 *
 * Returns: number of days in @month during the @year
 */
SysUInt8         
sys_date_get_days_in_month (SysDateMonth month, 
                          SysDateYear  year)
{
  SysInt idx;
  
  sys_return_val_if_fail (sys_date_valid_year (year), 0);
  sys_return_val_if_fail (sys_date_valid_month (month), 0);
  
  idx = sys_date_is_leap_year (year) ? 1 : 0;
  
  return days_in_months[idx][month];
}

/**
 * sys_date_get_monday_weeks_in_year:
 * @year: a year
 *
 * Returns the number of weeks in the year, where weeks
 * are taken to start on Monday. Will be 52 or 53. The
 * date must be valid. (Years always have 52 7-day periods,
 * plus 1 or 2 extra days depending on whether it's a leap
 * year. This function is basically telling you how many
 * Mondays are in the year, i.e. there are 53 Mondays if
 * one of the extra days happens to be a Monday.)
 *
 * Returns: number of Mondays in the year
 */
SysUInt8       
sys_date_get_monday_weeks_in_year (SysDateYear year)
{
  SysDate d;
  
  sys_return_val_if_fail (sys_date_valid_year (year), 0);
  
  sys_date_clear (&d, 1);
  sys_date_set_dmy (&d, 1, 1, year);
  if (sys_date_get_weekday (&d) == SYS_DATE_MONDAY) return 53;
  sys_date_set_dmy (&d, 31, 12, year);
  if (sys_date_get_weekday (&d) == SYS_DATE_MONDAY) return 53;
  if (sys_date_is_leap_year (year)) 
    {
      sys_date_set_dmy (&d, 2, 1, year);
      if (sys_date_get_weekday (&d) == SYS_DATE_MONDAY) return 53;
      sys_date_set_dmy (&d, 30, 12, year);
      if (sys_date_get_weekday (&d) == SYS_DATE_MONDAY) return 53;
    }
  return 52;
}

/**
 * sys_date_get_sunday_weeks_in_year:
 * @year: year to count weeks in
 *
 * Returns the number of weeks in the year, where weeks
 * are taken to start on Sunday. Will be 52 or 53. The
 * date must be valid. (Years always have 52 7-day periods,
 * plus 1 or 2 extra days depending on whether it's a leap
 * year. This function is basically telling you how many
 * Sundays are in the year, i.e. there are 53 Sundays if
 * one of the extra days happens to be a Sunday.)
 *
 * Returns: the number of weeks in @year
 */
SysUInt8       
sys_date_get_sunday_weeks_in_year (SysDateYear year)
{
  SysDate d;
  
  sys_return_val_if_fail (sys_date_valid_year (year), 0);
  
  sys_date_clear (&d, 1);
  sys_date_set_dmy (&d, 1, 1, year);
  if (sys_date_get_weekday (&d) == SYS_DATE_SUNDAY) return 53;
  sys_date_set_dmy (&d, 31, 12, year);
  if (sys_date_get_weekday (&d) == SYS_DATE_SUNDAY) return 53;
  if (sys_date_is_leap_year (year)) 
    {
      sys_date_set_dmy (&d, 2, 1, year);
      if (sys_date_get_weekday (&d) == SYS_DATE_SUNDAY) return 53;
      sys_date_set_dmy (&d, 30, 12, year);
      if (sys_date_get_weekday (&d) == SYS_DATE_SUNDAY) return 53;
    }
  return 52;
}

/**
 * sys_date_compare:
 * @lhs: first date to compare
 * @rhs: second date to compare
 *
 * qsort()-style comparison function for dates.
 * Both dates must be valid.
 *
 * Returns: 0 for equal, less than zero if @lhs is less than @rhs,
 *     greater than zero if @lhs is greater than @rhs
 */
SysInt sys_date_compare (const SysDate *lhs, const SysDate *rhs) {
  sys_return_val_if_fail (lhs != NULL, 0);
  sys_return_val_if_fail (rhs != NULL, 0);
  sys_return_val_if_fail (sys_date_valid (lhs), 0);
  sys_return_val_if_fail (sys_date_valid (rhs), 0);
  
  /* Remember the self-comparison case! I think it works right now. */
  
  while (true)
    {
      if (lhs->julian && rhs->julian) 
        {
          if (lhs->julian_days < rhs->julian_days) return -1;
          else if (lhs->julian_days > rhs->julian_days) return 1;
          else                                          return 0;
        }
      else if (lhs->dmy && rhs->dmy) 
        {
          if (lhs->year < rhs->year)               return -1;
          else if (lhs->year > rhs->year)               return 1;
          else 
            {
              if (lhs->month < rhs->month)         return -1;
              else if (lhs->month > rhs->month)         return 1;
              else 
                {
                  if (lhs->day < rhs->day)              return -1;
                  else if (lhs->day > rhs->day)              return 1;
                  else                                       return 0;
                }

            }

        }
      else
        {
          if (!lhs->julian) sys_date_update_julian (lhs);
          if (!rhs->julian) sys_date_update_julian (rhs);
          sys_return_val_if_fail (lhs->julian, 0);
          sys_return_val_if_fail (rhs->julian, 0);

          return 0;
        }
    }
}

/**
 * sys_date_to_struct_tm:
 * @date: a #SysDate to set the struct tm from
 * @tm: (not nullable): struct tm to fill
 *
 * Fills in the date-related bits of a struct tm using the @date value.
 * Initializes the non-date parts with something safe but meaningless.
 */
void        
sys_date_to_struct_tm (const SysDate *d, 
                     struct tm   *tm)
{
  SysDateWeekday day;

  sys_return_if_fail (sys_date_valid (d));
  sys_return_if_fail (tm != NULL);

  if (!d->dmy) 
    sys_date_update_dmy (d);

  sys_return_if_fail (d->dmy != 0);

  /* zero all the irrelevant fields to be sure they're valid */

  /* On Linux and maybe other systems, there are weird non-POSIX
   * fields on the end of struct tm that choke strftime if they
   * contain garbage.  So we need to 0 the entire struct, not just the
   * fields we know to exist. 
   */

  memset (tm, 0x0, sizeof (struct tm));

  tm->tm_mday = d->day;
  tm->tm_mon  = d->month - 1; /* 0-11 goes in tm */
  tm->tm_year = ((int)d->year) - 1900; /* X/Open says tm_year can be negative */

  day = sys_date_get_weekday (d);
  if (day == 7) day = 0; /* struct tm wants days since Sunday, so Sunday is 0 */

  tm->tm_wday = (int)day;

  tm->tm_yday = sys_date_get_day_of_year (d) - 1; /* 0 to 365 */
  tm->tm_isdst = -1; /* -1 means "information not available" */
}

/**
 * sys_date_clamp:
 * @date: a #SysDate to clamp
 * @min_date: minimum accepted value for @date
 * @max_date: maximum accepted value for @date
 *
 * If @date is prior to @min_date, sets @date equal to @min_date.
 * If @date falls after @max_date, sets @date equal to @max_date.
 * Otherwise, @date is unchanged.
 * Either of @min_date and @max_date may be %NULL.
 * All non-%NULL dates must be valid.
 */
void
sys_date_clamp (SysDate       *date,
              const SysDate *min_date,
              const SysDate *max_date) {
  sys_return_if_fail (sys_date_valid (date));

  if (min_date != NULL)
    sys_return_if_fail (sys_date_valid (min_date));

  if (max_date != NULL)
    sys_return_if_fail (sys_date_valid (max_date));

  if (min_date != NULL && max_date != NULL)
    sys_return_if_fail (sys_date_compare (min_date, max_date) <= 0);

  if (min_date && sys_date_compare (date, min_date) < 0)
    *date = *min_date;

  if (max_date && sys_date_compare (max_date, date) < 0)
    *date = *max_date;
}

/**
 * sys_date_order:
 * @date1: the first date
 * @date2: the second date
 *
 * Checks if @date1 is less than or equal to @date2,
 * and swap the values if this is not the case.
 */
void
sys_date_order (SysDate *date1,
              SysDate *date2) {
  sys_return_if_fail (sys_date_valid (date1));
  sys_return_if_fail (sys_date_valid (date2));

  if (sys_date_compare (date1, date2) > 0)
  {
    SysDate tmp = *date1;
    *date1 = *date2;
    *date2 = tmp;
  }
}

#ifdef SYS_OS_WIN32
static SysBool append_month_name (SysArray     *result,
                   LCID        lcid,
                   SYSTEMTIME *systemtime,
                   SysBool    abbreviated,
                   SysBool    alternative) {
  int n;
  WORD base;
  LPCWSTR lpFormat;

  if (alternative)
  {
    base = abbreviated ? LOCALE_SABBREVMONTHNAME1 : LOCALE_SMONTHNAME1;
    n = GetLocaleInfoW (lcid, base + systemtime->wMonth - 1, NULL, 0);
    if (n == 0)
      return false;

    sys_array_set_size (result, result->len + n);
    if (GetLocaleInfoW (lcid, base + systemtime->wMonth - 1,
          ((wchar_t *) result->pdata) + result->len - n, n) != n)
      return false;

    sys_array_set_size (result, result->len - 1);
  }
  else
  {
    /* According to MSDN, this is the correct method to obtain
     * the form of the month name used when formatting a full
     * date; it must be a genitive case in some languages.
     *
     * (n == 0) indicates an error, whereas (n < 2) is something we’d never
     * expect from the given format string, and would break the subsequent code.
     */
    lpFormat = abbreviated ? L"ddMMM" : L"ddMMMM";
    n = GetDateFormatW (lcid, 0, systemtime, lpFormat, NULL, 0);
    if (n < 2)
      return false;

    sys_array_set_size (result, result->len + n);
    if (GetDateFormatW (lcid, 0, systemtime, lpFormat,
          ((wchar_t *) result->pdata) + result->len - n, n) != n)
      return false;

    /* We have obtained a day number as two digits and the month name.
     * Now let's get rid of those two digits: overwrite them with the
     * month name.
     */
    memmove (((wchar_t *) result->pdata) + result->len - n,
        ((wchar_t *) result->pdata) + result->len - n + 2,
        (n - 2) * sizeof (wchar_t));
    sys_array_set_size (result, result->len - 3);
  }

  return true;
}

static SysSize
win32_strftime_helper (const SysDate     *d,
                       const SysChar     *format,
                       const struct tm *tm,
                       SysChar           *s,
                       SysSize                slen)
{
  SYSTEMTIME systemtime;
  TIME_ZONE_INFORMATION tzinfo;
  LCID lcid;
  int n, k;
  SysArray *result;
  const SysChar *p;
  SysUniChar c, modifier;
  const wchar_t digits[] = L"0123456789";
  SysChar *convbuf;
  SysLong convlen = 0;
  SysSize retval;

  systemtime.wYear = tm->tm_year + 1900;
  systemtime.wMonth = tm->tm_mon + 1;
  systemtime.wDayOfWeek = tm->tm_wday;
  systemtime.wDay = tm->tm_mday;
  systemtime.wHour = tm->tm_hour;
  systemtime.wMinute = tm->tm_min;
  systemtime.wSecond = tm->tm_sec;
  systemtime.wMilliseconds = 0;
  
  lcid = GetThreadLocale ();
  result = sys_array_sized_new (false, false, sizeof (wchar_t), (SysUInt)max (128, strlen (format) * 2));

  p = format;
  while (*p)
    {
      c = sys_utf8_get_char (p);
      if (c == '%')
        {
          p = sys_utf8_next_char (p);
          if (!*p)
            {
              s[0] = '\0';
              sys_array_free (result, true);

              return 0;
            }

          modifier = '\0';
          c = sys_utf8_get_char (p);
          if (c == 'E' || c == 'O')
            {
              /* "%OB", "%Ob", and "%Oh" are supported, ignore other modified
               * conversion specifiers for now.
               */
              modifier = c;
              p = sys_utf8_next_char (p);
              if (!*p)
                {
                  s[0] = '\0';
                  sys_array_free (result, true);

                  return 0;
                }

              c = sys_utf8_get_char (p);
            }

          switch (c)
            {
            case 'a':
              if (systemtime.wDayOfWeek == 0)
                k = 6;
              else
                k = systemtime.wDayOfWeek - 1;
              n = GetLocaleInfoW (lcid, LOCALE_SABBREVDAYNAME1+k, NULL, 0);
              sys_array_set_size (result, result->len + n);
              GetLocaleInfoW (lcid, LOCALE_SABBREVDAYNAME1+k, ((wchar_t *) result->pdata) + result->len - n, n);
              sys_array_set_size (result, result->len - 1);
              break;
            case 'A':
              if (systemtime.wDayOfWeek == 0)
                k = 6;
              else
                k = systemtime.wDayOfWeek - 1;
              n = GetLocaleInfoW (lcid, LOCALE_SDAYNAME1+k, NULL, 0);
              sys_array_set_size (result, result->len + n);
              GetLocaleInfoW (lcid, LOCALE_SDAYNAME1+k, ((wchar_t *) result->pdata) + result->len - n, n);
              sys_array_set_size (result, result->len - 1);
              break;
            case 'b':
            case 'h':
              if (!append_month_name (result, lcid, &systemtime, true, modifier == 'O'))
                {
                  /* Ignore the error; this placeholder will be replaced with nothing */
                }
              break;
            case 'B':
              if (!append_month_name (result, lcid, &systemtime, false, modifier == 'O'))
                {
                  /* Ignore the error; this placeholder will be replaced with nothing */
                }
              break;
            case 'c':
              n = GetDateFormatW (lcid, 0, &systemtime, NULL, NULL, 0);
              if (n > 0)
                {
                  sys_array_set_size (result, result->len + n);
                  GetDateFormatW (lcid, 0, &systemtime, NULL, ((wchar_t *) result->pdata) + result->len - n, n);
                  sys_array_set_size (result, result->len - 1);
                }
              sys_array_append_vals (result, L" ", 1);
              n = GetTimeFormatW (lcid, 0, &systemtime, NULL, NULL, 0);
              if (n > 0)
                {
                  sys_array_set_size (result, result->len + n);
                  GetTimeFormatW (lcid, 0, &systemtime, NULL, ((wchar_t *) result->pdata) + result->len - n, n);
                  sys_array_set_size (result, result->len - 1);
                }
              break;
            case 'C':
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wYear/1000), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + (systemtime.wYear/1000)%10), 1);
              break;
            case 'd':
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wDay/10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wDay%10), 1);
              break;
            case 'D':
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wMonth/10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wMonth%10), 1);
              sys_array_append_vals (result, L"/", 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wDay/10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wDay%10), 1);
              sys_array_append_vals (result, L"/", 1);
              sys_array_append_vals (result, (const SysPointer)(digits + (systemtime.wYear/10)%10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wYear%10), 1);
              break;
            case 'e':
              if (systemtime.wDay >= 10)
                sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wDay/10), 1);
              else
                sys_array_append_vals (result, L" ", 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wDay%10), 1);
              break;

              /* A SysDate has no time fields, so for now we can
               * hardcode all time conversions into zeros (or 12 for
               * %I). The alternative code snippets in the #else
               * branches are here ready to be taken into use when
               * needed by a sys_strftime() or sys_date_and_time_format()
               * or whatever.
               */
            case 'H':
#if 1
              sys_array_append_vals (result, L"00", 2);
#else
              sys_array_append_vals (result, digits + systemtime.wHour/10, 1);
              sys_array_append_vals (result, digits + systemtime.wHour%10, 1);
#endif
              break;
            case 'I':
#if 1
              sys_array_append_vals (result, L"12", 2);
#else
              if (systemtime.wHour == 0)
                sys_array_append_vals (result, L"12", 2);
              else
                {
                  sys_array_append_vals (result, digits + (systemtime.wHour%12)/10, 1);
                  sys_array_append_vals (result, digits + (systemtime.wHour%12)%10, 1);
                }
#endif
              break;
            case  'j':
              sys_array_append_vals (result, (const SysPointer)(digits + (tm->tm_yday+1)/100), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + ((tm->tm_yday+1)/10)%10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + (tm->tm_yday+1)%10), 1);
              break;
            case 'm':
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wMonth/10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wMonth%10), 1);
              break;
            case 'M':
#if 1
              sys_array_append_vals (result, L"00", 2);
#else
              sys_array_append_vals (result, digits + systemtime.wMinute/10, 1);
              sys_array_append_vals (result, digits + systemtime.wMinute%10, 1);
#endif
              break;
            case 'n':
              sys_array_append_vals (result, L"\n", 1);
              break;
            case 'p':
              n = GetTimeFormatW (lcid, 0, &systemtime, L"tt", NULL, 0);
              if (n > 0)
                {
                  sys_array_set_size (result, result->len + n);
                  GetTimeFormatW (lcid, 0, &systemtime, L"tt", ((wchar_t *) result->pdata) + result->len - n, n);
                  sys_array_set_size (result, result->len - 1);
                }
              break;
            case 'r':
              /* This is a rather odd format. Hard to say what to do.
               * Let's always use the POSIX %I:%M:%S %p
               */
#if 1
              sys_array_append_vals (result, L"12:00:00", 8);
#else
              if (systemtime.wHour == 0)
                sys_array_append_vals (result, L"12", 2);
              else
                {
                  sys_array_append_vals (result, digits + (systemtime.wHour%12)/10, 1);
                  sys_array_append_vals (result, digits + (systemtime.wHour%12)%10, 1);
                }
              sys_array_append_vals (result, L":", 1);
              sys_array_append_vals (result, digits + systemtime.wMinute/10, 1);
              sys_array_append_vals (result, digits + systemtime.wMinute%10, 1);
              sys_array_append_vals (result, L":", 1);
              sys_array_append_vals (result, digits + systemtime.wSecond/10, 1);
              sys_array_append_vals (result, digits + systemtime.wSecond%10, 1);
              sys_array_append_vals (result, L" ", 1);
#endif
              n = GetTimeFormatW (lcid, 0, &systemtime, L"tt", NULL, 0);
              if (n > 0)
                {
                  sys_array_set_size (result, result->len + n);
                  GetTimeFormatW (lcid, 0, &systemtime, L"tt", ((wchar_t *) result->pdata) + result->len - n, n);
                  sys_array_set_size (result, result->len - 1);
                }
              break;
            case 'R':
#if 1
              sys_array_append_vals (result, L"00:00", 5);
#else
              sys_array_append_vals (result, digits + systemtime.wHour/10, 1);
              sys_array_append_vals (result, digits + systemtime.wHour%10, 1);
              sys_array_append_vals (result, L":", 1);
              sys_array_append_vals (result, digits + systemtime.wMinute/10, 1);
              sys_array_append_vals (result, digits + systemtime.wMinute%10, 1);
#endif
              break;
            case 'S':
#if 1
              sys_array_append_vals (result, L"00", 2);
#else
              sys_array_append_vals (result, digits + systemtime.wSecond/10, 1);
              sys_array_append_vals (result, digits + systemtime.wSecond%10, 1);
#endif
              break;
            case 't':
              sys_array_append_vals (result, L"\t", 1);
              break;
            case 'T':
#if 1
              sys_array_append_vals (result, L"00:00:00", 8);
#else
              sys_array_append_vals (result, digits + systemtime.wHour/10, 1);
              sys_array_append_vals (result, digits + systemtime.wHour%10, 1);
              sys_array_append_vals (result, L":", 1);
              sys_array_append_vals (result, digits + systemtime.wMinute/10, 1);
              sys_array_append_vals (result, digits + systemtime.wMinute%10, 1);
              sys_array_append_vals (result, L":", 1);
              sys_array_append_vals (result, digits + systemtime.wSecond/10, 1);
              sys_array_append_vals (result, digits + systemtime.wSecond%10, 1);
#endif
              break;
            case 'u':
              if (systemtime.wDayOfWeek == 0)
                sys_array_append_vals (result, L"7", 1);
              else
                sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wDayOfWeek), 1);
              break;
            case 'U':
              n = sys_date_get_sunday_week_of_year (d);
              sys_array_append_vals (result, (const SysPointer)(digits + n/10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + n%10), 1);
              break;
            case 'V':
              n = sys_date_get_iso8601_week_of_year (d);
              sys_array_append_vals (result, (const SysPointer)(digits + n/10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + n%10), 1);
              break;
            case 'w':
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wDayOfWeek), 1);
              break;
            case 'W':
              n = sys_date_get_monday_week_of_year (d);
              sys_array_append_vals (result, (const SysPointer)(digits + n/10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + n%10), 1);
              break;
            case 'x':
              n = GetDateFormatW (lcid, 0, &systemtime, NULL, NULL, 0);
              if (n > 0)
                {
                  sys_array_set_size (result, result->len + n);
                  GetDateFormatW (lcid, 0, &systemtime, NULL, ((wchar_t *) result->pdata) + result->len - n, n);
                  sys_array_set_size (result, result->len - 1);
                }
              break;
            case 'X':
              n = GetTimeFormatW (lcid, 0, &systemtime, NULL, NULL, 0);
              if (n > 0)
                {
                  sys_array_set_size (result, result->len + n);
                  GetTimeFormatW (lcid, 0, &systemtime, NULL, ((wchar_t *) result->pdata) + result->len - n, n);
                  sys_array_set_size (result, result->len - 1);
                }
              break;
            case 'y':
              sys_array_append_vals (result, (const SysPointer)(digits + (systemtime.wYear/10)%10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wYear%10), 1);
              break;
            case 'Y':
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wYear/1000), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + (systemtime.wYear/100)%10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + (systemtime.wYear/10)%10), 1);
              sys_array_append_vals (result, (const SysPointer)(digits + systemtime.wYear%10), 1);
              break;
            case 'Z':
              n = GetTimeZoneInformation (&tzinfo);
              if (n == TIME_ZONE_ID_UNKNOWN || n == TIME_ZONE_ID_STANDARD)
                sys_array_append_vals (result, tzinfo.StandardName, (SysUInt)wcslen (tzinfo.StandardName));
              else if (n == TIME_ZONE_ID_DAYLIGHT)
                sys_array_append_vals (result, tzinfo.DaylightName, (SysUInt)wcslen (tzinfo.DaylightName));
              break;
            case '%':
              sys_array_append_vals (result, L"%", 1);
              break;
            }      
        } 
      else if (c <= 0xFFFF)
        {
          wchar_t wc = c;
          sys_array_append_vals (result, &wc, 1);
        }
      else
        {
          SysLong nwc;
          wchar_t *ws;

          ws = sys_ucs4_to_utf16 (&c, 1, NULL, &nwc, NULL);
          sys_array_append_vals (result, ws, nwc);
          sys_free (ws);
        }
      p = sys_utf8_next_char (p);
    }
  
  convbuf = sys_utf16_to_utf8 ((wchar_t *) result->pdata, result->len, NULL, &convlen, NULL);
  sys_array_free (result, true);

  if (!convbuf)
    {
      s[0] = '\0';
      return 0;
    }
  
  sys_assert (convlen >= 0);
  if ((SysSize) convlen >= slen)
    {
      /* Ensure only whole characters are copied into the buffer. */
      SysChar *end = sys_utf8_find_prev_char (convbuf, convbuf + slen);
      sys_assert (end != NULL);
      convlen = (SysLong)(end - convbuf);

      /* Return 0 because the buffer isn't large enough. */
      retval = 0;
    }
  else
    retval = convlen;

  memcpy (s, convbuf, convlen);
  s[convlen] = '\0';
  sys_free (convbuf);

  return retval;
}

#endif

/**
 * sys_date_strftime:
 * @s: destination buffer
 * @slen: buffer size
 * @format: format string
 * @date: valid #SysDate
 *
 * Generates a printed representation of the date, in a
 * [locale][setlocale]-specific way.
 * Works just like the platform's C library strftime() function,
 * but only accepts date-related formats; time-related formats
 * give undefined results. Date must be valid. Unlike strftime()
 * (which uses the locale encoding), works on a UTF-8 format
 * string and stores a UTF-8 result.
 *
 * This function does not provide any conversion specifiers in
 * addition to those implemented by the platform's C library.
 * For example, don't expect that using sys_date_strftime() would
 * make the \%F provided by the C99 strftime() work on Windows
 * where the C library only complies to C89.
 *
 * Returns: number of characters written to the buffer, or 0 the buffer was too small
 */
SysSize     
sys_date_strftime (SysChar       *s, 
                 SysSize        slen, 
                 const SysChar *format, 
                 const SysDate *d)
{
  struct tm tm;
#ifndef SYS_OS_WIN32
  SysSize locale_format_len = 0;
  SysChar *locale_format;
  SysSize tmplen;
  SysChar *tmpbuf;
  SysSize tmpbufsize;
  SysSize convlen = 0;
  SysChar *convbuf;
  SysError *error = NULL;
  SysSize retval;
#endif

  sys_return_val_if_fail (sys_date_valid (d), 0);
  sys_return_val_if_fail (slen > 0, 0); 
  sys_return_val_if_fail (format != NULL, 0);
  sys_return_val_if_fail (s != NULL, 0);

  sys_date_to_struct_tm (d, &tm);

#ifdef SYS_OS_WIN32
  if (!sys_utf8_validate (format, -1, NULL))
    {
      s[0] = '\0';
      return 0;
    }
  return win32_strftime_helper (d, format, &tm, s, slen);
#else

  locale_format = sys_locale_from_utf8 (format, -1, NULL, &locale_format_len, &error);

  if (error)
    {
      sys_warning_N ("Error converting format to locale encoding: %s", error->message);
      sys_error_free (error);

      s[0] = '\0';
      return 0;
    }

  tmpbufsize = max (128, locale_format_len * 2);
  while (true)
    {
      tmpbuf = sys_malloc (tmpbufsize);

      /* Set the first byte to something other than '\0', to be able to
       * recognize whether strftime actually failed or just returned "".
       */
      tmpbuf[0] = '\1';
      tmplen = strftime (tmpbuf, tmpbufsize, locale_format, &tm);

      if (tmplen == 0 && tmpbuf[0] != '\0')
        {
          sys_free (tmpbuf);
          tmpbufsize *= 2;

          if (tmpbufsize > 65536)
            {
              sys_warning_N ("%s","Maximum buffer size for sys_date_strftime exceeded: giving up");
              sys_free (locale_format);

              s[0] = '\0';
              return 0;
            }
        }
      else
        break;
    }
  sys_free (locale_format);

  convbuf = sys_locale_to_utf8 (tmpbuf, tmplen, NULL, &convlen, &error);
  sys_free (tmpbuf);

  if (error)
    {
      sys_warning_N ("Error converting results of strftime to UTF-8: %s", error->message);
      sys_error_free (error);

      sys_assert (convbuf == NULL);

      s[0] = '\0';
      return 0;
    }

  if (slen <= convlen)
    {
      /* Ensure only whole characters are copied into the buffer.
       */
      SysChar *end = sys_utf8_find_prev_char (convbuf, convbuf + slen);
      sys_assert (end != NULL);
      convlen = end - convbuf;

      /* Return 0 because the buffer isn't large enough.
       */
      retval = 0;
    }
  else
    retval = convlen;

  memcpy (s, convbuf, convlen);
  s[convlen] = '\0';
  sys_free (convbuf);

  return retval;
#endif
}
