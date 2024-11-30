#include <System/Type/SysGcCommon.h>

SysPointer ms_malloc0(SysSize size) {
  void *b = malloc(size);
  memset(b, 0, size);

  return b;
}

