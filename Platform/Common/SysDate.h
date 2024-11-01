#ifndef __SYS_DATE_H__
#define __SYS_DATE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

/* SysDate
 *
 * Date calculations (not time for now, to be resolved). These are a
 * mutant combination of Steffen Beyer's DateCalc routines
 * (http://www.perl.com/CPAN/authors/id/STBEY/) and Jon Trowbridge's
 * date routines (written for in-house software).  Written by Havoc
 * Pennington <hp@pobox.com>
 */

/* actual week and month values */

#define SYS_DATE_BAD_JULIAN 0U
#define SYS_DATE_BAD_DAY    0U
#define SYS_DATE_BAD_YEAR   0U

/* Note: directly manipulating structs is generally a bad idea, but
 * in this case it's an *incredibly* bad idea, because all or part
 * of this struct can be invalid at any given time. Use the functions,
 * or you will get hosed, I promise.
 */
struct _SysDate
{
  SysUInt julian_days : 32; /* julian days representation - we use a
                           *  bitfield hoping that 64 bit platforms
                           *  will pack this whole struct in one big
                           *  int
                           */

  SysUInt julian : 1;    /* julian is valid */
  SysUInt dmy    : 1;    /* dmy is valid */

  /* DMY representation */
  SysUInt day    : 6;
  SysUInt month  : 4;
  SysUInt year   : 16;
};

/* sys_date_new() returns an invalid date, you then have to _set() stuff
 * to get a usable object. You can also allocate a SysDate statically,
 * then call sys_date_clear() to initialize.
 */
SysDate*       sys_date_new                   (void);
SysDate*       sys_date_new_dmy               (SysDateDay     day,
                                           SysDateMonth   month,
                                           SysDateYear    year);
SysDate*       sys_date_new_julian            (SysUInt32      julian_day);
void         sys_date_free                  (SysDate       *date);
SysDate*       sys_date_copy                  (const SysDate *date);

/* check sys_date_valid() after doing an operation that might fail, like
 * _parse.  Almost all sys_date operations are undefined on invalid
 * dates (the exceptions are the mutators, since you need those to
 * return to validity).
 */
SysBool     sys_date_valid                 (const SysDate *date);
SysBool     sys_date_valid_day             (SysDateDay     day) SYS_GNUC_CONST;
SysBool     sys_date_valid_month           (SysDateMonth month) SYS_GNUC_CONST;
SysBool     sys_date_valid_year            (SysDateYear  year) SYS_GNUC_CONST;
SysBool     sys_date_valid_weekday         (SysDateWeekday weekday) SYS_GNUC_CONST;
SysBool     sys_date_valid_julian          (SysUInt32 julian_date) SYS_GNUC_CONST;
SysBool     sys_date_valid_dmy             (SysDateDay     day,
                                           SysDateMonth   month,
                                           SysDateYear    year) SYS_GNUC_CONST;

SysDateWeekday sys_date_get_weekday           (const SysDate *date);
SysDateMonth   sys_date_get_month             (const SysDate *date);
SysDateYear    sys_date_get_year              (const SysDate *date);
SysDateDay     sys_date_get_day               (const SysDate *date);
SysUInt32      sys_date_get_julian            (const SysDate *date);
SysUInt        sys_date_get_day_of_year       (const SysDate *date);
/* First monday/sunday is the start of week 1; if we haven't reached
 * that day, return 0. These are not ISO weeks of the year; that
 * routine needs to be added.
 * these functions return the number of weeks, starting on the
 * corresponding day
 */
SysUInt        sys_date_get_monday_week_of_year (const SysDate *date);
SysUInt        sys_date_get_sunday_week_of_year (const SysDate *date);
SysUInt        sys_date_get_iso8601_week_of_year (const SysDate *date);

/* If you create a static date struct you need to clear it to get it
 * in a safe state before use. You can clear a whole array at
 * once with the ndates argument.
 */
void         sys_date_clear                 (SysDate       *date,
                                           SysUInt        n_dates);

/* The parse routine is meant for dates typed in by a user, so it
 * permits many formats but tries to catch common typos. If your data
 * needs to be strictly validated, it is not an appropriate function.
 */
