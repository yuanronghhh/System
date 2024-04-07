#include <System/Platform/Common/SysOs.h>

#include <System/Utils/SysString.h>
#include <System/Utils/SysError.h>
#include <System/Utils/SysOpenSSL.h>
#include <System/DataTypes/SysQuark.h>
#include <System/DataTypes/SysArray.h>
#include <System/DataTypes/SysTypes.h>
#include <System/Platform/Common/SysThread.h>
#include <System/Platform/Common/SysOsPrivate.h>

#define  ALIGNOF_GUINT32 SYS_ALIGNOF (uint32_t)
#define  ALIGNOF_GUINT64 SYS_ALIGNOF (uint64_t)
#define  ALIGNOF_UNSIGNED_LONG SYS_ALIGNOF (unsigned long)


static SysBool inited = false;

void sys_console_setup(void) {
  sys_real_init_console();
}

SysUInt sys_nearest_pow(SysUInt num) {
  SysUInt n = 1;

  while (n < num && n > 0) {
    n <<= 1;
  }

  return n;
}

SysDouble sys_rand_double(void) {
  SysDouble d = (SysDouble)rand() / RAND_MAX;

  return d;
}

SysDouble sys_rand_double_range(SysDouble begin, SysDouble end) {
  SysDouble d = sys_rand_double();

  return begin + d * (end - begin);
}

struct msort_param
{
  size_t s;
  size_t var;
  SysCompareDataFunc cmp;
  void *arg;
  char *t;
};

static void msort_with_tmp(const struct msort_param *p, void *b, size_t n)
{
  char *b1, *b2;
  size_t n1, n2;
  char *tmp = p->t;
  const size_t s = p->s;
  SysCompareDataFunc cmp = p->cmp;
  void *arg = p->arg;

  if (n <= 1)
    return;

  n1 = n / 2;
  n2 = n - n1;
  b1 = b;
  b2 = (char *)b + (n1 * p->s);

  msort_with_tmp(p, b1, n1);
  msort_with_tmp(p, b2, n2);

  switch (p->var)
  {
  case 0:
    while (n1 > 0 && n2 > 0)
    {
      if ((*cmp) (b1, b2, arg) <= 0)
      {
        *(SysUInt32 *)tmp = *(SysUInt32 *)b1;
        b1 += sizeof(SysUInt32);
        --n1;
      }
      else
      {
        *(SysUInt32 *)tmp = *(SysUInt32 *)b2;
        b2 += sizeof(SysUInt32);
        --n2;
      }
      tmp += sizeof(SysUInt32);
    }
    break;
  case 1:
    while (n1 > 0 && n2 > 0)
    {
      if ((*cmp) (b1, b2, arg) <= 0)
      {
        *(SysUInt64 *)tmp = *(SysUInt64 *)b1;
        b1 += sizeof(SysUInt64);
        --n1;
      }
      else
      {
        *(SysUInt64 *)tmp = *(SysUInt64 *)b2;
        b2 += sizeof(SysUInt64);
        --n2;
      }
      tmp += sizeof(SysUInt64);
    }
    break;
  case 2:
    while (n1 > 0 && n2 > 0)
    {
      unsigned long *tmpl = (unsigned long *)tmp;
      unsigned long *bl;

      tmp += s;
      if ((*cmp) (b1, b2, arg) <= 0)
      {
        bl = (unsigned long *)b1;
        b1 += s;
        --n1;
      }
      else
      {
        bl = (unsigned long *)b2;
        b2 += s;
        --n2;
      }
      while (tmpl < (unsigned long *)tmp)
        *tmpl++ = *bl++;
    }
    break;
  case 3:
    while (n1 > 0 && n2 > 0)
    {
      if ((*cmp) (*(const void **)b1, *(const void **)b2, arg) <= 0)
      {
        *(void **)tmp = *(void **)b1;
        b1 += sizeof(void *);
        --n1;
      }
      else
      {
        *(void **)tmp = *(void **)b2;
        b2 += sizeof(void *);
        --n2;
      }
      tmp += sizeof(void *);
    }
    break;
  default:
    while (n1 > 0 && n2 > 0)
    {
      if ((*cmp) (b1, b2, arg) <= 0)
      {
        memcpy(tmp, b1, s);
        tmp += s;
        b1 += s;
        --n1;
      }
      else
      {
        memcpy(tmp, b2, s);
        tmp += s;
        b2 += s;
        --n2;
      }
    }
    break;
  }

  if (n1 > 0)
    memcpy(tmp, b1, n1 * s);
  memcpy(b, p->t, (n - n2) * s);
}

