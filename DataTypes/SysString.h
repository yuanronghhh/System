#ifndef __SYS_STRING_H__
#define __SYS_STRING_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

struct _SysString {
  SysChar  *str;
  SysSize len;
  SysSize allocated_len;
};

SysString*     sys_string_new               (const SysChar     *init);
SysString*     sys_string_new_take          (SysChar           *init);
SysString*     sys_string_new_len           (const SysChar     *init,
                                         SysSSize           len);
SysString*     sys_string_sized_new         (SysSSize            dfl_size);
SysChar*      sys_string_free             (SysString         *string,
                                         SysBool         free_segment);
SysChar*       sys_string_free_and_steal    (SysString         *string);

SysBytes*      sys_string_free_to_bytes     (SysString         *string);
SysBool     sys_string_equal             (const SysString   *v,
                                         const SysString   *v2);
SysUInt        sys_string_hash              (const SysString   *str);
SysString*     sys_string_assign            (SysString         *string,
                                         const SysChar     *rval);
SysString *sys_string_truncate (SysString *string,
    SysSize len);
SysString*     sys_string_set_size          (SysString         *string,
                                         SysSize            len);
SysString*     sys_string_insert_len        (SysString         *string,
                                         SysSSize           pos,
                                         const SysChar     *val,
                                         SysSSize           len);
SysString*     sys_string_append            (SysString         *string,
                                         const SysChar     *val);
SysString*     sys_string_append_len        (SysString         *string,
                                         const SysChar     *val,
                                         SysSSize           len);
SysString*     sys_string_append_c          (SysString         *string,
                                         SysChar            c);
SysString*     sys_string_append_unichar    (SysString         *string,
                                         SysUniChar         wc);
SysString*     sys_string_prepend           (SysString         *string,
                                         const SysChar     *val);
SysString*     sys_string_prepend_c         (SysString         *string,
                                         SysChar            c);
SysString*     sys_string_prepend_unichar   (SysString         *string,
                                         SysUniChar         wc);
SysString*     sys_string_prepend_len       (SysString         *string,
                                         const SysChar     *val,
                                         SysSSize           len);
SysString*     sys_string_insert            (SysString         *string,
                                         SysSSize           pos,
                                         const SysChar     *val);
SysString*     sys_string_insert_c          (SysString         *string,
                                         SysSSize           pos,
                                         SysChar            c);
SysString*     sys_string_insert_unichar    (SysString         *string,
                                         SysSSize           pos,
                                         SysUniChar         wc);
SysString*     sys_string_overwrite         (SysString         *string,
                                         SysSize            pos,
                                         const SysChar     *val);
SysString*     sys_string_overwrite_len     (SysString         *string,
                                         SysSize            pos,
                                         const SysChar     *val,
                                         SysSSize           len);
SysString*     sys_string_erase             (SysString         *string,
                                         SysSSize           pos,
                                         SysSSize           len);
SysUInt         sys_string_replace          (SysString         *string,
                                         const SysChar     *find,
                                         const SysChar     *replace,
                                         SysUInt            limit);
SysString*     sys_string_ascii_down        (SysString         *string);
SysString*     sys_string_ascii_up          (SysString         *string);
void           sys_string_vprintf           (SysString         *string,
                                         const SysChar     *format,
                                         va_list          args);
void         sys_string_printf            (SysString         *string,
                                         const SysChar     *format,
                                         ...);
void         sys_string_append_vprintf    (SysString         *string,
                                         const SysChar     *format,
                                         va_list          args);
void         sys_string_append_printf     (SysString         *string,
                                         const SysChar     *format,
                                         ...);
SysString *sys_string_down (SysString *string);
SysString * sys_string_up (SysString *string);

SYS_END_DECLS

#endif
