#include <System/Platform/Common/SysMemPrivate.h>

void sys_real_memcpy(
    void*       const dst,
    SysSize     const dst_size,
    void const* const src,
    SysSize     const src_size) {

  UNUSED(dst_size);

  memcpy(dst, src, src_size);
}

SysSize sys_real_get_msize(void* block) {

  return malloc_usable_size(block);
}

void sys_real_leaks_init(void) {
}

void sys_real_leaks_report (void) {
}
