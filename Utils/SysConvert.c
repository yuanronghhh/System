#include <System/Utils/SysConvert.h>
#include <System/DataTypes/SysQuark.h>

/* We try to terminate strings in unknown charsets with this many zero bytes
 * to ensure that multibyte strings really are nul-terminated when we return
 * them from sys_convert() and friends.
 */
#define NUL_TERMINATOR_LENGTH 4

SYS_DEFINE_QUARK (sys_convert_error, sys_convert_error)

static SysBool try_conversion (const char *to_codeset,
                const char *from_codeset,
                iconv_t    *cd)
{
  *cd = iconv_open (to_codeset, from_codeset);

  if (*cd == (iconv_t)-1 && errno == EINVAL)
    return false;
  else
    return true;
}

static SysBool
try_to_aliases (const char **to_aliases,
                const char  *from_codeset,
                iconv_t     *cd)
{
  if (to_aliases)
    {
      const char **p = to_aliases;
      while (*p)
        {
          if (try_conversion (*p, from_codeset, cd))
            return true;

          p++;
        }
    }

  return false;
}

/**
 * sys_iconv_open: (skip)
 * @to_codeset: destination codeset
 * @from_codeset: source codeset
 * 
 * Same as the standard UNIX routine iconv_open(), but
 * may be implemented via libiconv on UNIX flavors that lack
 * a native implementation.
 * 
 * GLib provides sys_convert() and sys_locale_to_utf8() which are likely
 * more convenient than the raw iconv wrappers.
 * 
 * Returns: a "conversion descriptor", or (GIConv)-1 if
 *  opening the converter failed.
 **/
GIConv
sys_iconv_open (const SysChar  *to_codeset,
              const SysChar  *from_codeset)
{
  iconv_t cd;
  
  if (!try_conversion (to_codeset, from_codeset, &cd))
    {
      const char **to_aliases = _sys_charset_get_aliases (to_codeset);
      const char **from_aliases = _sys_charset_get_aliases (from_codeset);

      if (from_aliases)
        {
          const char **p = from_aliases;
          while (*p)
            {
              if (try_conversion (to_codeset, *p, &cd))
                goto out;

              if (try_to_aliases (to_aliases, *p, &cd))
                goto out;

              p++;
            }
        }

      if (try_to_aliases (to_aliases, from_codeset, &cd))
        goto out;
    }

 out:
  return (cd == (iconv_t)-1) ? (GIConv)-1 : (GIConv)cd;
}

/**
 * sys_iconv: (skip)
 * @converter: conversion descriptor from sys_iconv_open()
 * @inbuf: bytes to convert
 * @inbytes_left: (inout): inout parameter, bytes remaining to convert in @inbuf
 * @outbuf: converted output bytes
 * @outbytes_left: (inout): inout parameter, bytes available to fill in @outbuf
 * 
 * Same as the standard UNIX routine iconv(), but
 * may be implemented via libiconv on UNIX flavors that lack
 * a native implementation.
 *
 * GLib provides sys_convert() and sys_locale_to_utf8() which are likely
 * more convenient than the raw iconv wrappers.
 * 
 * Note that the behaviour of iconv() for characters which are valid in the
 * input character set, but which have no representation in the output character
 * set, is implementation defined. This function may return success (with a
 * positive number of non-reversible conversions as replacement characters were
 * used), or it may return -1 and set an error such as %EILSEQ, in such a
 * situation.
 *
 * Returns: count of non-reversible conversions, or -1 on error
 **/
SysSize 
sys_iconv (GIConv   converter,
         SysChar  **inbuf,
         SysSize   *inbytes_left,
         SysChar  **outbuf,
         SysSize   *outbytes_left)
{
  iconv_t cd = (iconv_t)converter;

  return iconv (cd, inbuf, inbytes_left, outbuf, outbytes_left);
}

/**
 * sys_iconv_close: (skip)
 * @converter: a conversion descriptor from sys_iconv_open()
 *
 * Same as the standard UNIX routine iconv_close(), but
 * may be implemented via libiconv on UNIX flavors that lack
 * a native implementation. Should be called to clean up
 * the conversion descriptor from sys_iconv_open() when
 * you are done converting things.
 *
 * GLib provides sys_convert() and sys_locale_to_utf8() which are likely
 * more convenient than the raw iconv wrappers.
 * 
 * Returns: -1 on error, 0 on success
 **/
SysInt
sys_iconv_close (GIConv converter)
{
  iconv_t cd = (iconv_t)converter;

  return iconv_close (cd);
}

static GIConv
open_converter (const SysChar *to_codeset,
                const SysChar *from_codeset,
                SysError     **error)
{
  GIConv cd;

  cd = sys_iconv_open (to_codeset, from_codeset);

  if (cd == (GIConv) -1)
    {
      /* Something went wrong.  */
      if (error)
        {
          if (errno == EINVAL)
            sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_NO_CONVERSION,
                         SYS_("Conversion from character set “%s” to “%s” is not supported"),
                         from_codeset, to_codeset);
          else
            sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_FAILED,
                         SYS_("Could not open converter from “%s” to “%s”"),
                         from_codeset, to_codeset);
        }
    }
  
  return cd;
}

static int
close_converter (GIConv cd)
{
  if (cd == (GIConv) -1)
    return 0;
  
  return sys_iconv_close (cd);  
}

/**
 * sys_convert_with_iconv: (skip)
 * @str:           (array length=len) (element-type SysUInt8):
 *                 the string to convert.
 * @len:           the length of the string in bytes, or -1 if the string is
 *                 nul-terminated (Note that some encodings may allow nul
 *                 bytes to occur inside strings. In that case, using -1
 *                 for the @len parameter is unsafe)
 * @converter:     conversion descriptor from sys_iconv_open()
 * @bytes_read:    (out) (optional): location to store the number of bytes in
 *                 the input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 %G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will be the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: (out) (optional): the number of bytes stored in
 *                 the output buffer (not including the terminating nul).
 * @error:         location to store the error occurring, or %NULL to ignore
 *                 errors. Any of the errors in #SysConvertError may occur.
 *
 * Converts a string from one character set to another. 
 * 
 * Note that you should use sys_iconv() for streaming conversions. 
 * Despite the fact that @bytes_read can return information about partial
 * characters, the sys_convert_... functions are not generally suitable
 * for streaming. If the underlying converter maintains internal state,
 * then this won't be preserved across successive calls to sys_convert(),
 * sys_convert_with_iconv() or sys_convert_with_fallback(). (An example of
 * this is the GNU C converter for CP1255 which does not emit a base
 * character until it knows that the next character is not a mark that
 * could combine with the base character.)
 *
 * Characters which are valid in the input character set, but which have no
 * representation in the output character set will result in a
 * %G_CONVERT_ERROR_ILLEGAL_SEQUENCE error. This is in contrast to the iconv()
 * specification, which leaves this behaviour implementation defined. Note that
 * this is the same error code as is returned for an invalid byte sequence in
 * the input character set. To get defined behaviour for conversion of
 * unrepresentable characters, use sys_convert_with_fallback().
 *
 * Returns: (array length=bytes_written) (element-type SysUInt8) (transfer full):
 *               If the conversion was successful, a newly allocated buffer
 *               containing the converted string, which must be freed with
 *               sys_free(). Otherwise %NULL and @error will be set.
 **/
