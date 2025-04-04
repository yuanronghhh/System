#include <System/Utils/SysFile.h>
#include <System/Utils/SysString.h>

#define LINE_BLOCK 512

SysBool sys_fstat(FILE* fp, SysFileState* state) {
  SysInt fno;
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
  ns = sys_malloc(total);
  nlen = 0;
  ep = ns + total;
  se = ns;

  ns[total - 2] = '\0';
  ns[total - 1] = '\0';

  while (fgets(se, (SysInt)(ep - se), fp) != NULL) {
    SysChar c = *(ep - 2);

    if (c != '\0' && c != '\n') {
      nlen += (ep - se - 1);

      total += ep - ns;
      ts = sys_malloc(total);
      ts[total - 2] = '\0';
      ts[total - 1] = '\0';

      sys_memcpy(ts, total, ns, nlen + 1);
      sys_free(ns);

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
    sys_free(ns);
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

SysInt sys_fputs(FILE* const fp, SysChar const* s) {
  return fputs(s, fp);
}

SysInt sys_fprintf(FILE* const fp,SysChar const* const format, ...) {
  SysInt len;

  va_list args;
  va_start(args, format);

  len = sys_vfprintf(fp, format, args);

  va_end(args);

  return len;
}

SysInt sys_vfprintf(FILE* const fp, SysChar const* const format,va_list args) {
  return vfprintf(fp, format, args);
}

void sys_fclose(FILE* fp) {
  sys_return_if_fail(fp != NULL);

  fclose(fp);
}

SysBool sys_file_get_contents (const SysChar *filename,
                     SysChar       **contents,
                     SysSize       *length,
                     SysError      **error) {

  sys_return_val_if_fail (filename != NULL, false);
  sys_return_val_if_fail (length != NULL, false);

  struct stat st;
  SysInt fd;
  SysSize offset;
  SysSSize nread;
  SysChar *content = NULL;

  fd = sys_open(filename, O_RDONLY, S_IREAD);
  if(fd == -1) {
    sys_error_set_N(error, "open file failed: %s", filename);
    goto fail;
  }

  if (fstat(fd, &st) != 0) {
    sys_error_set_N(error, "fstate failed: %s", filename);
    goto fail;
  }

  content = sys_malloc(st.st_size + 1);
  offset = 0;
  do {
    nread = sys_read(fd, content + offset, st.st_size - offset);

    if (nread < 0) {
      goto fail;
    }
    
    if (nread > 0) {
      offset += nread;
    }
  } while ((nread > 0 && offset < (SysSize)st.st_size) || (nread == -1 && errno == EINTR));

#if SYS_OS_WIN32
  *length = offset;
  content[offset] = '\0';
#else
  *length = st.st_size;
  content[st.st_size] = '\0';
#endif

  *contents = content;
  return true;
fail:
  if(content != NULL) {
    sys_free(content);
  }

  sys_close(fd);
  return false;
}
