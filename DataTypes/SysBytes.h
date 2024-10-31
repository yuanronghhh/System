#ifndef __SYS_BYTES_H__
#define __SYS_BYTES_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

SysBytes *        sys_bytes_new                     (const SysPointer   data,
                                                 SysSize           size);

SysBytes *        sys_bytes_new_take                (SysPointer        data,
                                                 SysSize           size);

SysBytes *        sys_bytes_new_static              (const SysPointer   data,
                                                 SysSize           size);

SysBytes *        sys_bytes_new_with_free_func      (const SysPointer   data,
                                                 SysSize           size,
                                                 SysDestroyFunc  free_func,
                                                 SysPointer        user_data);

SysBytes *        sys_bytes_new_from_bytes          (SysBytes         *bytes,
                                                 SysSize           offset,
                                                 SysSize           length);

const SysPointer   sys_bytes_get_data                (SysBytes         *bytes,
                                                 SysSize          *size);

SysSize           sys_bytes_get_size                (SysBytes         *bytes);

SysBytes *        sys_bytes_ref                     (SysBytes         *bytes);

void            sys_bytes_unref                   (SysBytes         *bytes);

SysPointer        sys_bytes_unref_to_data           (SysBytes         *bytes,
                                                 SysSize          *size);

SysByteArray *    sys_bytes_unref_to_array          (SysBytes         *bytes);

SysUInt           sys_bytes_hash                    (const SysPointer   bytes);

SysBool        sys_bytes_equal                   (const SysPointer   bytes1,
                                                 const SysPointer   bytes2);

SysInt            sys_bytes_compare                 (const SysPointer   bytes1,
                                                 const SysPointer   bytes2);

const SysPointer   sys_bytes_get_region              (SysBytes         *bytes,
                                                 SysSize           element_size,
                                                 SysSize           offset,
                                                 SysSize           n_elements);

SYS_END_DECLS

#endif