SysChar*
sys_convert_with_iconv (const SysChar *str,
                      SysSize       len,
                      GIConv       converter,
                      SysSize       *bytes_read, 
                      SysSize       *bytes_written, 
                      SysError     **error)
{
  SysChar *dest;
  SysChar *outp;
  const SysChar *p;
  SysSize inbytes_remaining;
  SysSize outbytes_remaining;
  SysSize err;
  SysSize outbuf_size;
  SysBool have_error = false;
  SysBool done = false;
  SysBool reset = false;
  
  sys_return_val_if_fail (converter != (GIConv) -1, NULL);
     
  if (len < 0)
    len = strlen (str);

  p = str;
  inbytes_remaining = len;
  outbuf_size = len + NUL_TERMINATOR_LENGTH;
  
  outbytes_remaining = outbuf_size - NUL_TERMINATOR_LENGTH;
  outp = dest = sys_malloc (outbuf_size);

  while (!done && !have_error)
    {
      if (reset)
        err = sys_iconv (converter, NULL, &inbytes_remaining, &outp, &outbytes_remaining);
      else
        err = sys_iconv (converter, (char **)&p, &inbytes_remaining, &outp, &outbytes_remaining);

      if (err == (SysSize) -1)
        {
          switch (errno)
            {
            case EINVAL:
              /* Incomplete text, do not report an error */
              done = true;
              break;
            case E2BIG:
              {
                SysSize used = outp - dest;
                
                outbuf_size *= 2;
                dest = sys_realloc (dest, outbuf_size);
                
                outp = dest + used;
                outbytes_remaining = outbuf_size - used - NUL_TERMINATOR_LENGTH;
              }
              break;
            case EILSEQ:
              sys_error_set_N_literal (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                                   SYS_("Invalid byte sequence in conversion input"));
              have_error = true;
              break;
            default:
              {
                int errsv = errno;

                sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_FAILED,
                             SYS_("Error during conversion: %s"),
                             sys_strerror (errsv));
              }
              have_error = true;
              break;
            }
        }
      else if (err > 0)
        {
          /* @err gives the number of replacement characters used. */
          sys_error_set_N_literal (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                               SYS_("Unrepresentable character in conversion input"));
          have_error = true;
        }
      else 
        {
          if (!reset)
            {
              /* call sys_iconv with NULL inbuf to cleanup shift state */
              reset = true;
              inbytes_remaining = 0;
            }
          else
            done = true;
        }
    }

  memset (outp, 0, NUL_TERMINATOR_LENGTH);
  
  if (bytes_read)
    *bytes_read = p - str;
  else
    {
      if ((p - str) != len) 
        {
          if (!have_error)
            {
              sys_error_set_N_literal (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_PARTIAL_INPUT,
                                   SYS_("Partial character sequence at end of input"));
              have_error = true;
            }
        }
    }

  if (bytes_written)
    *bytes_written = outp - dest;        /* Doesn't include '\0' */

  if (have_error)
    {
      sys_free (dest);
      return NULL;
    }
  else
    return dest;
}

/**
 * sys_convert:
 * @str:           (array length=len) (element-type SysUInt8):
 *                 the string to convert.
 * @len:           the length of the string in bytes, or -1 if the string is
 *                 nul-terminated (Note that some encodings may allow nul
 *                 bytes to occur inside strings. In that case, using -1
 *                 for the @len parameter is unsafe)
 * @to_codeset:    name of character set into which to convert @str
 * @from_codeset:  character set of @str.
 * @bytes_read:    (out) (optional): location to store the number of bytes in
 *                 the input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 %G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will be the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: (out) (optional): the number of bytes stored in
 *                 the output buffer (not including the terminating nul).
 * @error:         location to store the error occurring, or %NULL to ignore
 *                 errors. Any of the errors in #SysConvertError may occur.
 *
 * Converts a string from one character set to another.
 *
 * Note that you should use sys_iconv() for streaming conversions. 
 * Despite the fact that @bytes_read can return information about partial
 * characters, the sys_convert_... functions are not generally suitable
 * for streaming. If the underlying converter maintains internal state,
 * then this won't be preserved across successive calls to sys_convert(),
 * sys_convert_with_iconv() or sys_convert_with_fallback(). (An example of
 * this is the GNU C converter for CP1255 which does not emit a base
 * character until it knows that the next character is not a mark that
 * could combine with the base character.)
 *
 * Using extensions such as "//TRANSLIT" may not work (or may not work
 * well) on many platforms.  Consider using sys_str_to_ascii() instead.
 *
 * Returns: (array length=bytes_written) (element-type SysUInt8) (transfer full):
 *          If the conversion was successful, a newly allocated buffer
 *          containing the converted string, which must be freed with sys_free().
 *          Otherwise %NULL and @error will be set.
 **/
SysChar*
sys_convert (const SysChar *str,
           SysSize       len,  
           const SysChar *to_codeset,
           const SysChar *from_codeset,
           SysSize       *bytes_read, 
           SysSize       *bytes_written, 
           SysError     **error)
{
  SysChar *res;
  GIConv cd;

  sys_return_val_if_fail (str != NULL, NULL);
  sys_return_val_if_fail (to_codeset != NULL, NULL);
  sys_return_val_if_fail (from_codeset != NULL, NULL);
  
  cd = open_converter (to_codeset, from_codeset, error);

  if (cd == (GIConv) -1)
    {
      if (bytes_read)
        *bytes_read = 0;
      
      if (bytes_written)
        *bytes_written = 0;
      
      return NULL;
    }

  res = sys_convert_with_iconv (str, len, cd,
                              bytes_read, bytes_written,
                              error);

  close_converter (cd);

  return res;
}

/**
 * sys_convert_with_fallback:
 * @str:          (array length=len) (element-type SysUInt8):
 *                the string to convert.
 * @len:          the length of the string in bytes, or -1 if the string is
 *                 nul-terminated (Note that some encodings may allow nul
 *                 bytes to occur inside strings. In that case, using -1
 *                 for the @len parameter is unsafe)
 * @to_codeset:   name of character set into which to convert @str
 * @from_codeset: character set of @str.
 * @fallback:     UTF-8 string to use in place of characters not
 *                present in the target encoding. (The string must be
 *                representable in the target encoding). 
 *                If %NULL, characters not in the target encoding will 
 *                be represented as Unicode escapes \uxxxx or \Uxxxxyyyy.
 * @bytes_read:   (out) (optional): location to store the number of bytes in
 *                the input string that were successfully converted, or %NULL.
 *                Even if the conversion was successful, this may be 
 *                less than @len if there were partial characters
 *                at the end of the input.
 * @bytes_written: (out) (optional): the number of bytes stored in
 *                 the output buffer (not including the terminating nul).
 * @error:        location to store the error occurring, or %NULL to ignore
 *                errors. Any of the errors in #SysConvertError may occur.
 *
 * Converts a string from one character set to another, possibly
 * including fallback sequences for characters not representable
 * in the output. Note that it is not guaranteed that the specification
 * for the fallback sequences in @fallback will be honored. Some
 * systems may do an approximate conversion from @from_codeset
 * to @to_codeset in their iconv() functions, 
 * in which case GLib will simply return that approximate conversion.
 *
 * Note that you should use sys_iconv() for streaming conversions. 
 * Despite the fact that @bytes_read can return information about partial
 * characters, the sys_convert_... functions are not generally suitable
 * for streaming. If the underlying converter maintains internal state,
 * then this won't be preserved across successive calls to sys_convert(),
 * sys_convert_with_iconv() or sys_convert_with_fallback(). (An example of
 * this is the GNU C converter for CP1255 which does not emit a base
 * character until it knows that the next character is not a mark that
 * could combine with the base character.)
 *
 * Returns: (array length=bytes_written) (element-type SysUInt8) (transfer full):
 *          If the conversion was successful, a newly allocated buffer
 *          containing the converted string, which must be freed with sys_free().
 *          Otherwise %NULL and @error will be set.
 **/
