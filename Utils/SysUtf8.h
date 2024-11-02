#ifndef __SYS_UTF8_H__
#define __SYS_UTF8_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

/* Array of skip-bytes-per-initial character.
*/
extern const SysChar * const sys_utf8_skip;

/**
 * sys_utf8_next_char:
 * @p: Pointer to the start of a valid UTF-8 character
 *
 * Skips to the next character in a UTF-8 string.
 *
 * The string must be valid; this macro is as fast as possible, and has
 * no error-checking.
 *
 * You would use this macro to iterate over a string character by character.
 *
 * The macro returns the start of the next UTF-8 character.
 *
 * Before using this macro, use sys_utf8_validate() to validate strings
 * that may contain invalid UTF-8.
 */
#define sys_utf8_next_char(p) (char *)((p) + sys_utf8_skip[*(const SysUChar *)(p)])

SysUniChar sys_utf8_get_char           (const SysChar  *p) SYS_GNUC_PURE;
SysUniChar sys_utf8_get_char_validated (const  SysChar *p,
                                    SysSize        max_len) SYS_GNUC_PURE;

SysChar*   sys_utf8_offset_to_pointer (const SysChar *str,
                                   SysLong        offset) SYS_GNUC_PURE;
SysLong    sys_utf8_pointer_to_offset (const SysChar *str,
                                   const SysChar *pos) SYS_GNUC_PURE;
SysChar*   sys_utf8_prev_char         (const SysChar *p) SYS_GNUC_PURE;
SysChar*   sys_utf8_find_next_char    (const SysChar *p,
                                   const SysChar *end) SYS_GNUC_PURE;
SysChar*   sys_utf8_find_prev_char    (const SysChar *str,
                                   const SysChar *p) SYS_GNUC_PURE;

SysLong    sys_utf8_strlen            (const SysChar *p,
                                   SysSize       max) SYS_GNUC_PURE;

SysChar   *sys_utf8_substring         (const SysChar *str,
                                   SysLong        start_pos,
                                   SysLong        end_pos) SYS_GNUC_MALLOC;

SysChar   *sys_utf8_strncpy           (SysChar       *dest,
                                   const SysChar *src,
                                   SysSize        n);

/* Find the UTF-8 character corresponding to ch, in string p. These
   functions are equivalants to strchr and strrchr */
SysChar* sys_utf8_strchr  (const SysChar *p,
                       SysSize       len,
                       SysUniChar     c);
SysChar* sys_utf8_strrchr (const SysChar *p,
                       SysSize       len,
                       SysUniChar     c);
SysChar* sys_utf8_strreverse (const SysChar *str,
                          SysSize len);

SysUniChar2 *sys_utf8_to_utf16     (const SysChar      *str,
                                SysLong             len,
                                SysLong            *items_read,
                                SysLong            *items_written,
                                SysError          **error) SYS_GNUC_MALLOC;
SysUniChar * sys_utf8_to_ucs4      (const SysChar      *str,
                                SysLong             len,
                                SysLong            *items_read,
                                SysLong            *items_written,
                                SysError          **error) SYS_GNUC_MALLOC;
SysUniChar * sys_utf8_to_ucs4_fast (const SysChar      *str,
                                SysLong             len,
                                SysLong            *items_written) SYS_GNUC_MALLOC;
SysUniChar * sys_utf16_to_ucs4     (const SysUniChar2  *str,
                                SysLong             len,
                                SysLong            *items_read,
                                SysLong            *items_written,
                                SysError          **error) SYS_GNUC_MALLOC;
SysChar*     sys_utf16_to_utf8     (const SysUniChar2  *str,
                                SysLong             len,
                                SysLong            *items_read,
                                SysLong            *items_written,
                                SysError          **error) SYS_GNUC_MALLOC;
SysUniChar2 *g_ucs4_to_utf16     (const SysUniChar   *str,
                                SysLong             len,
                                SysLong            *items_read,
                                SysLong            *items_written,
                                SysError          **error) SYS_GNUC_MALLOC;
SysChar*     sys_ucs4_to_utf8      (const SysUniChar   *str,
                                SysLong             len,
                                SysLong            *items_read,
                                SysLong            *items_written,
                                SysError          **error) SYS_GNUC_MALLOC;

