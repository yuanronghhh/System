#include <System/Platform/Common/SysMemPrivate.h>
#include <System/Utils/SysString.h>
#include <System/Utils/SysFile.h>
#include <System/Utils/SysError.h>

void sys_memcpy(
    SysPointer  const dst,
    SysSize     const dst_size,
    void        const* const src,
    SysSize     const src_size) {

  sys_real_memcpy(dst, dst_size, src, src_size);
}

void _sys_clear_pointer(void **pp, SysDestroyFunc destroy) {
  sys_return_if_fail(pp != NULL);
  sys_return_if_fail(*pp != NULL);

  /* Only one access, please; work around type aliasing */
  union { char *in; SysPointer *out; } _pp;
  SysPointer _p;

  _pp.in = (char *)(pp);
  _p = *_pp.out;
  if (_p) {
    *_pp.out = NULL;
    destroy(_p);
  }
}

SysPointer sys_realloc(void *mem, SysSize size) {
  void *nmem = NULL;

  if (size) {
    nmem = realloc(mem, size);
    if (nmem) { return nmem; }

    if (nmem == NULL) {
      sys_abort_N("%s", "realloc failed.");
    }
  }

  if (mem) {
    sys_free_N(mem);
  }

  sys_assert(nmem == NULL);

  return NULL;
}

void sys_free(void *block) {
#if !defined(NO_FREE_CHECK) || !NO_FREE_CHECK
  if (block == NULL) {
    sys_warning_N("%s", "sys_free is null.");
    return;
  }
#endif

  free(block);
}

SysPointer sys_malloc(SysSize size) {
  void *b = malloc(size);

  if(b == NULL) {
    sys_abort_N("%s", "sys_malloc run failed.");
  }

  return b;
}

SysPointer sys_malloc0(SysSize size) {
  void *b = sys_malloc(size);

  sys_assert(b != NULL);

  memset(b, 0, size);
  return b;
}

SysPointer sys_memdup(const SysPointer mem, SysUInt byte_size) {
  SysPointer new_mem;

  if (mem && byte_size != 0) {
    new_mem = sys_malloc(byte_size);
    memcpy(new_mem, mem, byte_size);
  } else
    new_mem = NULL;

  return new_mem;
}

void _sys_slice_free_chain(SysSize typesize, SysPointer ptr, SysSize offset) {
  SysUInt8 *node = ptr;
  while (node) {
    SysUInt8 *next = *(SysPointer *)(node + offset);
    sys_free_N(node);
    node = next;
  }
}

void sys_leaks_setup(void) {
  sys_real_leaks_init();
}

void sys_leaks_report(void) {
  sys_real_leaks_report();
}
