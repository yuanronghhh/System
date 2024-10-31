#ifndef __SYS_DATE_TIME_H__
#define __SYS_DATE_TIME_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

/**
 * SYS_TIME_SPAN_DAY:
 *
 * Evaluates to a time span of one day.
 *
 * Since: 2.26
 */
#define SYS_TIME_SPAN_DAY                 (SYS_INT64_CONSTANT (86400000000))

/**
 * SYS_TIME_SPAN_HOUR:
 *
 * Evaluates to a time span of one hour.
 *
 * Since: 2.26
 */
#define SYS_TIME_SPAN_HOUR                (SYS_INT64_CONSTANT (3600000000))

/**
 * SYS_TIME_SPAN_MINUTE:
 *
 * Evaluates to a time span of one minute.
 *
 * Since: 2.26
 */
#define SYS_TIME_SPAN_MINUTE              (SYS_INT64_CONSTANT (60000000))

/**
 * SYS_TIME_SPAN_SECOND:
 *
 * Evaluates to a time span of one second.
 *
 * Since: 2.26
 */
#define SYS_TIME_SPAN_SECOND              (SYS_INT64_CONSTANT (1000000))

/**
 * SYS_TIME_SPAN_MILLISECOND:
 *
 * Evaluates to a time span of one millisecond.
 *
 * Since: 2.26
 */
#define SYS_TIME_SPAN_MILLISECOND         (SYS_INT64_CONSTANT (1000))

/**
 * SysTimeSpan:
 *
 * A value representing an interval of time, in microseconds.
 *
 * Since: 2.26
 */
typedef SysInt64 SysTimeSpan;

/**
 * SysDateTime:
 *
 * An opaque structure that represents a date and time, including a time zone.
 *
 * Since: 2.26
 */
typedef struct _SysDateTime SysDateTime;

void                    sys_date_time_unref                               (SysDateTime      *datetime);
SysDateTime *             sys_date_time_ref                                 (SysDateTime      *datetime);

SysDateTime *             sys_date_time_new_now                             (SysTimeZone      *tz);
SysDateTime *             sys_date_time_new_now_local                       (void);
SysDateTime *             sys_date_time_new_now_utc                         (void);

SysDateTime *             sys_date_time_new_from_unix_local                 (SysInt64          t);
SysDateTime *             sys_date_time_new_from_unix_utc                   (SysInt64          t);

SysDateTime *             sys_date_time_new_from_iso8601                    (const SysChar    *text,
                                                                         SysTimeZone      *default_tz);

SysDateTime *             sys_date_time_new                                 (SysTimeZone      *tz,
                                                                         SysInt            year,
                                                                         SysInt            month,
                                                                         SysInt            day,
                                                                         SysInt            hour,
                                                                         SysInt            minute,
                                                                         SysDouble         seconds);
SysDateTime *             sys_date_time_new_local                           (SysInt            year,
                                                                         SysInt            month,
                                                                         SysInt            day,
                                                                         SysInt            hour,
                                                                         SysInt            minute,
                                                                         SysDouble         seconds);
SysDateTime *             sys_date_time_new_utc                             (SysInt            year,
                                                                         SysInt            month,
                                                                         SysInt            day,
                                                                         SysInt            hour,
                                                                         SysInt            minute,
                                                                         SysDouble         seconds);

SysDateTime *             sys_date_time_add                                 (SysDateTime      *datetime,
                                                                         SysTimeSpan       timespan);

SysDateTime *             sys_date_time_add_years                           (SysDateTime      *datetime,
                                                                         SysInt            years);
SysDateTime *             sys_date_time_add_months                          (SysDateTime      *datetime,
                                                                         SysInt            months);
SysDateTime *             sys_date_time_add_weeks                           (SysDateTime      *datetime,
                                                                         SysInt            weeks);
SysDateTime *             sys_date_time_add_days                            (SysDateTime      *datetime,
                                                                         SysInt            days);

SysDateTime *             sys_date_time_add_hours                           (SysDateTime      *datetime,
                                                                         SysInt            hours);
SysDateTime *             sys_date_time_add_minutes                         (SysDateTime      *datetime,
                                                                         SysInt            minutes);
SysDateTime *             sys_date_time_add_seconds                         (SysDateTime      *datetime,
                                                                         SysDouble         seconds);

SysDateTime *             sys_date_time_add_full                            (SysDateTime      *datetime,
                                                                         SysInt            years,
                                                                         SysInt            months,
                                                                         SysInt            days,
                                                                         SysInt            hours,
                                                                         SysInt            minutes,
                                                                         SysDouble         seconds);

SysInt                    sys_date_time_compare                             (const SysPointer   dt1,
                                                                         const SysPointer   dt2);
SysTimeSpan               sys_date_time_difference                          (SysDateTime      *end,
                                                                         SysDateTime      *begin);
SysUInt                   sys_date_time_hash                                (const SysPointer   datetime);
SysBool                sys_date_time_equal                               (const SysPointer   dt1,
                                                                         const SysPointer   dt2);

void                    sys_date_time_get_ymd                             (SysDateTime      *datetime,
                                                                         SysInt           *year,
                                                                         SysInt           *month,
                                                                         SysInt           *day);

SysInt                    sys_date_time_get_year                            (SysDateTime      *datetime);
SysInt                    sys_date_time_get_month                           (SysDateTime      *datetime);
SysInt                    sys_date_time_get_day_of_month                    (SysDateTime      *datetime);

SysInt                    sys_date_time_get_week_numbering_year             (SysDateTime      *datetime);
SysInt                    sys_date_time_get_week_of_year                    (SysDateTime      *datetime);
SysInt                    sys_date_time_get_day_of_week                     (SysDateTime      *datetime);

SysInt                    sys_date_time_get_day_of_year                     (SysDateTime      *datetime);

SysInt                    sys_date_time_get_hour                            (SysDateTime      *datetime);
SysInt                    sys_date_time_get_minute                          (SysDateTime      *datetime);
SysInt                    sys_date_time_get_second                          (SysDateTime      *datetime);
SysInt                    sys_date_time_get_microsecond                     (SysDateTime      *datetime);
SysDouble                 sys_date_time_get_seconds                         (SysDateTime      *datetime);

SysInt64                  sys_date_time_to_unix                             (SysDateTime      *datetime);

SysTimeSpan               sys_date_time_get_utc_offset                      (SysDateTime      *datetime);
SysTimeZone *             sys_date_time_get_timezone                        (SysDateTime      *datetime);
const SysChar *           sys_date_time_get_timezone_abbreviation           (SysDateTime      *datetime);
SysBool                sys_date_time_is_daylight_savings                 (SysDateTime      *datetime);

SysDateTime *             sys_date_time_to_timezone                         (SysDateTime      *datetime,
                                                                         SysTimeZone      *tz);
SysDateTime *             sys_date_time_to_local                            (SysDateTime      *datetime);
SysDateTime *             sys_date_time_to_utc                              (SysDateTime      *datetime);

SysChar *                 sys_date_time_format                              (SysDateTime      *datetime,
                                                                         const SysChar    *format) SYS_GNUC_MALLOC;
SysChar *                 sys_date_time_format_iso8601                      (SysDateTime      *datetime) SYS_GNUC_MALLOC;


SYS_END_DECLS

#endif