SysInt      sys_unichar_to_utf8 (SysUniChar    c,
                             SysChar      *outbuf);

SysBool sys_utf8_validate (const SysChar  *str,
                          SysSize        max_len,
                          const SysChar **end);
SysBool sys_utf8_validate_len (const SysChar  *str,
                              SysSize         max_len,
                              const SysChar **end);

SysChar *sys_utf8_strup   (const SysChar *str,
                       SysSize       len) SYS_GNUC_MALLOC;
SysChar *sys_utf8_strdown (const SysChar *str,
                       SysSize       len) SYS_GNUC_MALLOC;
SysChar *sys_utf8_casefold (const SysChar *str,
                        SysSize       len) SYS_GNUC_MALLOC;

/**
 * GNormalizeMode:
 * @G_NORMALIZE_DEFAULT: standardize differences that do not affect the
 *     text content, such as the above-mentioned accent representation
 * @G_NORMALIZE_NFD: another name for %G_NORMALIZE_DEFAULT
 * @G_NORMALIZE_DEFAULT_COMPOSE: like %G_NORMALIZE_DEFAULT, but with
 *     composed forms rather than a maximally decomposed form
 * @G_NORMALIZE_NFC: another name for %G_NORMALIZE_DEFAULT_COMPOSE
 * @G_NORMALIZE_ALL: beyond %G_NORMALIZE_DEFAULT also standardize the
 *     "compatibility" characters in Unicode, such as SUPERSCRIPT THREE
 *     to the standard forms (in this case DIGIT THREE). Formatting
 *     information may be lost but for most text operations such
 *     characters should be considered the same
 * @G_NORMALIZE_NFKD: another name for %G_NORMALIZE_ALL
 * @G_NORMALIZE_ALL_COMPOSE: like %G_NORMALIZE_ALL, but with composed
 *     forms rather than a maximally decomposed form
 * @G_NORMALIZE_NFKC: another name for %G_NORMALIZE_ALL_COMPOSE
 *
 * Defines how a Unicode string is transformed in a canonical
 * form, standardizing such issues as whether a character with
 * an accent is represented as a base character and combining
 * accent or as a single precomposed character. Unicode strings
 * should generally be normalized before comparing them.
 */
typedef enum {
  SYS_NORMALIZE_DEFAULT,
  SYS_NORMALIZE_NFD = SYS_NORMALIZE_DEFAULT,
  SYS_NORMALIZE_DEFAULT_COMPOSE,
  SYS_NORMALIZE_NFC = SYS_NORMALIZE_DEFAULT_COMPOSE,
  SYS_NORMALIZE_ALL,
  SYS_NORMALIZE_NFKD = SYS_NORMALIZE_ALL,
  SYS_NORMALIZE_ALL_COMPOSE,
  SYS_NORMALIZE_NFKC = SYS_NORMALIZE_ALL_COMPOSE
} GNormalizeMode;

SysChar *sys_utf8_normalize (const SysChar   *str,
                         SysSize         len,
                         GNormalizeMode mode) SYS_GNUC_MALLOC;

SysInt   sys_utf8_collate     (const SysChar *str1,
                           const SysChar *str2) SYS_GNUC_PURE;
SysChar *sys_utf8_collate_key (const SysChar *str,
                           SysSize       len) SYS_GNUC_MALLOC;
SysChar *sys_utf8_collate_key_for_filename (const SysChar *str,
                                        SysSize       len) SYS_GNUC_MALLOC;

SysChar *sys_utf8_make_valid (const SysChar *str,
                          SysSize       len) SYS_GNUC_MALLOC;

SysBool sys_utf8_validate (const SysChar  *str,
                          SysSize        max_len,
                          const SysChar **end);
SysBool sys_utf8_validate_len (const SysChar  *str,
                              SysSize         max_len,
                              const SysChar **end);

SysChar *sys_utf8_strup   (const SysChar *str,
                       SysSize       len) SYS_GNUC_MALLOC;
SysChar *sys_utf8_strdown (const SysChar *str,
                       SysSize       len) SYS_GNUC_MALLOC;
SysChar *sys_utf8_casefold (const SysChar *str,
                        SysSize       len) SYS_GNUC_MALLOC;


SYS_END_DECLS

#endif
