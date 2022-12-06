#include <Platform/SysMemPrivate.h>

void sys_real_memcpy(
    void*       const dst,
    SysSize     const dst_size,
    void const* const src,
    SysSize     const src_size) {

  UNUSED(dst_size);

  memcpy(dst, src, src_size);
}

void sys_real_leaks_init(void) {
}

void sys_real_leaks_report (void) {
}