SysChar*
sys_convert_with_fallback (const SysChar *str,
                         SysSize       len,    
                         const SysChar *to_codeset,
                         const SysChar *from_codeset,
                         const SysChar *fallback,
                         SysSize       *bytes_read,
                         SysSize       *bytes_written,
                         SysError     **error)
{
  SysChar *utf8;
  SysChar *dest;
  SysChar *outp;
  const SysChar *insert_str = NULL;
  const SysChar *p;
  SysSize inbytes_remaining;   
  const SysChar *save_p = NULL;
  SysSize save_inbytes = 0;
  SysSize outbytes_remaining; 
  SysSize err;
  GIConv cd;
  SysSize outbuf_size;
  SysBool have_error = false;
  SysBool done = false;

  SysError *local_error = NULL;
  
  sys_return_val_if_fail (str != NULL, NULL);
  sys_return_val_if_fail (to_codeset != NULL, NULL);
  sys_return_val_if_fail (from_codeset != NULL, NULL);
     
  if (len < 0)
    len = strlen (str);
  
  /* Try an exact conversion; we only proceed if this fails
   * due to an illegal sequence in the input string.
   */
  dest = sys_convert (str, len, to_codeset, from_codeset, 
                    bytes_read, bytes_written, &local_error);
  if (!local_error)
    return dest;

  sys_assert (dest == NULL);

  if (!sys_error_matches (local_error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_ILLEGAL_SEQUENCE))
    {
      sys_propagate_error (error, local_error);
      return NULL;
    }
  else
    sys_error_N_free (local_error);

  local_error = NULL;
  
  /* No go; to proceed, we need a converter from "UTF-8" to
   * to_codeset, and the string as UTF-8.
   */
  cd = open_converter (to_codeset, "UTF-8", error);
  if (cd == (GIConv) -1)
    {
      if (bytes_read)
        *bytes_read = 0;
      
      if (bytes_written)
        *bytes_written = 0;
      
      return NULL;
    }

  utf8 = sys_convert (str, len, "UTF-8", from_codeset, 
                    bytes_read, &inbytes_remaining, error);
  if (!utf8)
    {
      close_converter (cd);
      if (bytes_written)
        *bytes_written = 0;
      return NULL;
    }

  /* Now the heart of the code. We loop through the UTF-8 string, and
   * whenever we hit an offending character, we form fallback, convert
   * the fallback to the target codeset, and then go back to
   * converting the original string after finishing with the fallback.
   *
   * The variables save_p and save_inbytes store the input state
   * for the original string while we are converting the fallback
   */
  p = utf8;

  outbuf_size = len + NUL_TERMINATOR_LENGTH;
  outbytes_remaining = outbuf_size - NUL_TERMINATOR_LENGTH;
  outp = dest = sys_malloc (outbuf_size);

  while (!done && !have_error)
    {
      SysSize inbytes_tmp = inbytes_remaining;
      err = sys_iconv (cd, (char **)&p, &inbytes_tmp, &outp, &outbytes_remaining);
      inbytes_remaining = inbytes_tmp;

      if (err == (SysSize) -1)
        {
          switch (errno)
            {
            case EINVAL:
              sys_assert_not_reached();
              break;
            case E2BIG:
              {
                SysSize used = outp - dest;

                outbuf_size *= 2;
                dest = sys_realloc (dest, outbuf_size);
                
                outp = dest + used;
                outbytes_remaining = outbuf_size - used - NUL_TERMINATOR_LENGTH;
                
                break;
              }
            case EILSEQ:
              if (save_p)
                {
                  /* Error converting fallback string - fatal
                   */
                  sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                               SYS_("Cannot convert fallback “%s” to codeset “%s”"),
                               insert_str, to_codeset);
                  have_error = true;
                  break;
                }
              else if (p)
                {
                  if (!fallback)
                    { 
                      SysUniChar ch = sys_utf8_get_char (p);
                      insert_str = sys_strdup_printf (ch < 0x10000 ? "\\u%04x" : "\\U%08x",
                                                    ch);
                    }
                  else
                    insert_str = fallback;
                  
                  save_p = sys_utf8_next_char (p);
                  save_inbytes = inbytes_remaining - (save_p - p);
                  p = insert_str;
                  inbytes_remaining = strlen (p);
                  break;
                }
              /* if p is null */
              SYS_GNUC_FALLTHROUGH;
            default:
              {
                int errsv = errno;

                sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_FAILED,
                             SYS_("Error during conversion: %s"),
                             sys_strerror (errsv));
              }

              have_error = true;
              break;
            }
        }
      else
        {
          if (save_p)
            {
              if (!fallback)
                sys_free ((SysChar *)insert_str);
              p = save_p;
              inbytes_remaining = save_inbytes;
              save_p = NULL;
            }
          else if (p)
            {
              /* call sys_iconv with NULL inbuf to cleanup shift state */
              p = NULL;
              inbytes_remaining = 0;
            }
          else
            done = true;
        }
    }

  /* Cleanup
   */
  memset (outp, 0, NUL_TERMINATOR_LENGTH);
  
  close_converter (cd);

  if (bytes_written)
    *bytes_written = outp - dest;        /* Doesn't include '\0' */

  sys_free (utf8);

  if (have_error)
    {
      if (save_p && !fallback)
        sys_free ((SysChar *)insert_str);
      sys_free (dest);
      return NULL;
    }
  else
    return dest;
}

/*
 * sys_locale_to_utf8
 *
 * 
 */

/*
 * Validate @string as UTF-8. @len can be negative if @string is
 * nul-terminated, or a non-negative value in bytes. If @string ends in an
 * incomplete sequence, or contains any illegal sequences or nul codepoints,
 * %NULL will be returned and the error set to
 * %G_CONVERT_ERROR_ILLEGAL_SEQUENCE.
 * On success, @bytes_read and @bytes_written, if provided, will be set to
 * the number of bytes in @string up to @len or the terminating nul byte.
 * On error, @bytes_read will be set to the byte offset after the last valid
 * and non-nul UTF-8 sequence in @string, and @bytes_written will be set to 0.
 */
static SysChar *
strdup_len (const SysChar *string,
            SysSize       len,
            SysSize       *bytes_read,
            SysSize       *bytes_written,
            SysError     **error)
{
  SysSize real_len;
  const SysChar *end_valid;

  if (!sys_utf8_validate (string, len, &end_valid))
    {
      if (bytes_read)
        *bytes_read = end_valid - string;
      if (bytes_written)
        *bytes_written = 0;

      sys_error_set_N_literal (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                           SYS_("Invalid byte sequence in conversion input"));
      return NULL;
    }

  real_len = end_valid - string;

  if (bytes_read)
    *bytes_read = real_len;
  if (bytes_written)
    *bytes_written = real_len;

  return sys_strndup (string, real_len);
}

