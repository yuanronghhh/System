#ifndef __SYS_BIT_H__
#define __SYS_BIT_H__

#include <System/Fundamental/SysCommon.h>


SYS_BEGIN_DECLS

#define BIT_DEFAULT_MASK (0xffffffff)
#define bit_true_m(x, f, mask) ((x) |= (f & mask))
#define bit_false_m(x, f, mask) ((x) &= (f & mask))
#define bit_toggle_m(x, f, mask) ((x) ^ (f & (mask)))

#define bit_true(x, f) ((x) |= (f))
#define bit_false(x, f) ((x) &= ~(f))
#define bit_toggle(x, f) ((x) ^ (f))

SYS_END_DECLS

#endif
