#ifndef __SYS_TIME_ZONE_H__
#define __SYS_TIME_ZONE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

typedef struct _SysTimeZone SysTimeZone;

/**
 * SysTimeType:
 * @SYS_TIME_TYPE_STANDARD: the time is in local standard time
 * @SYS_TIME_TYPE_DAYLIGHT: the time is in local daylight time
 * @SYS_TIME_TYPE_UNIVERSAL: the time is in UTC
 *
 * Disambiguates a given time in two ways.
 *
 * First, specifies if the given time is in universal or local time.
 *
 * Second, if the time is in local time, specifies if it is local
 * standard time or local daylight time.  This is important for the case
 * where the same local time occurs twice (during daylight savings time
 * transitions, for example).
 */
typedef enum
{
  SYS_TIME_TYPE_STANDARD,
  SYS_TIME_TYPE_DAYLIGHT,
  SYS_TIME_TYPE_UNIVERSAL
} SysTimeType;

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