typedef enum
{
  CONVERT_CHECK_NO_NULS_IN_INPUT  = 1 << 0,
  CONVERT_CHECK_NO_NULS_IN_OUTPUT = 1 << 1
} ConvertCheckFlags;

/*
 * Convert from @string in the encoding identified by @from_codeset,
 * returning a string in the encoding identifed by @to_codeset.
 * @len can be negative if @string is nul-terminated, or a non-negative
 * value in bytes. Flags defined in #ConvertCheckFlags can be set in @flags
 * to check the input, the output, or both, for embedded nul bytes.
 * On success, @bytes_read, if provided, will be set to the number of bytes
 * in @string up to @len or the terminating nul byte, and @bytes_written, if
 * provided, will be set to the number of output bytes written into the
 * returned buffer, excluding the terminating nul sequence.
 * On error, @bytes_read will be set to the byte offset after the last valid
 * sequence in @string, and @bytes_written will be set to 0.
 */
static SysChar *
convert_checked (const SysChar      *string,
                 SysSize            len,
                 const SysChar      *to_codeset,
                 const SysChar      *from_codeset,
                 ConvertCheckFlags flags,
                 SysSize            *bytes_read,
                 SysSize            *bytes_written,
                 SysError          **error)
{
  SysChar *out;
  SysSize outbytes;

  if ((flags & CONVERT_CHECK_NO_NULS_IN_INPUT) && len > 0)
    {
      const SysChar *early_nul = memchr (string, '\0', len);
      if (early_nul != NULL)
        {
          if (bytes_read)
            *bytes_read = early_nul - string;
          if (bytes_written)
            *bytes_written = 0;

          sys_error_set_N_literal (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                               SYS_("Embedded NUL byte in conversion input"));
          return NULL;
        }
    }

  out = sys_convert (string, len, to_codeset, from_codeset,
                   bytes_read, &outbytes, error);
  if (out == NULL)
    {
      if (bytes_written)
        *bytes_written = 0;
      return NULL;
    }

  if ((flags & CONVERT_CHECK_NO_NULS_IN_OUTPUT)
      && memchr (out, '\0', outbytes) != NULL)
    {
      sys_free (out);
      if (bytes_written)
        *bytes_written = 0;
      sys_error_set_N_literal (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_EMBEDDED_NUL,
                           SYS_("Embedded NUL byte in conversion output"));
      return NULL;
    }

  if (bytes_written)
    *bytes_written = outbytes;
  return out;
}

/**
 * sys_locale_to_utf8:
 * @opsysstring:   (array length=len) (element-type SysUInt8): a string in the
 *                 encoding of the current locale. On Windows
 *                 this means the system codepage.
 * @len:           the length of the string, or -1 if the string is
 *                 nul-terminated (Note that some encodings may allow nul
 *                 bytes to occur inside strings. In that case, using -1
 *                 for the @len parameter is unsafe)
 * @bytes_read: (out) (optional): location to store the number of bytes in the
 *                 input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 %G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will be the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: (out) (optional): the number of bytes stored in the output
 *                 buffer (not including the terminating nul).
 * @error:         location to store the error occurring, or %NULL to ignore
 *                 errors. Any of the errors in #SysConvertError may occur.
 * 
 * Converts a string which is in the encoding used for strings by
 * the C runtime (usually the same as that used by the operating
 * system) in the [current locale][setlocale] into a UTF-8 string.
 *
 * If the source encoding is not UTF-8 and the conversion output contains a
 * nul character, the error %G_CONVERT_ERROR_EMBEDDED_NUL is set and the
 * function returns %NULL.
 * If the source encoding is UTF-8, an embedded nul character is treated with
 * the %G_CONVERT_ERROR_ILLEGAL_SEQUENCE error for backward compatibility with
 * earlier versions of this library. Use sys_convert() to produce output that
 * may contain embedded nul characters.
 * 
 * Returns: (type utf8): The converted string, or %NULL on an error.
 **/
SysChar *
sys_locale_to_utf8 (const SysChar  *opsysstring,
                  SysSize        len,            
                  SysSize        *bytes_read,    
                  SysSize        *bytes_written,
                  SysError      **error)
{
  const char *charset;

  if (sys_get_charset (&charset))
    return strdup_len (opsysstring, len, bytes_read, bytes_written, error);
  else
    return convert_checked (opsysstring, len, "UTF-8", charset,
                            CONVERT_CHECK_NO_NULS_IN_OUTPUT,
                            bytes_read, bytes_written, error);
}

/*
 * Do the exact same as sys_locale_to_utf8 except that the charset would
 * be retrieved from _sys_get_time_charset (which uses LC_TIME)
 *
 * Returns: The converted string, or %NULL on an error.
 */
SysChar *
_sys_time_locale_to_utf8 (const SysChar *opsysstring,
                        SysSize       len,
                        SysSize       *bytes_read,
                        SysSize       *bytes_written,
                        SysError     **error)
{
  const char *charset;

  if (_sys_get_time_charset (&charset))
    return strdup_len (opsysstring, len, bytes_read, bytes_written, error);
  else
    return convert_checked (opsysstring, len, "UTF-8", charset,
                            CONVERT_CHECK_NO_NULS_IN_OUTPUT,
                            bytes_read, bytes_written, error);
}

/*
 * Do the exact same as sys_locale_to_utf8 except that the charset would
 * be retrieved from _sys_get_ctype_charset (which uses LC_CTYPE)
 *
 * Returns: The converted string, or %NULL on an error.
 */
SysChar *
_sys_ctype_locale_to_utf8 (const SysChar *opsysstring,
                         SysSize       len,
                         SysSize       *bytes_read,
                         SysSize       *bytes_written,
                         SysError     **error)
{
  const char *charset;

  if (_sys_get_ctype_charset (&charset))
    return strdup_len (opsysstring, len, bytes_read, bytes_written, error);
  else
    return convert_checked (opsysstring, len, "UTF-8", charset,
                            CONVERT_CHECK_NO_NULS_IN_OUTPUT,
                            bytes_read, bytes_written, error);
}

/**
 * sys_locale_from_utf8:
 * @utf8string:    a UTF-8 encoded string 
 * @len:           the length of the string, or -1 if the string is
 *                 nul-terminated.
 * @bytes_read: (out) (optional): location to store the number of bytes in the
 *                 input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 %G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will be the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: (out) (optional): the number of bytes stored in the output
 *                 buffer (not including the terminating nul).
 * @error:         location to store the error occurring, or %NULL to ignore
 *                 errors. Any of the errors in #SysConvertError may occur.
 * 
 * Converts a string from UTF-8 to the encoding used for strings by
 * the C runtime (usually the same as that used by the operating
 * system) in the [current locale][setlocale]. On Windows this means
 * the system codepage.
 *
 * The input string shall not contain nul characters even if the @len
 * argument is positive. A nul character found inside the string will result
 * in error %G_CONVERT_ERROR_ILLEGAL_SEQUENCE. Use sys_convert() to convert
 * input that may contain embedded nul characters.
 *
 * Returns: (array length=bytes_written) (element-type SysUInt8) (transfer full):
 *          A newly-allocated buffer containing the converted string,
 *          or %NULL on an error, and error will be set.
 **/
