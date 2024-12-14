#ifndef __SYS_TYPE_COMMON_H__
#define __SYS_TYPE_COMMON_H__

#include <System/Fundamental/SysCommon.h>

SYS_BEGIN_DECLS

typedef enum _SYS_NODE_ENUM {
  SYS_NODE_FUNDAMENTAL = 1 << 0,
  SYS_NODE_BASE_CLASS = 1 << 1,
  SYS_NODE_ABSTRACT_CLASS = 1 << 2,
  SYS_NODE_CLASS = SYS_NODE_BASE_CLASS | SYS_NODE_ABSTRACT_CLASS,
  SYS_NODE_INTERFACE = 1 << 3,
} SYS_NODE_ENUM;

typedef enum _SYS_MS_STATUS_ENUM {
  SYS_MS_STATUS_MALLOCED,
  SYS_MS_STATUS_MARKED,
} SYS_MS_STATUS_ENUM;

typedef enum _SYS_MS_TRACK_ENUM {
  SYS_MS_TRACK_MANUAL,
  SYS_MS_TRACK_AUTO,
} SYS_MS_TRACK_ENUM;

typedef size_t SysType;
typedef struct _SysTypeInfo SysTypeInfo;
typedef struct _SysInterfaceInfo SysInterfaceInfo;
typedef struct _SysTypeNode SysTypeNode;
typedef struct _SysTypeInstance SysTypeInstance;
typedef struct _SysTypeInterface SysTypeInterface;
typedef struct _IFaceEntry IFaceEntry;
typedef struct _SysTypeClass SysTypeClass;

typedef struct _SysParamContext SysParamContext;
typedef struct _SysParam SysParam;
typedef struct _SysParamClass SysParamClass;
typedef struct _SysObject SysObject;
typedef struct _SysObjectClass SysObjectClass;

typedef struct _SysBlock SysBlock;
typedef struct _SysRefBlock SysRefBlock;
typedef struct _SysMsMap SysMsMap;
typedef struct _SysMsStack SysMsStack;
typedef struct _SysMsBlock SysMsBlock;

typedef SysType (*SysTypeFunc) (void);
typedef void (*SysTypeInitFunc) (void *self);
typedef void (*SysTypeFinalizeFunc) (void *self);
typedef void (*SysInstanceInitFunc) (void* self);
typedef void (*SysInterfaceInitFunc) (void *iface);
typedef SysPointer (*SysCloneFunc) (SysPointer o);
typedef void (*SysRefHook) (SysPointer o, SysInt ref_count);
typedef void (*SysObjectRefHook) (SysObject *o,
    SysInt ref_count,
    SysType type, 
    const SysChar *name);

SYS_END_DECLS

#endif
