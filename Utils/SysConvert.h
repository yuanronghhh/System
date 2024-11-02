#ifndef __SYS_CONVERT_H__
#define __SYS_CONVERT_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

/**
 * SysConvertError:
 * @G_CONVERT_ERROR_NO_CONVERSION: Conversion between the requested character
 *     sets is not supported.
 * @G_CONVERT_ERROR_ILLEGAL_SEQUENCE: Invalid byte sequence in conversion input;
 *    or the character sequence could not be represented in the target
 *    character set.
 * @G_CONVERT_ERROR_FAILED: Conversion failed for some reason.
 * @G_CONVERT_ERROR_PARTIAL_INPUT: Partial character sequence at end of input.
 * @G_CONVERT_ERROR_BAD_URI: URI is invalid.
 * @G_CONVERT_ERROR_NOT_ABSOLUTE_PATH: Pathname is not an absolute path.
 * @G_CONVERT_ERROR_NO_MEMORY: No memory available. Since: 2.40
 * @G_CONVERT_ERROR_EMBEDDED_NUL: An embedded NUL character is present in
 *     conversion output where a NUL-terminated string is expected.
 *     Since: 2.56
 *
 * Error codes returned by character set conversion routines.
 */
typedef enum
{
  SYS_CONVERT_ERROR_NO_CONVERSION,
  SYS_CONVERT_ERROR_ILLEGAL_SEQUENCE,
  SYS_CONVERT_ERROR_FAILED,
  SYS_CONVERT_ERROR_PARTIAL_INPUT,
  SYS_CONVERT_ERROR_BAD_URI,
  SYS_CONVERT_ERROR_NOT_ABSOLUTE_PATH,
  SYS_CONVERT_ERROR_NO_MEMORY,
  SYS_CONVERT_ERROR_EMBEDDED_NUL
} SysConvertError;

/**
 * SYS_CONVERT_ERROR:
 *
 * Error domain for character set conversions. Errors in this domain will
 * be from the #SysConvertError enumeration. See #SysError for information on
 * error domains.
 */
#define SYS_CONVERT_ERROR sys_convert_error_quark()
SysQuark sys_convert_error_quark (void);

/**
 * GIConv: (skip)
 *
 * The GIConv struct wraps an iconv() conversion descriptor. It contains
 * private data and should only be accessed using the following functions.
 */
typedef struct _GIConv *GIConv;

GIConv sys_iconv_open   (const SysChar  *to_codeset,
                       const SysChar  *from_codeset);
SysSize  sys_iconv        (GIConv        converter,
                       SysChar       **inbuf,
                       SysSize        *inbytes_left,
                       SysChar       **outbuf,
                       SysSize        *outbytes_left);
SysInt   sys_iconv_close  (GIConv        converter);


SysChar* sys_convert               (const SysChar  *str,
                                SysSize        len,            
                                const SysChar  *to_codeset,
                                const SysChar  *from_codeset,
                                SysSize        *bytes_read,     
                                SysSize        *bytes_written,  
                                SysError      **error) SYS_GNUC_MALLOC;
SysChar* sys_convert_with_iconv    (const SysChar  *str,
                                SysSize        len,
                                GIConv        converter,
                                SysSize        *bytes_read,     
                                SysSize        *bytes_written,  
                                SysError      **error) SYS_GNUC_MALLOC;
SysChar* sys_convert_with_fallback (const SysChar  *str,
                                SysSize        len,            
                                const SysChar  *to_codeset,
                                const SysChar  *from_codeset,
                                const SysChar  *fallback,
                                SysSize        *bytes_read,     
                                SysSize        *bytes_written,  
                                SysError      **error) SYS_GNUC_MALLOC;


/* Convert between libc's idea of strings and UTF-8.
 */
SysChar* sys_locale_to_utf8   (const SysChar  *opsysstring,
                           SysSize        len,            
                           SysSize        *bytes_read,     
                           SysSize        *bytes_written,  
                           SysError      **error) SYS_GNUC_MALLOC;
SysChar* sys_locale_from_utf8 (const SysChar  *utf8string,
                           SysSize        len,            
                           SysSize        *bytes_read,     
                           SysSize        *bytes_written,  
                           SysError      **error) SYS_GNUC_MALLOC;

/* Convert between the operating system (or C runtime)
 * representation of file names and UTF-8.
 */
SysChar* sys_filename_to_utf8   (const SysChar  *opsysstring,
                             SysSize        len,            
                             SysSize        *bytes_read,     
                             SysSize        *bytes_written,  
                             SysError      **error) SYS_GNUC_MALLOC;
SysChar* sys_filename_from_utf8 (const SysChar  *utf8string,
                             SysSize        len,            
                             SysSize        *bytes_read,     
                             SysSize        *bytes_written,  
                             SysError      **error) SYS_GNUC_MALLOC;

SysChar *sys_filename_from_uri (const SysChar *uri,
                            SysChar      **hostname,
                            SysError     **error) SYS_GNUC_MALLOC;
  
SysChar *sys_filename_to_uri   (const SysChar *filename,
                            const SysChar *hostname,
                            SysError     **error) SYS_GNUC_MALLOC;
SysChar *sys_filename_display_name (const SysChar *filename) SYS_GNUC_MALLOC;
SysBool sys_get_filename_charsets (const SysChar ***filename_charsets);

SysChar *sys_filename_display_basename (const SysChar *filename) SYS_GNUC_MALLOC;

SysChar **g_uri_list_extract_uris (const SysChar *uri_list);

SYS_END_DECLS

#endif
