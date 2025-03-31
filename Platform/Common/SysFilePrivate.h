#ifndef __SYS_FILE_PRIVATE_H__
#define __SYS_FILE_PRIVATE_H__

#include <System/Platform/Common/SysFile.h>

SYS_BEGIN_DECLS

typedef struct _SysFileState SysFileState;
typedef struct _SysTimeSpec  SysTimeSpec;
typedef struct _SysWin32PrivateStat SysWin32PrivateStat;

typedef struct _REPARSE_DATA_BUFFER
{
  ULONG  ReparseTag;
  USHORT ReparseDataLength;
  USHORT Reserved;
  union
  {
    struct
    {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      ULONG  Flags;
      WCHAR  PathBuffer[1];
    } SymbolicLinkReparseBuffer;
    struct
    {
      USHORT SubstituteNameOffset;
      USHORT SubstituteNameLength;
      USHORT PrintNameOffset;
      USHORT PrintNameLength;
      WCHAR  PathBuffer[1];
    } MountPointReparseBuffer;
    struct
    {
      UCHAR  DataBuffer[1];
    } GenericReparseBuffer;
  };
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

struct _SysTimeSpec {
  SysInt64 tv_sec;
  SysInt32 tv_nsec;
};

struct _SysWin32PrivateStat
{
  SysUInt32 volume_serial;
  SysUInt64 file_index;
  SysUInt64 attributes;
  SysUInt64 allocated_size;
  SysUInt32 reparse_tag;

  SysUInt32 st_dev;
  SysUInt32 st_ino;
  SysUInt16 st_mode;
  SysUInt16 st_uid;
  SysUInt16 st_gid;
  SysUInt32 st_nlink;
  SysUInt64 st_size;
  SysTimeSpec st_ctim;
  SysTimeSpec st_atim;
  SysTimeSpec st_mtim;
};

SYS_END_DECLS

#endif
