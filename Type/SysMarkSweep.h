#ifndef __SYS_MARKSWEEP_H__
#define __SYS_MARKSWEEP_H__

#include <System/Type/SysTypeCommon.h>
#include <System/DataTypes/SysHList.h>

SYS_BEGIN_DECLS

#define SYS_MS_INIT_VALUE UINT_TO_POINTER(0xCCCCCCCC)
#define SYS_MS_MAP(o) ((SysMsMap *)o)
#define SYS_MS_BLOCK(o) ((SysMsBlock *)o)

#define sys_ms_alloca(map) \
  map = sys_alloca(sizeof(SysMsMap)); \
  sys_ms_map_init(map)

#define SYS_MS_REGISTER_VAR(TypeName, var) \
  { \
    SysMsMap *msMap; \
    sys_ms_alloca(msMap); \
    msMap->addr = (void **)&(var); \
    sys_ms_map_push_head(msMap); \
  }

#define SYS_DECLARE_BEGIN(var, TypeName, func) \
  sys_cleanup(func) \
  TypeName* var = SYS_MS_INIT_VALUE; \
  SYS_MS_REGISTER_VAR(TypeName, var);

#define SYS_MS_DEFINE_CLEAN_FUNC(TypeName)       \
  static void TypeName##_cleanup(TypeName **addr) { \
    sys_ms_unregister_var((void **)addr);           \
    *addr = NULL;                                   \
  }

#define SYS_DECLARE_PTR(TypeName, var) SYS_DECLARE_BEGIN(var, TypeName, TypeName##_cleanup)

#define sys_alloca_s(struct_type) \
  (struct_type)sys_alloca(sizeof(struct_type))

#define sys_alloca_ptr() \
  sys_alloca(sizeof(SysPointer))

#define sys_ms_malloc(size) sys_ms_block_malloc(size)
#define sys_ms_new(TypeName, nsize) sys_ms_block_malloc((SysSize)nsize * sizeof(TypeName))
#define sys_ms_realloc(o, nsize) sys_ms_block_realloc(o, nsize)
#define sys_ms_free(o) sys_ms_block_remove(o)

struct _SysMsMap {
  SysHList parent;

  /* <private> */
  void** addr;
  SysDestroyFunc destroy;
};

struct _SysMsBlock {
  SysHList parent;
  /* <private> */
  SysBool marked;
};


SYS_API void sys_ms_lock(void);
SYS_API void sys_ms_unlock(void);
SYS_API void sys_ms_unregister_var(void **addr);
SYS_API void sys_ms_unregister_map(void **addr, SysMsMap *map);
SYS_API void sys_ms_collect(void);

SYS_API void sys_ms_map_push_head(SysMsMap *o);
SYS_API void sys_ms_map_push_tail(SysMsMap *o);
SYS_API void sys_ms_map_init(SysMsMap *o);
SYS_API void sys_ms_map_free(SysMsMap *o);

SYS_API void sys_ms_block_free(SysPointer o);
SYS_API void sys_ms_block_remove(SysMsBlock *o);
SYS_API SysPointer sys_ms_block_malloc(SysSize size);
SYS_API SysPointer sys_ms_block_realloc(SysPointer o, SysSize nsize);

SYS_API void sys_ms_setup(void);
SYS_API void sys_ms_teardown(void);

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

SYS_END_DECLS

#endif