SysChar *
sys_locale_from_utf8 (const SysChar *utf8string,
                    SysSize       len,            
                    SysSize       *bytes_read,    
                    SysSize       *bytes_written,
                    SysError     **error)
{
  const SysChar *charset;

  if (sys_get_charset (&charset))
    return strdup_len (utf8string, len, bytes_read, bytes_written, error);
  else
    return convert_checked (utf8string, len, charset, "UTF-8",
                            CONVERT_CHECK_NO_NULS_IN_INPUT,
                            bytes_read, bytes_written, error);
}

#ifndef SYS_PLATFORM_WIN32

typedef struct _GFilenameCharsetCache GFilenameCharsetCache;

struct _GFilenameCharsetCache {
  SysBool is_utf8;
  SysChar *charset;
  SysChar **filename_charsets;
};

static void
filename_charset_cache_free (SysPointer data)
{
  GFilenameCharsetCache *cache = data;
  sys_free (cache->charset);
  sys_strfreev (cache->filename_charsets);
  sys_free (cache);
}

/**
 * sys_get_filename_charsets:
 * @filename_charsets: (out) (transfer none) (array zero-terminated=1):
 *    return location for the %NULL-terminated list of encoding names
 *
 * Determines the preferred character sets used for filenames.
 * The first character set from the @charsets is the filename encoding, the
 * subsequent character sets are used when trying to generate a displayable
 * representation of a filename, see sys_filename_display_name().
 *
 * On Unix, the character sets are determined by consulting the
 * environment variables `G_FILENAME_ENCODING` and `G_BROKEN_FILENAMES`.
 * On Windows, the character set used in the GLib API is always UTF-8
 * and said environment variables have no effect.
 *
 * `G_FILENAME_ENCODING` may be set to a comma-separated list of
 * character set names. The special token "\@locale" is taken
 * to  mean the character set for the [current locale][setlocale].
 * If `G_FILENAME_ENCODING` is not set, but `G_BROKEN_FILENAMES` is,
 * the character set of the current locale is taken as the filename
 * encoding. If neither environment variable  is set, UTF-8 is taken
 * as the filename encoding, but the character set of the current locale
 * is also put in the list of encodings.
 *
 * The returned @charsets belong to GLib and must not be freed.
 *
 * Note that on Unix, regardless of the locale character set or
 * `G_FILENAME_ENCODING` value, the actual file names present 
 * on a system might be in any random encoding or just gibberish.
 *
 * Returns: %true if the filename encoding is UTF-8.
 * 
 * Since: 2.6
 */
SysBool
sys_get_filename_charsets (const SysChar ***filename_charsets)
{
  static SysPrivate cache_private = SYS_PRIVATE_INIT (filename_charset_cache_free);
  GFilenameCharsetCache *cache = sys_private_get (&cache_private);
  const SysChar *charset;

  if (!cache)
    cache = sys_private_set_alloc0 (&cache_private, sizeof (GFilenameCharsetCache));

  sys_get_charset (&charset);

  if (!(cache->charset && strcmp (cache->charset, charset) == 0))
    {
      const SysChar *new_charset;
      const SysChar *p;
      SysInt i;

      sys_free (cache->charset);
      sys_strfreev (cache->filename_charsets);
      cache->charset = sys_strdup (charset);
      
      p = sys_env_get ("G_FILENAME_ENCODING");
      if (p != NULL && p[0] != '\0') 
        {
          cache->filename_charsets = sys_strsplit (p, ",", 0);
          cache->is_utf8 = (strcmp (cache->filename_charsets[0], "UTF-8") == 0);

          for (i = 0; cache->filename_charsets[i]; i++)
            {
              if (strcmp ("@locale", cache->filename_charsets[i]) == 0)
                {
                  sys_get_charset (&new_charset);
                  sys_free (cache->filename_charsets[i]);
                  cache->filename_charsets[i] = sys_strdup (new_charset);
                }
            }
        }
      else if (sys_env_get ("G_BROKEN_FILENAMES") != NULL)
        {
          cache->filename_charsets = sys_new0 (SysChar *, 2);
          cache->is_utf8 = sys_get_charset (&new_charset);
          cache->filename_charsets[0] = sys_strdup (new_charset);
        }
      else 
        {
          cache->filename_charsets = sys_new0 (SysChar *, 3);
          cache->is_utf8 = true;
          cache->filename_charsets[0] = sys_strdup ("UTF-8");
          if (!sys_get_charset (&new_charset))
            cache->filename_charsets[1] = sys_strdup (new_charset);
        }
    }

  if (filename_charsets)
    *filename_charsets = (const SysChar **)cache->filename_charsets;

  return cache->is_utf8;
}

#else /* SYS_PLATFORM_WIN32 */

SysBool
sys_get_filename_charsets (const SysChar ***filename_charsets) 
{
  static const SysChar *charsets[] = {
    "UTF-8",
    NULL
  };

#ifdef SYS_OS_WIN32
  /* On Windows GLib pretends that the filename charset is UTF-8 */
  if (filename_charsets)
    *filename_charsets = charsets;

  return true;
#else
  SysBool result;

  /* Cygwin works like before */
  result = sys_get_charset (&(charsets[0]));

  if (filename_charsets)
    *filename_charsets = charsets;

  return result;
#endif
}

#endif /* SYS_PLATFORM_WIN32 */

static SysBool
get_filename_charset (const SysChar **filename_charset)
{
  const SysChar **charsets;
  SysBool is_utf8;
  
  is_utf8 = sys_get_filename_charsets (&charsets);

  if (filename_charset)
    *filename_charset = charsets[0];
  
  return is_utf8;
}

/**
 * sys_filename_to_utf8:
 * @opsysstring: (type filename): a string in the encoding for filenames
 * @len:           the length of the string, or -1 if the string is
 *                 nul-terminated (Note that some encodings may allow nul
 *                 bytes to occur inside strings. In that case, using -1
 *                 for the @len parameter is unsafe)
 * @bytes_read: (out) (optional): location to store the number of bytes in the
 *                 input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 %G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will be the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: (out) (optional): the number of bytes stored in the output
 *                 buffer (not including the terminating nul).
 * @error:         location to store the error occurring, or %NULL to ignore
 *                 errors. Any of the errors in #SysConvertError may occur.
 * 
 * Converts a string which is in the encoding used by GLib for
 * filenames into a UTF-8 string. Note that on Windows GLib uses UTF-8
 * for filenames; on other platforms, this function indirectly depends on 
 * the [current locale][setlocale].
 *
 * The input string shall not contain nul characters even if the @len
 * argument is positive. A nul character found inside the string will result
 * in error %G_CONVERT_ERROR_ILLEGAL_SEQUENCE.
 * If the source encoding is not UTF-8 and the conversion output contains a
 * nul character, the error %G_CONVERT_ERROR_EMBEDDED_NUL is set and the
 * function returns %NULL. Use sys_convert() to produce output that
 * may contain embedded nul characters.
 * 
 * Returns: (type utf8): The converted string, or %NULL on an error.
 **/
SysChar*
sys_filename_to_utf8 (const SysChar *opsysstring, 
                    SysSize       len,           
                    SysSize       *bytes_read,   
                    SysSize       *bytes_written,
                    SysError     **error)
{
  const SysChar *charset;

  sys_return_val_if_fail (opsysstring != NULL, NULL);

  if (get_filename_charset (&charset))
    return strdup_len (opsysstring, len, bytes_read, bytes_written, error);
  else
    return convert_checked (opsysstring, len, "UTF-8", charset,
                            CONVERT_CHECK_NO_NULS_IN_INPUT |
                            CONVERT_CHECK_NO_NULS_IN_OUTPUT,
                            bytes_read, bytes_written, error);
}

