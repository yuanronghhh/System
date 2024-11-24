#ifndef __SYS_MARKSWEEP_H__
#define __SYS_MARKSWEEP_H__

#include <System/Type/SysTypeCommon.h>
#include <System/DataTypes/SysHList.h>

SYS_BEGIN_DECLS

#define SYS_DECLARE(var) \
  sys_cleanup(_sys_ptr_clean) \
  SysPointer var = NULL; \
  sys_ms_register_var(&var); \

#define sys_alloca_s(struct_type) \
  (struct_type)_sys_alloca(sizeof(struct_type))

#define sys_alloca_ptr() \
  sys_alloca(sizeof(SysPointer))

#define sys_ms_alloc(size) _sys_ms_alloc(SYS_MS_HEAP, size)

struct _SysMsMap {
  SysHList list;
  void** addr;
};

struct _SysMsBlock {
  SysHList parent;

  /* <private> */

  /* stakc / heap */
  SysInt type;
  SysPointer bptr;
  SysBool marked;
};

typedef enum _SYS_MS_ENUM {
  SYS_MS_STACK,
  SYS_MS_HEAP,
} SYS_MS_ENUM;

SYS_API void sys_ms_lock(void);
SYS_API void sys_ms_unlock(void);
SYS_API void _sys_ptr_clean(SysPointer *o);
SYS_API void sys_ms_register_map(SysMsMap *map);
SYS_API void sys_ms_register_var(void **addr);
SYS_API void sys_ms_collect(void);
SYS_API SysMsMap *sys_ms_map_alloc(void**addr);
SYS_API SysPointer _sys_ms_alloc(SysInt type, SysSize size);
SYS_API SysChar* sys_ms_alloc_str(const SysChar *str);

SYS_API void sys_ms_setup(void);
SYS_API void sys_ms_teardown(void);

SYS_END_DECLS

#endif
