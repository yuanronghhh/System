#ifndef __SYS_MARKSWEEP_H__
#define __SYS_MARKSWEEP_H__

#include <System/Type/SysTypeCommon.h>
#include <System/DataTypes/SysHList.h>

SYS_BEGIN_DECLS

#define SYS_MS_INIT_VALUE UINT_TO_POINTER(0xCCCCCCCC)
#define SYS_DECLARE_BEGIN(var, TypeName, func) \
  sys_cleanup(func) \
  TypeName* var = SYS_MS_INIT_VALUE; \
  sys_ms_register_var((void **)&var); \

#define SYS_MS_DEFINE_CLEAN_FUNC(TypeName)       \
  static void TypeName##_cleanup(TypeName **o) { \
    *o = NULL;                                   \
  }

#define SYS_DECLARE_PTR(TypeName, var) SYS_DECLARE_BEGIN(var, TypeName, TypeName##_cleanup)

#define sys_alloca_s(struct_type) \
  (struct_type)_sys_alloca(sizeof(struct_type))

#define sys_alloca_ptr() \
  sys_alloca(sizeof(SysPointer))

struct _SysMsMap {
  SysHList list;
  void** addr;
};

struct _SysMsBlock {
  SysHList parent;
  /* <private> */
  SysPointer bptr;
  SysBool marked;
};

SYS_MS_DEFINE_CLEAN_FUNC(SysBool);
SYS_MS_DEFINE_CLEAN_FUNC(SysRef);
SYS_MS_DEFINE_CLEAN_FUNC(SysPointer);
SYS_MS_DEFINE_CLEAN_FUNC(SysInt);
SYS_MS_DEFINE_CLEAN_FUNC(SysShort);
SYS_MS_DEFINE_CLEAN_FUNC(SysUShort);
SYS_MS_DEFINE_CLEAN_FUNC(SysUInt);
SYS_MS_DEFINE_CLEAN_FUNC(SysFloat);
SYS_MS_DEFINE_CLEAN_FUNC(SysVoid);
SYS_MS_DEFINE_CLEAN_FUNC(SysChar);
SYS_MS_DEFINE_CLEAN_FUNC(SysUChar);
SYS_MS_DEFINE_CLEAN_FUNC(SysUInt8);
SYS_MS_DEFINE_CLEAN_FUNC(SysInt8);
SYS_MS_DEFINE_CLEAN_FUNC(SysULong);
SYS_MS_DEFINE_CLEAN_FUNC(SysLong);
SYS_MS_DEFINE_CLEAN_FUNC(SysUIntPtr);
SYS_MS_DEFINE_CLEAN_FUNC(SysDouble);
SYS_MS_DEFINE_CLEAN_FUNC(SysSize);
SYS_MS_DEFINE_CLEAN_FUNC(SysSSize);
SYS_MS_DEFINE_CLEAN_FUNC(SysWChar);

SYS_API void sys_ms_lock(void);
SYS_API void sys_ms_unlock(void);
SYS_API void sys_ms_register_map(SysMsMap *map);
SYS_API void sys_ms_register_var(void **addr);
SYS_API void sys_ms_collect(void);

SYS_API SysMsMap *sys_ms_map_alloc(void**addr);
SYS_API void sys_ms_map_free(SysMsMap *o);

SYS_API void sys_ms_block_free(SysMsBlock* o);
SYS_API SysPointer sys_ms_block_alloc(SysSize size);
SYS_API SysChar* sys_ms_block_alloc_str(const SysChar *str);

SYS_API void sys_ms_setup(void);
SYS_API void sys_ms_teardown(void);

SYS_END_DECLS

#endif
