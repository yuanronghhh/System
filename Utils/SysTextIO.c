#include <System/Utils/SysPath.h>
#include <System/Utils/SysFile.h>
#include <System/Utils/SysString.h>
#include <System/Utils/SysTextIO.h>

#define is_id(c) (c >= 0 && c <= 9) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_') || ((c == '-'))
#define LINE_MACRO_FORMAT "#line %d %s\n"

typedef struct _TIOVar TIOVar;

struct _SysTextIO {
  SysChar *filename;
  FILE *fp;

  SysInt bend;
  SysInt bcur;
  SysUInt brows;
  SysUInt bcols;

  SysInt srcline;
  const SysChar *srcname;
};

struct _TIOVar {
  const SysChar *start;
  const SysChar *end;
  SysChar *varname;
  SysChar *content;
  SysInt content_len;
};

static void tio_var_free(TIOVar* var) {
  sys_free(var->varname);

  if (var->content != NULL) {
    sys_free(var->content);
  }

  sys_free(var);
}

SysTextIO *sys_tio_new(const SysChar *filename) {
  SysTextIO *tio = sys_new(SysTextIO, 1);

  tio->fp = sys_fopen(filename, "w+");
  if (tio->fp == NULL) {
    return NULL;
  }

  tio->filename = sys_strdup(filename);
  tio->srcname = NULL;
  tio->brows = 1;
  tio->bcols = 1;
  tio->bcur = 1;
  tio->bend = 0;

  return tio;
}

void sys_tio_free(SysTextIO *tio) {
  sys_free(tio->filename);
  sys_fclose(tio->fp);
  sys_free(tio);
}

SysChar *sys_tio_filename(SysTextIO *tio) {
  sys_return_val_if_fail(tio != NULL, NULL);

  return tio->filename;
}

void sys_tio_set_src(SysTextIO *tio, const SysChar *srcname) {
  sys_return_if_fail(tio != NULL);

  tio->srcname = srcname;
}

void sys_tio_write_source(SysTextIO *tio, SysInt src_line, const SysChar *content) {
  sys_return_if_fail(tio != NULL);
  sys_return_if_fail(content != NULL);
  sys_return_if_fail(src_line > 0);

  SysChar *line;

  tio->srcline = src_line;

  line = sys_strdup_printf(LINE_MACRO_FORMAT, src_line, tio->srcname);

  sys_tio_write(tio, line);
  sys_tio_write(tio, content);
}

static void sys_tio_skip_c(SysTextIO *tio, SysChar c) {
  if(c == '\n') {
    tio->brows += 1;
    tio->bcols = 0;
    tio->bend += 1;
    tio->srcline += 1;
  }

  tio->bcols += 1;
}

void sys_tio_write(SysTextIO *tio, const SysChar *str) {
  SysSize i;
  SysChar *s = (SysChar *)str;

  for(i = 0; *s; s++, i++) {
    if(*s == '\0') {
      break;
    }

    sys_tio_skip_c(tio, *s);
  }

  sys_fwrite(str, sizeof(SysChar), i, tio->fp);
}

SysInt sys_tio_match_vars(SysTextIO *tio, 
    const SysChar *line, SysInt lineno,
    SysChar *template,
    SysHashTable *map, SysPtrArray **vars, SysInt *total, 
    SysError **error) {

  SysChar *s, *se, *content;
  SysInt ntotal;

  TIOVar *nvar;
  SysPtrArray *nvars;
  SysInt varlen, slen;

  s = (SysChar *)line;
  ntotal = 0;
  slen = 0;

  nvars = sys_ptr_array_new_with_free_func((SysDestroyFunc)tio_var_free);
  while (*s) {
    if (*s++ == '$' && *s++ == '{') {
      se = s;
      while (is_id(*se)) {
        se++;
      }

      if (*se != '}') {
        continue;
      }

      nvar = sys_new0(TIOVar, 1);
      nvar->start = s;
      nvar->end = se;

      varlen = (SysInt)(se - s);
      nvar->varname = sys_strndup(nvar->start, varlen);
      sys_ptr_array_add(nvars, nvar);

      content = sys_hash_table_lookup(map, nvar->varname);
      if (content == NULL) {
        sys_error_set_N(error, "Variable %s in %s:%d not found.", nvar->varname, template, lineno);
        goto fail;
      }

      nvar->content_len = (SysInt)strlen(content);
      nvar->content = sys_strndup(content, nvar->content_len);

      ntotal += nvar->content_len;
      ntotal -= (varlen + 3);
    }
  }

  slen = (SysInt)(s - line);
  ntotal += slen;

  *total = ntotal;
  *vars = nvars;

  return 0;

fail:
  sys_ptr_array_unref(nvars);
  return 1;
}

SysBool sys_tio_write_template(SysTextIO *tio, SysChar *tpl, SysHashTable *map) {
  sys_return_val_if_fail(tio != NULL, false);
  sys_return_val_if_fail(map != NULL, false);

  SysChar *line, *s, *nline, *ptr;
  SysSize len;
  SysInt total, status, lineno;
  SysPtrArray *vars = NULL;
  SysError* error = NULL;
  const SysChar *start, *end;
  FILE *tfp;

  len = 0;
  total = 0;
  status = 0;
  lineno = 0;

  tfp = sys_fopen(tpl, "r");

  while ((s = sys_freadline(&line, &len, tfp)) != NULL) {
    lineno += 1;

    status = sys_tio_match_vars(tio, line, lineno, tpl, map, &vars, &total, &error);
    if (status != 0) {
      sys_error_N("%s", error->message);
      sys_error_free(error);

    } else {

      if (vars->len > 0) {
        nline = sys_malloc0(total + 1);

        ptr = nline;
        start = line;
        for (SysSize i = 0; i < vars->len; i++) {
          TIOVar* var = (TIOVar*)vars->pdata[i];
          end = var->start;

          ptr = sys_strpncpy(ptr, (SysInt)(end - start - 2), start);
          ptr = sys_strpncpy(ptr, var->content_len, var->content);

          start = var->end + 1;
          if (i == vars->len - 1) {
            ptr = sys_strpcpy(ptr, start);
          }
        }

        sys_tio_write(tio, nline);
        sys_free(nline);
      } else {
        sys_tio_write(tio, line);
      }

      sys_ptr_array_unref(vars);
    }

    sys_free(line);
  }

  return 0;
}