void         sys_date_set_parse             (SysDate       *date,
                                           const SysChar *str);
void         sys_date_set_time_t            (SysDate       *date,
    time_t       timet);
void         sys_date_set_month             (SysDate       *date,
                                           SysDateMonth   month);
void         sys_date_set_day               (SysDate       *date,
                                           SysDateDay     day);
void         sys_date_set_year              (SysDate       *date,
                                           SysDateYear    year);
void         sys_date_set_dmy               (SysDate       *date,
                                           SysDateDay     day,
                                           SysDateMonth   month,
                                           SysDateYear    y);
void         sys_date_set_julian            (SysDate       *date,
                                           SysUInt32      julian_date);
SysBool     sys_date_is_first_of_month     (const SysDate *date);
SysBool     sys_date_is_last_of_month      (const SysDate *date);

/* To go forward by some number of weeks just go forward weeks*7 days */
void         sys_date_add_days              (SysDate       *date,
                                           SysUInt        n_days);
void         sys_date_subtract_days         (SysDate       *date,
                                           SysUInt        n_days);

/* If you add/sub months while day > 28, the day might change */
void         sys_date_add_months            (SysDate       *date,
                                           SysUInt        n_months);
void         sys_date_subtract_months       (SysDate       *date,
                                           SysUInt        n_months);

/* If it's feb 29, changing years can move you to the 28th */
void         sys_date_add_years             (SysDate       *date,
                                           SysUInt        n_years);
void         sys_date_subtract_years        (SysDate       *date,
                                           SysUInt        n_years);
SysBool     sys_date_is_leap_year          (SysDateYear    year) SYS_GNUC_CONST;
SysUInt8       sys_date_get_days_in_month     (SysDateMonth   month,
                                           SysDateYear    year) SYS_GNUC_CONST;
SysUInt8       sys_date_get_monday_weeks_in_year  (SysDateYear    year) SYS_GNUC_CONST;
SysUInt8       sys_date_get_sunday_weeks_in_year  (SysDateYear    year) SYS_GNUC_CONST;

/* Returns the number of days between the two dates.  If date2 comes
   before date1, a negative value is return. */
SysInt         sys_date_days_between          (const SysDate *date1,
    const SysDate *date2);

/* qsort-friendly (with a cast...) */
SysInt         sys_date_compare               (const SysDate *lhs,
                                           const SysDate *rhs);
void         sys_date_to_struct_tm          (const SysDate *date,
                                           struct tm   *tm);

void         sys_date_clamp                 (SysDate *date,
    const SysDate *min_date,
    const SysDate *max_date);

/* Swap date1 and date2's values if date1 > date2. */
void         sys_date_order                 (SysDate *date1, SysDate *date2);

/* Just like strftime() except you can only use date-related formats.
 *   Using a time format is undefined.
 */
SysSize        sys_date_strftime              (SysChar       *s,
                                           SysSize        slen,
                                           const SysChar *format,
                                           const SysDate *date);

#define sys_date_weekday 			sys_date_get_weekday GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_weekday)
#define sys_date_month 			sys_date_get_month GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_month)
#define sys_date_year 			sys_date_get_year GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_year)
#define sys_date_day 			sys_date_get_day GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_day)
#define sys_date_julian 			sys_date_get_julian GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_julian)
#define sys_date_day_of_year 		sys_date_get_day_of_year GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_day_of_year)
#define sys_date_monday_week_of_year 	sys_date_get_monday_week_of_year GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_monday_week_of_year)
#define sys_date_sunday_week_of_year 	sys_date_get_sunday_week_of_year GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_sunday_week_of_year)
#define sys_date_days_in_month 		sys_date_get_days_in_month GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_days_in_month)
#define sys_date_monday_weeks_in_year 	sys_date_get_monday_weeks_in_year GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_monday_weeks_in_year)
#define sys_date_sunday_weeks_in_year	sys_date_get_sunday_weeks_in_year GLIB_DEPRECATED_MACRO_IN_2_26_FOR(sys_date_get_sunday_weeks_in_year)


SYS_END_DECLS

#endif /* __SYS_DATE_H__ */
