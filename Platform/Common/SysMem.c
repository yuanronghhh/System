#include <System/Platform/Common/SysMemPrivate.h>
#include <System/Utils/SysStr.h>
#include <System/Platform/Common/SysFile.h>
#include <System/Utils/SysError.h>

#define SIZE_OVERFLOWS(a,b) (SYS_UNLIKELY ((b) > 0 && (a) > SYS_MAXSIZE / (b)))

static SysChar* sys_leakfile = NULL;

static SysMVTable allocatior = {
  .malloc = malloc,
  .free = free,
  .realloc = realloc
};

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

  SysPointer _p;

  _p = *pp;
  if (_p) {
    *pp = NULL;
    destroy(_p);
  }
}

SysPointer sys_realloc(void *mem, SysSize size) {
  void *nmem = NULL;

  if (size) {
    nmem = allocatior.realloc(mem, size);
    if (nmem) { return nmem; }

    sys_error_N("%s", "realloc failed.");
  }

  if (mem) {
    sys_free(mem);
  }

  sys_assert(nmem == NULL);

  return NULL;
}

void sys_free(void *ptr) {
  if (ptr == NULL) {
    sys_warning_N("%s", "sys_free block is null.");
    return;
  }

  allocatior.free(ptr);
}

/**
 * sys_strfreev:
 * @str_array: (nullable): a %NULL-terminated array of strings to free
 *
 * Frees a %NULL-terminated array of strings, as well as each
 * string it contains.
 *
 * If @str_array is %NULL, this function simply returns.
 */
void sys_strfreev (SysChar **str_array)
{
  if (str_array)
  {
    SysSize i;

    for (i = 0; str_array[i] != NULL; i++) {

      sys_free (str_array[i]);
    }

    sys_free (str_array);
  }
}


SysPointer sys_calloc(SysSize count, SysSize size) {
  void* b = calloc(count, size);

  if (b == NULL) {
    sys_error_N("%s", "sys_calloc run failed.");
  }

  return b;
}

SysPointer sys_malloc(SysSize size) {
  void *b = allocatior.malloc(size);

  if(b == NULL) {

    sys_error_N("%s", "sys_malloc run failed.");
  }

  return b;
}

SysPointer sys_malloc0(SysSize size) {
  void *b = sys_malloc(size);

  sys_assert(b != NULL);

  memset(b, 0, size);
  return b;
}

SysPointer sys_try_malloc (SysSize n_bytes) {
  SysPointer mem;

  if (SYS_LIKELY (n_bytes)) {
    mem = sys_malloc (n_bytes);

  } else {
    mem = NULL;
  }

  return mem;
}

SysPointer sys_try_malloc_n (SysSize n_blocks, SysSize n_block_bytes) {
  if (SIZE_OVERFLOWS (n_blocks, n_block_bytes))
    return NULL;

  return sys_try_malloc (n_blocks * n_block_bytes);
}

SysSize sys_get_msize(void *block) {
  return sys_real_get_msize(block);
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

SysPointer sys_memdup2 (const SysPointer mem,
    SysSize byte_size) {
  SysPointer new_mem;

  if (mem && byte_size != 0) {

    new_mem = sys_malloc (byte_size);
    memcpy (new_mem, mem, byte_size);

  } else {

    new_mem = NULL;
  }

  return new_mem;
}

SysPointer sys_aligned_malloc(SysSize align, SysSize size) {

  return sys_real_aligned_malloc(align, size);
}

void sys_aligned_free(void *ptr) {

  sys_real_aligned_free(ptr);
}

void _sys_slice_free_chain(SysSize typesize, SysPointer ptr, SysSize offset) {
  SysUInt8 *node = ptr;
  while (node) {
    SysUInt8 *next = *(SysPointer *)(node + offset);
    sys_free(node);
    node = next;
  }
}

void sys_mem_set_vtable(SysMVTable *funcs) {
  sys_return_if_fail(funcs != NULL);

  allocatior.malloc = funcs->malloc;
  allocatior.free = funcs->free;
  allocatior.realloc = funcs->realloc;
}

static void sys_leaks_setup(void) {
  if(!sys_get_debugger()) { return; }
  sys_real_leaks_init();
}

const SysChar* sys_leaks_get_file(void) {

  return sys_leakfile;
}

void sys_leaks_set_file(const SysChar *leakfile) {

  SYS_LEAK_IGNORE_BEGIN;
  sys_leakfile = sys_strdup(leakfile);
  SYS_LEAK_IGNORE_END;
}

static void sys_leaks_report(void) {
  if(!sys_get_debugger()) { return; }
  sys_real_leaks_report();
}

void sys_mem_setup(void) {

  sys_leaks_setup();
}

void sys_mem_teardown(void) {
  sys_leaks_report();
}

