#include <Utils/SysString.h>
#include <Utils/SysFilePrivate.h>

#define LINE_BLOCK 512

bool sys_fstat(FILE* fp, SysFileState* state) {
  int fno;
  struct stat _fstate;

  if (fp == NULL) {
    printf("[Cannot Process NULL fp]\n");
    return false;
  }

  fno = fstat(fileno(fp), &_fstate);
  if (fno == -1) {
    return false;
  }

  state->st_size = _fstate.st_size;

  return true;
}

SysSize sys_fwrite(const void* buf, SysSize size, SysSize n, FILE* fp) {
    return fwrite(buf, size, n, fp);
}

SysSize sys_fwrite_string(FILE* fp, const SysChar* s, SysSize len) {
  return fwrite(s, sizeof(SysChar), len, fp);
}

/**
 * sys_vfformat: function to write format line string.
 * @fp: file stream
 * @maxbuf: max buffer for cache.
 * @format: format string
 * @args: va_list args.
 *
 * Returns: length of result string.
 */
SysSize sys_vfformat(FILE* fp, SysChar** maxbuf, const SysChar* format, va_list args) {
  SysSize len;

  len = sys_vasprintf(maxbuf, format, args);
  sys_fwrite_string(fp, (const SysChar* )(*maxbuf), len);

  return len;
}

SysChar* sys_freadline(SysChar** dst, SysSize* len, FILE* fp) {
  SysSize nlen, total;
  SysChar *ts, *ns, *se, *ep;

  total = LINE_BLOCK;
  ns = sys_malloc_N(total);
  nlen = 0;
  ep = ns + total;
  se = ns;

  ns[total - 2] = '\0';
  ns[total - 1] = '\0';

  while (fgets(se, (int)(ep - se), fp) != NULL) {
    SysChar c = *(ep - 2);

    if (c != '\0' && c != '\n') {
      nlen += (ep - se - 1);

      total += ep - ns;
      ts = sys_malloc_N(total);
      ts[total - 2] = '\0';
      ts[total - 1] = '\0';

      sys_memcpy(ts, total, ns, nlen + 1);
      sys_free_N(ns);

      ns = ts;
      ep = ns + total;
      se = ns + nlen;
    } else {
      nlen += strlen(se);
      se = ns + nlen;

      if (*(se - 1) == '\n') {
        break;
      }
    }

  }

  if (nlen > 0) {
    *dst = ns;
    *len = nlen;
    return ns;
  } else {
    sys_free_N(ns);
  }

  return NULL;
}

SysChar* sys_fgets(SysChar* buf, SysInt max, FILE* fp) {
  return fgets(buf, max, fp);
}

SysInt sys_putc(SysInt c, FILE* fp) {
  return putc(c, fp);
}

SysInt sys_fgetc(FILE* fp) {
  return fgetc(fp);
}

SysInt sys_ungetc(SysInt c, FILE* fp) {
  return ungetc(c, fp);
}

FILE* sys_fopen(const SysChar* filename, const SysChar* mode) {
  return sys_real_fopen(filename, mode);
}

SysSize sys_fread(
    void*  buffer,
    SysSize bufsize,
    SysSize elem_size,
    SysSize elem_count,
    FILE*  fp) {
  return sys_real_fread(buffer, bufsize, elem_size, elem_count, fp);
}

SysInt sys_open(SysChar* filename, SysInt flags, SysInt mode) {
  return sys_real_open(filename, flags, mode);
}

SysInt sys_fputs(FILE* const fp, SysChar const* s) {
  SysInt len;
  SYS_LEAK_IGNORE(
      len = fputs(s, fp);
      )
    return len;
}

SysInt sys_fprintf(FILE* const fp,SysChar const* const format, ...) {
  SysInt len;

  va_list args;
  va_start(args, format);

  len = sys_vfprintf(fp, format, args);

  va_end(args);

  return len;
}

SysInt sys_vfprintf(FILE* const fp,char const* const format,va_list args) {
  SysInt len;
  SYS_LEAK_IGNORE(
      len = vfprintf(fp, format, args);
      )

    return len;
}

void sys_fclose(FILE* fp) {
  sys_return_if_fail(fp != NULL);

  fclose(fp);
}

void sys_fcloseall(void) {
  sys_real_fcloseall();
}

const SysChar* sys_exe_path(void) {
  return sys_real_exe_path();
}