/**
 * sys_filename_from_utf8:
 * @utf8string:    (type utf8): a UTF-8 encoded string.
 * @len:           the length of the string, or -1 if the string is
 *                 nul-terminated.
 * @bytes_read:    (out) (optional): location to store the number of bytes in
 *                 the input string that were successfully converted, or %NULL.
 *                 Even if the conversion was successful, this may be 
 *                 less than @len if there were partial characters
 *                 at the end of the input. If the error
 *                 %G_CONVERT_ERROR_ILLEGAL_SEQUENCE occurs, the value
 *                 stored will be the byte offset after the last valid
 *                 input sequence.
 * @bytes_written: (out) (optional): the number of bytes stored in
 *                 the output buffer (not including the terminating nul).
 * @error:         location to store the error occurring, or %NULL to ignore
 *                 errors. Any of the errors in #SysConvertError may occur.
 * 
 * Converts a string from UTF-8 to the encoding GLib uses for
 * filenames. Note that on Windows GLib uses UTF-8 for filenames;
 * on other platforms, this function indirectly depends on the 
 * [current locale][setlocale].
 *
 * The input string shall not contain nul characters even if the @len
 * argument is positive. A nul character found inside the string will result
 * in error %G_CONVERT_ERROR_ILLEGAL_SEQUENCE. If the filename encoding is
 * not UTF-8 and the conversion output contains a nul character, the error
 * %G_CONVERT_ERROR_EMBEDDED_NUL is set and the function returns %NULL.
 *
 * Returns: (type filename):
 *               The converted string, or %NULL on an error.
 **/
SysChar*
sys_filename_from_utf8 (const SysChar *utf8string,
                      SysSize       len,            
                      SysSize       *bytes_read,    
                      SysSize       *bytes_written,
                      SysError     **error)
{
  const SysChar *charset;

  if (get_filename_charset (&charset))
    return strdup_len (utf8string, len, bytes_read, bytes_written, error);
  else
    return convert_checked (utf8string, len, charset, "UTF-8",
                            CONVERT_CHECK_NO_NULS_IN_INPUT |
                            CONVERT_CHECK_NO_NULS_IN_OUTPUT,
                            bytes_read, bytes_written, error);
}

/* Test of haystack has the needle prefix, comparing case
 * insensitive. haystack may be UTF-8, but needle must
 * contain only ascii. */
static SysBool
has_case_prefix (const SysChar *haystack, const SysChar *needle)
{
  const SysChar *h, *n;
  
  /* Eat one character at a time. */
  h = haystack;
  n = needle;

  while (*n && *h &&
         sys_ascii_tolower (*n) == sys_ascii_tolower (*h))
    {
      n++;
      h++;
    }
  
  return *n == '\0';
}

typedef enum {
  UNSAFE_ALL        = 0x1,  /* Escape all unsafe characters   */
  UNSAFE_ALLOW_PLUS = 0x2,  /* Allows '+'  */
  UNSAFE_PATH       = 0x8,  /* Allows '/', '&', '=', ':', '@', '+', '$' and ',' */
  UNSAFE_HOST       = 0x10, /* Allows '/' and ':' and '@' */
  UNSAFE_SLASHES    = 0x20  /* Allows all characters except for '/' and '%' */
} UnsafeCharacterSet;

static const SysUChar acceptable[96] = {
  /* A table of the ASCII chars from space (32) to DEL (127) */
  /*      !    "    #    $    %    &    '    (    )    *    +    ,    -    .    / */ 
  0x00,0x3F,0x20,0x20,0x28,0x00,0x2C,0x3F,0x3F,0x3F,0x3F,0x2A,0x28,0x3F,0x3F,0x1C,
  /* 0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ? */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x38,0x20,0x20,0x2C,0x20,0x20,
  /* @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O */
  0x38,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
  /* P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _ */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x20,0x3F,
  /* `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o */
  0x20,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
  /* p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~  DEL */
  0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x20,0x20,0x20,0x3F,0x20
};

static const SysChar hex[] = "0123456789ABCDEF";

/* Note: This escape function works on file: URIs, but if you want to
 * escape something else, please read RFC-2396 */
static SysChar *
sys_escape_uri_string (const SysChar *string, 
                     UnsafeCharacterSet mask)
{
#define ACCEPTABLE(a) ((a)>=32 && (a)<128 && (acceptable[(a)-32] & use_mask))

  const SysChar *p;
  SysChar *q;
  SysChar *result;
  int c;
  SysInt unacceptable;
  UnsafeCharacterSet use_mask;
  
  sys_return_val_if_fail (mask == UNSAFE_ALL
                        || mask == UNSAFE_ALLOW_PLUS
                        || mask == UNSAFE_PATH
                        || mask == UNSAFE_HOST
                        || mask == UNSAFE_SLASHES, NULL);
  
  unacceptable = 0;
  use_mask = mask;
  for (p = string; *p != '\0'; p++)
    {
      c = (SysUChar) *p;
      if (!ACCEPTABLE (c)) 
        unacceptable++;
    }
  
  result = sys_malloc (p - string + unacceptable * 2 + 1);
  
  use_mask = mask;
  for (q = result, p = string; *p != '\0'; p++)
    {
      c = (SysUChar) *p;
      
      if (!ACCEPTABLE (c))
        {
          *q++ = '%'; /* means hex coming */
          *q++ = hex[c >> 4];
          *q++ = hex[c & 15];
        }
      else
        *q++ = *p;
    }
  
  *q = '\0';
  
  return result;
}


static SysChar *
sys_escape_file_uri (const SysChar *hostname,
                   const SysChar *pathname)
{
  char *escaped_hostname = NULL;
  char *escaped_path;
  char *res;

#ifdef SYS_OS_WIN32
  char *p, *backslash;

  /* Turn backslashes into forward slashes. That's what Netscape
   * does, and they are actually more or less equivalent in Windows.
   */
  
  pathname = sys_strdup (pathname);
  p = (char *) pathname;
  
  while ((backslash = strchr (p, '\\')) != NULL)
    {
      *backslash = '/';
      p = backslash + 1;
    }
#endif

  if (hostname && *hostname != '\0')
    {
      escaped_hostname = sys_escape_uri_string (hostname, UNSAFE_HOST);
    }

  escaped_path = sys_escape_uri_string (pathname, UNSAFE_PATH);

  res = sys_strconcat ("file://",
                     (escaped_hostname) ? escaped_hostname : "",
                     (*escaped_path != '/') ? "/" : "",
                     escaped_path,
                     NULL);

#ifdef SYS_OS_WIN32
  sys_free ((char *) pathname);
#endif

  sys_free (escaped_hostname);
  sys_free (escaped_path);
  
  return res;
}

static int
unescape_character (const char *scanner)
{
  int first_digit;
  int second_digit;

  first_digit = sys_ascii_xdigit_value (scanner[0]);
  if (first_digit < 0) 
    return -1;
  
  second_digit = sys_ascii_xdigit_value (scanner[1]);
  if (second_digit < 0) 
    return -1;
  
  return (first_digit << 4) | second_digit;
}

