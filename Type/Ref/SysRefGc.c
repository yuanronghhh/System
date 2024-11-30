#include <System/Type/Ref/SysRefGc.h>
#include <System/Type/SysBlockPrivate.h>
#include <System/Platform/Common/SysMem.h>

#if 0
static SysMVTable allocator = {
  .malloc = sys_real_block_malloc,
  .free = sys_real_block_free,
  .realloc = sys_real_block_realloc,
};
#endif

SysPointer ms_malloc0(SysSize size) {
  void *b = malloc(size);
  memset(b, 0, size);

  return b;
}

void sys_gc_setup (void) {
}

void sys_gc_teardown(void) {
}
