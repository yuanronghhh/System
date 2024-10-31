#ifndef __SYS_QUARK_H__
#define __SYS_QUARK_H__

#include <System/DataTypes/SysHashTable.h>

#define SYS_DEFINE_QUARK(QN, q_n)                                         \
SysQuark                                                                  \
q_n##_quark (void)                                                      \
{                                                                       \
  static SysQuark q;                                                      \
                                                                        \
  if SYS_UNLIKELY (q == 0)                                                \
    q = sys_quark_from_static_string (#QN);                               \
                                                                        \
  return q;                                                             \
}

#define sys_quark_string(str) sys_intern_string(str)

/* Quarks (string<->id association)
 */
SysQuark                sys_quark_try_string         (const SysChar *string);
SysQuark                sys_quark_from_static_string (const SysChar *string);
SysQuark                sys_quark_from_string        (const SysChar *string);
const SysChar *         sys_quark_to_string          (SysQuark       quark) SYS_GNUC_CONST;

const SysChar *         sys_intern_string            (const SysChar *string);
const SysChar *         sys_intern_static_string     (const SysChar *string);

SYS_API void sys_quark_setup(void);
SYS_API void sys_quark_teardown(void);

#endif