static SysChar *
sys_unescape_uri_string (const char *escaped,
                       int         len,
                       const char *illegal_escaped_characters,
                       SysBool    ascii_must_not_be_escaped)
{
  const SysChar *in, *in_end;
  SysChar *out, *result;
  int c;
  
  if (escaped == NULL)
    return NULL;

  if (len < 0)
    len = strlen (escaped);

  result = sys_malloc (len + 1);
  
  out = result;
  for (in = escaped, in_end = escaped + len; in < in_end; in++)
    {
      c = *in;

      if (c == '%')
        {
          /* catch partial escape sequences past the end of the substring */
          if (in + 3 > in_end)
            break;

          c = unescape_character (in + 1);

          /* catch bad escape sequences and NUL characters */
          if (c <= 0)
            break;

          /* catch escaped ASCII */
          if (ascii_must_not_be_escaped && c <= 0x7F)
            break;

          /* catch other illegal escaped characters */
          if (strchr (illegal_escaped_characters, c) != NULL)
            break;

          in += 2;
        }

      *out++ = c;
    }
  
  sys_assert (out - result <= len);
  *out = '\0';

  if (in != in_end)
    {
      sys_free (result);
      return NULL;
    }

  return result;
}

static SysBool
is_asciialphanum (SysUniChar c)
{
  return c <= 0x7F && sys_ascii_isalnum (c);
}

static SysBool
is_asciialpha (SysUniChar c)
{
  return c <= 0x7F && sys_ascii_isalpha (c);
}

/* allows an empty string */
static SysBool
hostname_validate (const char *hostname)
{
  const char *p;
  SysUniChar c, first_char, last_char;

  p = hostname;
  if (*p == '\0')
    return true;
  do
    {
      /* read in a label */
      c = sys_utf8_get_char (p);
      p = sys_utf8_next_char (p);
      if (!is_asciialphanum (c))
        return false;
      first_char = c;
      do
        {
          last_char = c;
          c = sys_utf8_get_char (p);
          p = sys_utf8_next_char (p);
        }
      while (is_asciialphanum (c) || c == '-');
      if (last_char == '-')
        return false;
      
      /* if that was the last label, check that it was a toplabel */
      if (c == '\0' || (c == '.' && *p == '\0'))
        return is_asciialpha (first_char);
    }
  while (c == '.');
  return false;
}

/**
 * sys_filename_from_uri:
 * @uri: a uri describing a filename (escaped, encoded in ASCII).
 * @hostname: (out) (optional) (nullable): Location to store hostname for the URI.
 *            If there is no hostname in the URI, %NULL will be
 *            stored in this location.
 * @error: location to store the error occurring, or %NULL to ignore
 *         errors. Any of the errors in #SysConvertError may occur.
 * 
 * Converts an escaped ASCII-encoded URI to a local filename in the
 * encoding used for filenames. 
 * 
 * Returns: (type filename): a newly-allocated string holding
 *               the resulting filename, or %NULL on an error.
 **/
SysChar * sys_filename_from_uri (const SysChar *uri,
                     SysChar      **hostname,
                     SysError     **error)
{
  const char *path_part;
  const char *host_part;
  char *unescaped_hostname;
  char *result;
  char *filename;
  int offs;
#ifdef SYS_OS_WIN32
  char *p, *slash;
#endif

  if (hostname)
    *hostname = NULL;

  if (!has_case_prefix (uri, "file:/"))
    {
      sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_BAD_URI,
                   SYS_("The URI “%s” is not an absolute URI using the “file” scheme"),
                   uri);
      return NULL;
    }
  
  path_part = uri + strlen ("file:");
  
  if (strchr (path_part, '#') != NULL)
    {
      sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_BAD_URI,
                   SYS_("The local file URI “%s” may not include a “#”"),
                   uri);
      return NULL;
    }
        
  if (has_case_prefix (path_part, "///")) 
    path_part += 2;
  else if (has_case_prefix (path_part, "//"))
    {
      path_part += 2;
      host_part = path_part;

      path_part = strchr (path_part, '/');

      if (path_part == NULL)
        {
          sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_BAD_URI,
                       SYS_("The URI “%s” is invalid"),
                       uri);
          return NULL;
        }

      unescaped_hostname = sys_unescape_uri_string (host_part, path_part - host_part, "", true);

      if (unescaped_hostname == NULL ||
          !hostname_validate (unescaped_hostname))
        {
          sys_free (unescaped_hostname);
          sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_BAD_URI,
                       SYS_("The hostname of the URI “%s” is invalid"),
                       uri);
          return NULL;
        }
      
      if (hostname)
        *hostname = unescaped_hostname;
      else
        sys_free (unescaped_hostname);
    }

  filename = sys_unescape_uri_string (path_part, -1, "/", false);

  if (filename == NULL)
    {
      sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_BAD_URI,
                   SYS_("The URI “%s” contains invalidly escaped characters"),
                   uri);
      return NULL;
    }

  offs = 0;
#ifdef SYS_OS_WIN32
  /* Drop localhost */
  if (hostname && *hostname != NULL &&
      sys_ascii_strcasecmp (*hostname, "localhost") == 0)
    {
      sys_free (*hostname);
      *hostname = NULL;
    }

  /* Turn slashes into backslashes, because that's the canonical spelling */
  p = filename;
  while ((slash = strchr (p, '/')) != NULL)
    {
      *slash = '\\';
      p = slash + 1;
    }

  /* Windows URIs with a drive letter can be like "file://host/c:/foo"
   * or "file://host/c|/foo" (some Netscape versions). In those cases, start
   * the filename from the drive letter.
   */
  if (sys_ascii_isalpha (filename[1]))
    {
      if (filename[2] == ':')
        offs = 1;
      else if (filename[2] == '|')
        {
          filename[2] = ':';
          offs = 1;
        }
    }
#endif

  result = sys_strdup (filename + offs);
  sys_free (filename);

  return result;
}

/**
 * sys_filename_to_uri:
 * @filename: (type filename): an absolute filename specified in the GLib file
 *     name encoding, which is the on-disk file name bytes on Unix, and UTF-8
 *     on Windows
 * @hostname: (nullable): A UTF-8 encoded hostname, or %NULL for none.
 * @error: location to store the error occurring, or %NULL to ignore
 *         errors. Any of the errors in #SysConvertError may occur.
 * 
 * Converts an absolute filename to an escaped ASCII-encoded URI, with the path
 * component following Section 3.3. of RFC 2396.
 * 
 * Returns: a newly-allocated string holding the resulting
 *               URI, or %NULL on an error.
 **/
SysChar *
sys_filename_to_uri (const SysChar *filename,
                   const SysChar *hostname,
                   SysError     **error)
{
  char *escaped_uri;

  sys_return_val_if_fail (filename != NULL, NULL);

  if (!sys_path_is_absolute (filename))
    {
      sys_error_set_N (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_NOT_ABSOLUTE_PATH,
                   SYS_("The pathname “%s” is not an absolute path"),
                   filename);
      return NULL;
    }

  if (hostname &&
      !(sys_utf8_validate (hostname, -1, NULL)
        && hostname_validate (hostname)))
    {
      sys_error_set_N_literal (error, SYS_CONVERT_ERROR, SYS_CONVERT_ERROR_ILLEGAL_SEQUENCE,
                           SYS_("Invalid hostname"));
      return NULL;
    }
  
#ifdef SYS_OS_WIN32
  /* Don't use localhost unnecessarily */
  if (hostname && sys_ascii_strcasecmp (hostname, "localhost") == 0)
    hostname = NULL;
#endif

  escaped_uri = sys_escape_file_uri (hostname, filename);

  return escaped_uri;
}

