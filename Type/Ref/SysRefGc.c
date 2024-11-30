#include <System/Type/Ref/SysRefGc.h>
#include <System/Type/SysBlockPrivate.h>
#include <System/Platform/Common/SysMem.h>
#include <System/Type/SysGcCommonPrivate.h>

static SysMVTable allocator = {
  .malloc = sys_real_block_malloc,
  .free = sys_real_block_free,
  .realloc = sys_real_block_realloc,
};

void sys_real_gc_setup (void) {
}

void sys_real_gc_teardown(void) {
}