static void msort_r(void *b, size_t n, size_t s, SysCompareDataFunc cmp, void *arg)
{
  size_t size = n * s;
  char *tmp = NULL;
  struct msort_param p;

  /* For large object sizes use indirect sorting.  */
  if (s > 32)
    size = 2 * n * sizeof(void *) + s;

  if (size < 1024)
    /* The temporary array is small, so put it on the stack.  */
    p.t = sys_alloca(size);
  else
  {
    /* It's large, so malloc it.  */
    tmp = sys_malloc_N(size);
    p.t = tmp;
  }

  p.s = s;
  p.var = 4;
  p.cmp = cmp;
  p.arg = arg;

  if (s > 32)
  {
    /* Indirect sorting.  */
    char *ip = (char *)b;
    void **tp = (void **)(p.t + n * sizeof(void *));
    void **t = tp;
    void *tmp_storage = (void *)(tp + n);
    char *kp;
    size_t i;

    while ((void *)t < tmp_storage)
    {
      *t++ = ip;
      ip += s;
    }
    p.s = sizeof(void *);
    p.var = 3;
    msort_with_tmp(&p, p.t + n * sizeof(void *), n);

    /* tp[0] .. tp[n - 1] is now sorted, copy around entries of
 the original array.  Knuth vol. 3 (2nd ed.) exercise 5.2-10.  */
    for (i = 0, ip = (char *)b; i < n; i++, ip += s)
      if ((kp = tp[i]) != ip)
      {
        size_t j = i;
        char *jp = ip;
        memcpy(tmp_storage, ip, s);

        do
        {
          size_t k = (kp - (char *)b) / s;
          tp[j] = jp;
          memcpy(jp, kp, s);
          j = k;
          jp = kp;
          kp = tp[k];
        } while (kp != ip);

        tp[j] = jp;
        memcpy(jp, tmp_storage, s);
      }
  }
  else
  {
    if ((s & (sizeof(SysUInt32) - 1)) == 0
      && ((char *)b - (char *)0) % ALIGNOF_GUINT32 == 0)
    {
      if (s == sizeof(SysUInt32))
        p.var = 0;
      else if (s == sizeof(SysUInt64)
        && ((char *)b - (char *)0) % ALIGNOF_GUINT64 == 0)
        p.var = 1;
      else if ((s & (sizeof(unsigned long) - 1)) == 0
        && ((char *)b - (char *)0)
        % ALIGNOF_UNSIGNED_LONG == 0)
        p.var = 2;
    }
    msort_with_tmp(&p, b, n);
  }
  free(tmp);
}

void sys_qsort_with_data(const SysPointer  pbase,
  SysInt total_elems,
  SysSize            size,
  SysCompareDataFunc compare_func,
  SysPointer         user_data) {
  msort_r((SysPointer)pbase, total_elems, size, compare_func, user_data);
}

bool sys_console_is_utf8(void) {
  return sys_real_console_is_utf8();
}

bool sys_set_env(const char *var, const char *value) {
  return sys_real_set_env(var, value);
}

const char* sys_get_env(const char *var) {
  return sys_real_get_env(var);
}

SysUInt64 sys_get_monotonic_time(void) {
  return sys_real_get_monotonic_time();
}

void sys_usleep(unsigned long mseconds) {
  sys_real_usleep(mseconds);
}

void* sys_dlopen(const char *filename) {
  return sys_real_dlopen(filename);
}

void* sys_dlmodule(const char *name) {
  return sys_real_dlmodule(name);
}

void* sys_dlsymbol(void *handle, const char *symbol) {
  return sys_real_dlsymbol(handle, symbol);
}

void sys_dlclose(void* handle) {
  sys_real_dlclose(handle);
}

/**
 * sys_backtrace_string: get call stack at runtime, only works in debug mode.
 *
 * @size: the stack size.
 *
 * Returns: new allocted string array.
 */
SysChar **sys_backtrace_string(SysInt *size) {
  return sys_real_backtrace_string(size);
}

void sys_arg_init(SysSArg *self, SysInt argc, const SysChar* argv[]) {
  sys_return_if_fail(self != NULL);
  sys_return_if_fail(argv != NULL);

  self->argc = argc;
  self->argv = (SysChar **)argv;
}

static SysBool pflag_equal(const SysChar *pflag, const SysChar *key) {
  sys_return_val_if_fail(pflag != NULL, false);
  sys_return_val_if_fail(key != NULL, false);

  if (*pflag != '-') {
    return false;
  }

  if (*(pflag + 1) == '-') {

    return sys_strcmp(key, pflag + 2) == 0;
  } else {

    return sys_strcmp(key, pflag + 1) == 0;
  }
}

int sys_arg_index(SysSArg *self, const SysChar *key, SysBool is_flag) {
  sys_return_val_if_fail(self != NULL, -1);
  sys_return_val_if_fail(key != NULL, -1);

  const SysChar* arg;
  if (self->argc < 2) {
    return -1;
  }

  for (int i = 1; i < self->argc; i++) {
    arg = self->argv[i];
    if (!pflag_equal(arg, key)) {
      continue;
    }

    if (is_flag) {
      return i;
    }

    if (i >= self->argc) {
      return -1;
    }

    if (self->argv[i + 1] == NULL) {
      return -1;
    }

    if(*self->argv[i + 1] == '-') {
      return -1;
    }

    return i + 1;
  }

  return -1;
}

void sys_setup(void) {
  if(inited) {return;}

  sys_console_setup();
  sys_quark_setup();
  sys_leaks_setup();
  sys_type_setup();
  sys_real_setup();
  sys_thread_init();
  sys_ssl_setup();

  inited = true;
}

void sys_teardown(void) {
  if(!inited) {return;}

  sys_thread_detach();
  sys_real_teardown();
  sys_type_teardown();
  sys_quark_teardown();
  sys_leaks_report();

  inited = false;
}