/**
 * sys_uri_list_extract_uris:
 * @uri_list: an URI list 
 *
 * Splits an URI list conforming to the text/uri-list
 * mime type defined in RFC 2483 into individual URIs,
 * discarding any comments. The URIs are not validated.
 *
 * Returns: (transfer full): a newly allocated %NULL-terminated list
 *   of strings holding the individual URIs. The array should be freed
 *   with sys_strfreev().
 *
 * Since: 2.6
 */
SysChar **
sys_uri_list_extract_uris (const SysChar *uri_list)
{
  SysPtrArray *uris;
  const SysChar *p, *q;

  uris = sys_ptr_array_new ();

  p = uri_list;

  /* We don't actually try to validate the URI according to RFC
   * 2396, or even check for allowed characters - we just ignore
   * comments and trim whitespace off the ends.  We also
   * allow LF delimination as well as the specified CRLF.
   *
   * We do allow comments like specified in RFC 2483.
   */
  while (p)
    {
      if (*p != '#')
        {
          while (sys_ascii_isspace (*p))
            p++;

          q = p;
          while (*q && (*q != '\n') && (*q != '\r'))
            q++;

          if (q > p)
            {
              q--;
              while (q > p && sys_ascii_isspace (*q))
                q--;

              if (q > p)
                sys_ptr_array_add (uris, sys_strndup (p, q - p + 1));
            }
        }
      p = strchr (p, '\n');
      if (p)
        p++;
    }

  sys_ptr_array_add (uris, NULL);

  return (SysChar **) sys_ptr_array_free (uris, false);
}

/**
 * sys_filename_display_basename:
 * @filename: (type filename): an absolute pathname in the
 *     GLib file name encoding
 *
 * Returns the display basename for the particular filename, guaranteed
 * to be valid UTF-8. The display name might not be identical to the filename,
 * for instance there might be problems converting it to UTF-8, and some files
 * can be translated in the display.
 *
 * If GLib cannot make sense of the encoding of @filename, as a last resort it 
 * replaces unknown characters with U+FFFD, the Unicode replacement character.
 * You can search the result for the UTF-8 encoding of this character (which is
 * "\357\277\275" in octal notation) to find out if @filename was in an invalid
 * encoding.
 *
 * You must pass the whole absolute pathname to this functions so that
 * translation of well known locations can be done.
 *
 * This function is preferred over sys_filename_display_name() if you know the
 * whole path, as it allows translation.
 *
 * Returns: a newly allocated string containing
 *   a rendition of the basename of the filename in valid UTF-8
 *
 * Since: 2.6
 **/
SysChar *
sys_filename_display_basename (const SysChar *filename)
{
  char *basename;
  char *display_name;

  sys_return_val_if_fail (filename != NULL, NULL);
  
  basename = sys_path_get_basename (filename);
  display_name = sys_filename_display_name (basename);
  sys_free (basename);
  return display_name;
}

/**
 * sys_filename_display_name:
 * @filename: (type filename): a pathname hopefully in the
 *     GLib file name encoding
 * 
 * Converts a filename into a valid UTF-8 string. The conversion is 
 * not necessarily reversible, so you should keep the original around 
 * and use the return value of this function only for display purposes.
 * Unlike sys_filename_to_utf8(), the result is guaranteed to be non-%NULL 
 * even if the filename actually isn't in the GLib file name encoding.
 *
 * If GLib cannot make sense of the encoding of @filename, as a last resort it 
 * replaces unknown characters with U+FFFD, the Unicode replacement character.
 * You can search the result for the UTF-8 encoding of this character (which is
 * "\357\277\275" in octal notation) to find out if @filename was in an invalid
 * encoding.
 *
 * If you know the whole pathname of the file you should use
 * sys_filename_display_basename(), since that allows location-based
 * translation of filenames.
 *
 * Returns: a newly allocated string containing
 *   a rendition of the filename in valid UTF-8
 *
 * Since: 2.6
 **/
SysChar *
sys_filename_display_name (const SysChar *filename)
{
  SysInt i;
  const SysChar **charsets;
  SysChar *display_name = NULL;
  SysBool is_utf8;
 
  is_utf8 = sys_get_filename_charsets (&charsets);

  if (is_utf8)
    {
      if (sys_utf8_validate (filename, -1, NULL))
        display_name = sys_strdup (filename);
    }
  
  if (!display_name)
    {
      /* Try to convert from the filename charsets to UTF-8.
       * Skip the first charset if it is UTF-8.
       */
      for (i = is_utf8 ? 1 : 0; charsets[i]; i++)
        {
          display_name = sys_convert (filename, -1, "UTF-8", charsets[i], 
                                    NULL, NULL, NULL);

          if (display_name)
            break;
        }
    }
  
  /* if all conversions failed, we replace invalid UTF-8
   * by a question mark
   */
  if (!display_name) 
    display_name = sys_utf8_make_valid (filename, -1);

  return display_name;
}

#ifdef SYS_OS_WIN32

/* Binary compatibility versions. Not for newly compiled code. */

_GLIB_EXTERN SysChar *sys_filename_to_utf8_utf8   (const SysChar  *opsysstring,
                                               SysSize        len,
                                               SysSize        *bytes_read,
                                               SysSize        *bytes_written,
                                               SysError      **error) SYS_GNUC_MALLOC;
_GLIB_EXTERN SysChar *sys_filename_from_utf8_utf8 (const SysChar  *utf8string,
                                               SysSize        len,
                                               SysSize        *bytes_read,
                                               SysSize        *bytes_written,
                                               SysError      **error) SYS_GNUC_MALLOC;
_GLIB_EXTERN SysChar *sys_filename_from_uri_utf8  (const SysChar  *uri,
                                               SysChar       **hostname,
                                               SysError      **error) SYS_GNUC_MALLOC;
_GLIB_EXTERN SysChar *sys_filename_to_uri_utf8    (const SysChar  *filename,
                                               const SysChar  *hostname,
                                               SysError      **error) SYS_GNUC_MALLOC;

SysChar *
sys_filename_to_utf8_utf8 (const SysChar *opsysstring,
                         SysSize       len,
                         SysSize       *bytes_read,
                         SysSize       *bytes_written,
                         SysError     **error)
{
  return sys_filename_to_utf8 (opsysstring, len, bytes_read, bytes_written, error);
}

SysChar *
sys_filename_from_utf8_utf8 (const SysChar *utf8string,
                           SysSize       len,
                           SysSize       *bytes_read,
                           SysSize       *bytes_written,
                           SysError     **error)
{
  return sys_filename_from_utf8 (utf8string, len, bytes_read, bytes_written, error);
}

SysChar * sys_filename_from_uri_utf8 (const SysChar *uri,
                          SysChar      **hostname,
                          SysError     **error)
{
  return sys_filename_from_uri (uri, hostname, error);
}

SysChar *
sys_filename_to_uri_utf8 (const SysChar *filename,
                        const SysChar *hostname,
                        SysError     **error)
{
  return sys_filename_to_uri (filename, hostname, error);
}

#endif
