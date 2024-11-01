#ifndef __SYS_TIME_ZONE_H__
#define __SYS_TIME_ZONE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

/* POSIX Timezone abbreviations are typically 3 or 4 characters, but
   Microsoft uses 32-character names. We'll use one larger to ensure
   we have room for the terminating \0.
 */
#define SYS_TIME_ZONE_NAME_SIZE 33

/* A Transition Date structure for TZ Rules, an intermediate structure
   for parsing MSWindows and Environment-variable time zones. It
   Generalizes MSWindows's SYSTEMTIME struct.
 */
struct _TimeZoneDate {
  SysDateYear     year;
  SysDateMonth     mon;
  SysDateDay mday;
  SysInt     wday;
  SysInt     week;
  SysInt32   offset;  /* hour*3600 + min*60 + sec; can be negative.  */
};

struct _TimeZoneRule {
  SysUInt        start_year;
  SysInt32       std_offset;
  SysInt32       dlt_offset;
  TimeZoneDate dlt_start;
  TimeZoneDate dlt_end;
  SysChar std_name[SYS_TIME_ZONE_NAME_SIZE];
  SysChar dlt_name[SYS_TIME_ZONE_NAME_SIZE];
};

SysTimeZone *             sys_time_zone_new                                 (const SysChar *identifier);
SysTimeZone *             sys_time_zone_new_identifier                      (const SysChar *identifier);
SysTimeZone *             sys_time_zone_new_utc                             (void);
SysTimeZone *             sys_time_zone_new_local                           (void);
SysTimeZone *             sys_time_zone_new_offset                          (SysInt32       seconds);

SysTimeZone *             sys_time_zone_ref                                 (SysTimeZone   *tz);
void                    sys_time_zone_unref                               (SysTimeZone   *tz);

SysInt                    sys_time_zone_find_interval                       (SysTimeZone   *tz,
                                                                         SysTimeType    type,
                                                                         SysInt64       time_);

SysInt                    sys_time_zone_adjust_time                         (SysTimeZone   *tz,
                                                                         SysTimeType    type,
                                                                         SysInt64      *time_);

const SysChar *           sys_time_zone_get_abbreviation                    (SysTimeZone   *tz,
                                                                         SysInt         interval);
SysInt32                  sys_time_zone_get_offset                          (SysTimeZone   *tz,
                                                                         SysInt         interval);
SysBool                sys_time_zone_is_dst                              (SysTimeZone   *tz,
                                                                         SysInt         interval);
const SysChar *           sys_time_zone_get_identifier                      (SysTimeZone   *tz);

SYS_END_DECLS

#endif /* __SYS_TIME_ZONE_H__ */
