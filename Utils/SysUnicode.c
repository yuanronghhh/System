#include <System/Utils/SysUnicode.h>
#include <System/Utils/SysUtf8.h>
#include <System/Utils/SysUnicodeTable.h>
#include <System/Utils/SysUnicodeScriptTable.h>
#include <System/Utils/SysUnicodeMirrorTable.h>

#define SYS_UNICHAR_FULLWIDTH_A 0xff21
#define SYS_UNICHAR_FULLWIDTH_I 0xff29
#define SYS_UNICHAR_FULLWIDTH_J 0xff2a
#define SYS_UNICHAR_FULLWIDTH_F 0xff26
#define SYS_UNICHAR_FULLWIDTH_a 0xff41
#define SYS_UNICHAR_FULLWIDTH_f 0xff46

#define ATTR_TABLE(Page) (((Page) <= SYS_UNICODE_LAST_PAGE_PART1) \
                          ? attr_table_part1[Page] \
                          : attr_table_part2[(Page) - 0xe00])

#define ATTTABLE(Page, Char) \
  ((ATTR_TABLE(Page) == SYS_UNICODE_MAX_TABLE_INDEX) ? 0 : (attr_data[ATTR_TABLE(Page)][Char]))

#define TTYPE_PART1(Page, Char) \
  ((type_table_part1[Page] >= SYS_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part1[Page] - SYS_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part1[Page]][Char]))

#define TTYPE_PART2(Page, Char) \
  ((type_table_part2[Page] >= SYS_UNICODE_MAX_TABLE_INDEX) \
   ? (type_table_part2[Page] - SYS_UNICODE_MAX_TABLE_INDEX) \
   : (type_data[type_table_part2[Page]][Char]))

#define TYPE(Char) \
  (((Char) <= SYS_UNICODE_LAST_CHAR_PART1) \
   ? TTYPE_PART1 ((Char) >> 8, (Char) & 0xff) \
   : (((Char) >= 0xe0000 && (Char) <= SYS_UNICODE_LAST_CHAR) \
      ? TTYPE_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
      : SYS_UNICODE_UNASSIGNED))


#define IS(Type, Class)        (((SysUInt)1 << (Type)) & (Class))
#define OR(Type, Rest)        (((SysUInt)1 << (Type)) | (Rest))



#define ISALPHA(Type)        IS ((Type),                                \
                            OR (SYS_UNICODE_LOWERCASE_LETTER,        \
                            OR (SYS_UNICODE_UPPERCASE_LETTER,        \
                            OR (SYS_UNICODE_TITLECASE_LETTER,        \
                            OR (SYS_UNICODE_MODIFIER_LETTER,        \
                            OR (SYS_UNICODE_OTHER_LETTER,                0))))))

#define ISALDIGIT(Type)        IS ((Type),                                \
                            OR (SYS_UNICODE_DECIMAL_NUMBER,        \
                            OR (SYS_UNICODE_LETTER_NUMBER,        \
                            OR (SYS_UNICODE_OTHER_NUMBER,                \
                            OR (SYS_UNICODE_LOWERCASE_LETTER,        \
                            OR (SYS_UNICODE_UPPERCASE_LETTER,        \
                            OR (SYS_UNICODE_TITLECASE_LETTER,        \
                            OR (SYS_UNICODE_MODIFIER_LETTER,        \
                            OR (SYS_UNICODE_OTHER_LETTER,                0)))))))))

#define ISMARK(Type)        IS ((Type),                                \
                            OR (SYS_UNICODE_NON_SPACING_MARK,        \
                            OR (SYS_UNICODE_SPACING_MARK,        \
                            OR (SYS_UNICODE_ENCLOSING_MARK,        0))))

#define ISZEROWIDTHTYPE(Type)        IS ((Type),                        \
                            OR (SYS_UNICODE_NON_SPACING_MARK,        \
                            OR (SYS_UNICODE_ENCLOSING_MARK,        \
                            OR (SYS_UNICODE_FORMAT,                0))))

/**
 * sys_unichar_isalnum:
 * @c: a Unicode character
 * 
 * Determines whether a character is alphanumeric.
 * Given some UTF-8 text, obtain a character value
 * with sys_utf8_get_char().
 * 
 * Returns: %true if @c is an alphanumeric character
 **/
SysBool
sys_unichar_isalnum (SysUniChar c)
{
  return ISALDIGIT (TYPE (c)) ? true : false;
}

/**
 * sys_unichar_isalpha:
 * @c: a Unicode character
 * 
 * Determines whether a character is alphabetic (i.e. a letter).
 * Given some UTF-8 text, obtain a character value with
 * sys_utf8_get_char().
 * 
 * Returns: %true if @c is an alphabetic character
 **/
SysBool
sys_unichar_isalpha (SysUniChar c)
{
  return ISALPHA (TYPE (c)) ? true : false;
}


/**
 * sys_unichar_iscntrl:
 * @c: a Unicode character
 * 
 * Determines whether a character is a control character.
 * Given some UTF-8 text, obtain a character value with
 * sys_utf8_get_char().
 * 
 * Returns: %true if @c is a control character
 **/
SysBool
sys_unichar_iscntrl (SysUniChar c)
{
  return TYPE (c) == SYS_UNICODE_CONTROL;
}

/**
 * sys_unichar_isdigit:
 * @c: a Unicode character
 * 
 * Determines whether a character is numeric (i.e. a digit).  This
 * covers ASCII 0-9 and also digits in other languages/scripts.  Given
 * some UTF-8 text, obtain a character value with sys_utf8_get_char().
 * 
 * Returns: %true if @c is a digit
 **/
SysBool
sys_unichar_isdigit (SysUniChar c)
{
  return TYPE (c) == SYS_UNICODE_DECIMAL_NUMBER;
}


/**
 * sys_unichar_isgraph:
 * @c: a Unicode character
 * 
 * Determines whether a character is printable and not a space
 * (returns %false for control characters, format characters, and
 * spaces). sys_unichar_isprint() is similar, but returns %true for
 * spaces. Given some UTF-8 text, obtain a character value with
 * sys_utf8_get_char().
 * 
 * Returns: %true if @c is printable unless it's a space
 **/
SysBool
sys_unichar_isgraph (SysUniChar c)
{
  return !IS (TYPE(c),
              OR (SYS_UNICODE_CONTROL,
              OR (SYS_UNICODE_FORMAT,
              OR (SYS_UNICODE_UNASSIGNED,
              OR (SYS_UNICODE_SURROGATE,
              OR (SYS_UNICODE_SPACE_SEPARATOR,
             0))))));
}

/**
 * sys_unichar_islower:
 * @c: a Unicode character
 * 
 * Determines whether a character is a lowercase letter.
 * Given some UTF-8 text, obtain a character value with
 * sys_utf8_get_char().
 * 
 * Returns: %true if @c is a lowercase letter
 **/
SysBool
sys_unichar_islower (SysUniChar c)
{
  return TYPE (c) == SYS_UNICODE_LOWERCASE_LETTER;
}


/**
 * sys_unichar_isprint:
 * @c: a Unicode character
 * 
 * Determines whether a character is printable.
 * Unlike sys_unichar_isgraph(), returns %true for spaces.
 * Given some UTF-8 text, obtain a character value with
 * sys_utf8_get_char().
 * 
 * Returns: %true if @c is printable
 **/
SysBool
sys_unichar_isprint (SysUniChar c)
{
  return !IS (TYPE(c),
              OR (SYS_UNICODE_CONTROL,
              OR (SYS_UNICODE_FORMAT,
              OR (SYS_UNICODE_UNASSIGNED,
              OR (SYS_UNICODE_SURROGATE,
             0)))));
}

/**
 * sys_unichar_ispunct:
 * @c: a Unicode character
 * 
 * Determines whether a character is punctuation or a symbol.
 * Given some UTF-8 text, obtain a character value with
 * sys_utf8_get_char().
 * 
 * Returns: %true if @c is a punctuation or symbol character
 **/
SysBool
sys_unichar_ispunct (SysUniChar c)
{
  return IS (TYPE(c),
             OR (SYS_UNICODE_CONNECT_PUNCTUATION,
             OR (SYS_UNICODE_DASH_PUNCTUATION,
             OR (SYS_UNICODE_CLOSE_PUNCTUATION,
             OR (SYS_UNICODE_FINAL_PUNCTUATION,
             OR (SYS_UNICODE_INITIAL_PUNCTUATION,
             OR (SYS_UNICODE_OTHER_PUNCTUATION,
             OR (SYS_UNICODE_OPEN_PUNCTUATION,
             OR (SYS_UNICODE_CURRENCY_SYMBOL,
             OR (SYS_UNICODE_MODIFIER_SYMBOL,
             OR (SYS_UNICODE_MATH_SYMBOL,
             OR (SYS_UNICODE_OTHER_SYMBOL,
            0)))))))))))) ? true : false;
}

/**
 * sys_unichar_isspace:
 * @c: a Unicode character
 * 
 * Determines whether a character is a space, tab, or line separator
 * (newline, carriage return, etc.).  Given some UTF-8 text, obtain a
 * character value with sys_utf8_get_char().
 *
 * (Note: don't use this to do word breaking; you have to use
 * Pango or equivalent to get word breaking right, the algorithm
 * is fairly complex.)
 *  
 * Returns: %true if @c is a space character
 **/
SysBool
sys_unichar_isspace (SysUniChar c)
{
  switch (c)
    {
      /* special-case these since Unicode thinks they are not spaces */
    case '\t':
    case '\n':
    case '\r':
    case '\f':
      return true;
      break;
      
    default:
      {
        return IS (TYPE(c),
                   OR (SYS_UNICODE_SPACE_SEPARATOR,
                   OR (SYS_UNICODE_LINE_SEPARATOR,
                   OR (SYS_UNICODE_PARAGRAPH_SEPARATOR,
                  0)))) ? true : false;
      }
      break;
    }
}

/**
 * sys_unichar_ismark:
 * @c: a Unicode character
 *
 * Determines whether a character is a mark (non-spacing mark,
 * combining mark, or enclosing mark in Unicode speak).
 * Given some UTF-8 text, obtain a character value
 * with sys_utf8_get_char().
 *
 * Note: in most cases where isalpha characters are allowed,
 * ismark characters should be allowed to as they are essential
 * for writing most European languages as well as many non-Latin
 * scripts.
 *
 * Returns: %true if @c is a mark character
 *
 * Since: 2.14
 **/
SysBool
sys_unichar_ismark (SysUniChar c)
{
  return ISMARK (TYPE (c));
}

/**
 * sys_unichar_isupper:
 * @c: a Unicode character
 * 
 * Determines if a character is uppercase.
 * 
 * Returns: %true if @c is an uppercase character
 **/
SysBool
sys_unichar_isupper (SysUniChar c)
{
  return TYPE (c) == SYS_UNICODE_UPPERCASE_LETTER;
}

/**
 * sys_unichar_istitle:
 * @c: a Unicode character
 * 
 * Determines if a character is titlecase. Some characters in
 * Unicode which are composites, such as the DZ digraph
 * have three case variants instead of just two. The titlecase
 * form is used at the beginning of a word where only the
 * first letter is capitalized. The titlecase form of the DZ
 * digraph is U+01F2 LATIN CAPITAL LETTTER D WITH SMALL LETTER Z.
 * 
 * Returns: %true if the character is titlecase
 **/
SysBool
sys_unichar_istitle (SysUniChar c)
{
  unsigned int i;
  for (i = 0; i < ARRAY_SIZE (title_table); ++i)
    if (title_table[i][0] == c)
      return true;
  return false;
}

/**
 * sys_unichar_isxdigit:
 * @c: a Unicode character.
 * 
 * Determines if a character is a hexadecimal digit.
 * 
 * Returns: %true if the character is a hexadecimal digit
 **/
SysBool
sys_unichar_isxdigit (SysUniChar c)
{
  return ((c >= 'a' && c <= 'f') ||
          (c >= 'A' && c <= 'F') ||
          (c >= SYS_UNICHAR_FULLWIDTH_a && c <= SYS_UNICHAR_FULLWIDTH_f) ||
          (c >= SYS_UNICHAR_FULLWIDTH_A && c <= SYS_UNICHAR_FULLWIDTH_F) ||
          (TYPE (c) == SYS_UNICODE_DECIMAL_NUMBER));
}

/**
 * sys_unichar_isdefined:
 * @c: a Unicode character
 * 
 * Determines if a given character is assigned in the Unicode
 * standard.
 *
 * Returns: %true if the character has an assigned value
 **/
SysBool
sys_unichar_isdefined (SysUniChar c)
{
  return !IS (TYPE(c),
              OR (SYS_UNICODE_UNASSIGNED,
              OR (SYS_UNICODE_SURROGATE,
             0)));
}

/**
 * sys_unichar_iszerowidth:
 * @c: a Unicode character
 * 
 * Determines if a given character typically takes zero width when rendered.
 * The return value is %true for all non-spacing and enclosing marks
 * (e.g., combining accents), format characters, zero-width
 * space, but not U+00AD SOFT HYPHEN.
 *
 * A typical use of this function is with one of sys_unichar_iswide() or
 * sys_unichar_iswide_cjk() to determine the number of cells a string occupies
 * when displayed on a grid display (terminals).  However, note that not all
 * terminals support zero-width rendering of zero-width marks.
 *
 * Returns: %true if the character has zero width
 *
 * Since: 2.14
 **/
SysBool
sys_unichar_iszerowidth (SysUniChar c)
{
  if (SYS_UNLIKELY (c == 0x00AD))
    return false;

  if (SYS_UNLIKELY (ISZEROWIDTHTYPE (TYPE (c))))
    return true;

  /* A few additional codepoints are zero-width:
   *  - Part of the Hangul Jamo block covering medial/vowels/jungseong and
   *    final/trailinsys_consonants/jongseong Jamo
   *  - Jungseong and jongseong for Old Korean
   *  - Zero-width space (U+200B)
   */
  if (SYS_UNLIKELY ((c >= 0x1160 && c < 0x1200) ||
                  (c >= 0xD7B0 && c < 0xD800) ||
                  c == 0x200B))
    return true;

  return false;
}

static int
interval_compare (const void *key, const void *elt)
{
  SysUniChar c = POINTER_TO_UINT (key);
  struct Interval *interval = (struct Interval *)elt;

  if (c < interval->start)
    return -1;
  if (c > interval->end)
    return +1;

  return 0;
}

#define SYS_WIDTH_TABLE_MIDPOINT (ARRAY_SIZE (sys_unicode_width_table_wide) / 2)

static inline SysBool
sys_unichar_iswide_bsearch (SysUniChar ch)
{
  int lower = 0;
  int upper = ARRAY_SIZE (sys_unicode_width_table_wide) - 1;
  static int saved_mid = SYS_WIDTH_TABLE_MIDPOINT;
  int mid = saved_mid;

  do
  {
    if (ch < sys_unicode_width_table_wide[mid].start)
      upper = mid - 1;
    else if (ch > sys_unicode_width_table_wide[mid].end)
      lower = mid + 1;
    else
      return true;

    mid = (lower + upper) / 2;
  }
  while (lower <= upper);

  return false;
}

static const struct Interval default_wide_blocks[] = {
  { 0x3400, 0x4dbf },
  { 0x4e00, 0x9fff },
  { 0xf900, 0xfaff },
  { 0x20000, 0x2fffd },
  { 0x30000, 0x3fffd }
};

/**
 * sys_unichar_iswide:
 * @c: a Unicode character
 * 
 * Determines if a character is typically rendered in a double-width
 * cell.
 * 
 * Returns: %true if the character is wide
 **/
SysBool
sys_unichar_iswide (SysUniChar c)
{
  if (c < sys_unicode_width_table_wide[0].start)
    return false;
  else if (sys_unichar_iswide_bsearch (c))
    return true;
  else if (sys_unichar_type (c) == SYS_UNICODE_UNASSIGNED &&
           bsearch (UINT_TO_POINTER (c),
                    default_wide_blocks,
                    ARRAY_SIZE (default_wide_blocks),
                    sizeof default_wide_blocks[0],
                    interval_compare))
    return true;

  return false;
}


/**
 * sys_unichar_iswide_cjk:
 * @c: a Unicode character
 * 
 * Determines if a character is typically rendered in a double-width
 * cell under legacy East Asian locales.  If a character is wide according to
 * sys_unichar_iswide(), then it is also reported wide with this function, but
 * the converse is not necessarily true. See the
 * [Unicode Standard Annex #11](http://www.unicode.org/reports/tr11/)
 * for details.
 *
 * If a character passes the sys_unichar_iswide() test then it will also pass
 * this test, but not the other way around.  Note that some characters may
 * pass both this test and sys_unichar_iszerowidth().
 * 
 * Returns: %true if the character is wide in legacy East Asian locales
 *
 * Since: 2.12
 */
SysBool
sys_unichar_iswide_cjk (SysUniChar c)
{
  if (sys_unichar_iswide (c))
    return true;

  /* bsearch() is declared attribute(nonnull(1)) so we can't validly search
   * for a NULL key */
  if (c == 0)
    return false;

  if (bsearch (UINT_TO_POINTER (c), 
               sys_unicode_width_table_ambiguous,
               ARRAY_SIZE (sys_unicode_width_table_ambiguous),
               sizeof sys_unicode_width_table_ambiguous[0],
               interval_compare))
    return true;

  return false;
}


/**
 * sys_unichar_toupper:
 * @c: a Unicode character
 * 
 * Converts a character to uppercase.
 * 
 * Returns: the result of converting @c to uppercase.
 *               If @c is not a lowercase or titlecase character,
 *               or has no upper case equivalent @c is returned unchanged.
 **/
SysUniChar
sys_unichar_toupper (SysUniChar c)
{
  int t = TYPE (c);
  if (t == SYS_UNICODE_LOWERCASE_LETTER)
    {
      SysUniChar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
        {
          const SysChar *p = special_case_table + val - 0x1000000;
          val = sys_utf8_get_char (p);
        }
      /* Some lowercase letters, e.g., U+000AA, FEMININE ORDINAL INDICATOR,
       * do not have an uppercase equivalent, in which case val will be
       * zero. 
       */
      return val ? val : c;
    }
  else if (t == SYS_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < ARRAY_SIZE (title_table); ++i)
        {
          if (title_table[i][0] == c)
            return title_table[i][1] ? title_table[i][1] : c;
        }
    }
  return c;
}

/**
 * sys_unichar_tolower:
 * @c: a Unicode character.
 * 
 * Converts a character to lower case.
 * 
 * Returns: the result of converting @c to lower case.
 *               If @c is not an upperlower or titlecase character,
 *               or has no lowercase equivalent @c is returned unchanged.
 **/
SysUniChar
sys_unichar_tolower (SysUniChar c)
{
  int t = TYPE (c);
  if (t == SYS_UNICODE_UPPERCASE_LETTER)
    {
      SysUniChar val = ATTTABLE (c >> 8, c & 0xff);
      if (val >= 0x1000000)
        {
          const SysChar *p = special_case_table + val - 0x1000000;
          return sys_utf8_get_char (p);
        }
      else
        {
          /* Not all uppercase letters are guaranteed to have a lowercase
           * equivalent.  If this is the case, val will be zero. */
          return val ? val : c;
        }
    }
  else if (t == SYS_UNICODE_TITLECASE_LETTER)
    {
      unsigned int i;
      for (i = 0; i < ARRAY_SIZE (title_table); ++i)
        {
          if (title_table[i][0] == c)
            return title_table[i][2];
        }
    }
  return c;
}

/**
 * sys_unichar_totitle:
 * @c: a Unicode character
 * 
 * Converts a character to the titlecase.
 * 
 * Returns: the result of converting @c to titlecase.
 *               If @c is not an uppercase or lowercase character,
 *               @c is returned unchanged.
 **/
SysUniChar
sys_unichar_totitle (SysUniChar c)
{
  unsigned int i;

  /* We handle U+0000 explicitly because some elements in
   * title_table[i][1] may be null. */
  if (c == 0)
    return c;

  for (i = 0; i < ARRAY_SIZE (title_table); ++i)
    {
      if (title_table[i][0] == c || title_table[i][1] == c
          || title_table[i][2] == c)
        return title_table[i][0];
    }

  if (TYPE (c) == SYS_UNICODE_LOWERCASE_LETTER)
    return sys_unichar_toupper (c);

  return c;
}

/**
 * sys_unichar_digit_value:
 * @c: a Unicode character
 *
 * Determines the numeric value of a character as a decimal
 * digit.
 *
 * Returns: If @c is a decimal digit (according to
 * sys_unichar_isdigit()), its numeric value. Otherwise, -1.
 **/
int
sys_unichar_digit_value (SysUniChar c)
{
  if (TYPE (c) == SYS_UNICODE_DECIMAL_NUMBER)
    return ATTTABLE (c >> 8, c & 0xff);
  return -1;
}

/**
 * sys_unichar_xdigit_value:
 * @c: a Unicode character
 *
 * Determines the numeric value of a character as a hexadecimal
 * digit.
 *
 * Returns: If @c is a hex digit (according to
 * sys_unichar_isxdigit()), its numeric value. Otherwise, -1.
 **/
int
sys_unichar_xdigit_value (SysUniChar c)
{
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= SYS_UNICHAR_FULLWIDTH_A && c <= SYS_UNICHAR_FULLWIDTH_F)
    return c - SYS_UNICHAR_FULLWIDTH_A + 10;
  if (c >= SYS_UNICHAR_FULLWIDTH_a && c <= SYS_UNICHAR_FULLWIDTH_f)
    return c - SYS_UNICHAR_FULLWIDTH_a + 10;
  if (TYPE (c) == SYS_UNICODE_DECIMAL_NUMBER)
    return ATTTABLE (c >> 8, c & 0xff);
  return -1;
}

/**
 * sys_unichar_type:
 * @c: a Unicode character
 * 
 * Classifies a Unicode character by type.
 * 
 * Returns: the type of the character.
 **/
SysUnicodeType
sys_unichar_type (SysUniChar c)
{
  return TYPE (c);
}

/*
 * Case mapping functions
 */

LocaleType sys_unicode_get_locale_type (void)
{
#ifdef SYS_OS_WIN32
  char *tem = sys_win32_getlocale ();
  char locale[2];

  locale[0] = tem[0];
  locale[1] = tem[1];
  sys_free (tem);
#else
  const char *locale = setlocale (LC_CTYPE, NULL);

  if (locale == NULL)
    return LOCALE_NORMAL;
#endif

  switch (locale[0])
    {
   case 'a':
      if (locale[1] == 'z')
        return LOCALE_TURKIC;
      break;
    case 'l':
      if (locale[1] == 't')
        return LOCALE_LITHUANIAN;
      break;
    case 't':
      if (locale[1] == 'r')
        return LOCALE_TURKIC;
      break;
    }

  return LOCALE_NORMAL;
}

static SysInt
output_marks (const char **p_inout,
              char        *out_buffer,
              SysBool     remove_dot)
{
  const char *p = *p_inout;
  SysInt len = 0;
  
  while (*p)
    {
      SysUniChar c = sys_utf8_get_char (p);

      if (ISMARK (TYPE (c)))
        {
          if (!remove_dot || c != 0x307 /* COMBINING DOT ABOVE */)
            len += sys_unichar_to_utf8 (c, out_buffer ? out_buffer + len : NULL);
          p = sys_utf8_next_char (p);
        }
      else
        break;
    }

  *p_inout = p;
  return len;
}

static SysInt
output_special_case (SysChar *out_buffer,
                     int    offset,
                     int    type,
                     int    which)
{
  const SysChar *p = special_case_table + offset;
  SysInt len;

  if (type != SYS_UNICODE_TITLECASE_LETTER)
    p = sys_utf8_next_char (p);

  if (which == 1)
    p += strlen (p) + 1;

  len = strlen (p);
  if (out_buffer)
    memcpy (out_buffer, p, len);

  return len;
}

SysSize sys_unicode_real_toupper (const SysChar *str,
              SysSize       max_len,
              SysChar       *out_buffer,
              LocaleType   locale_type)
{
  const SysChar *p = str;
  const char *last = NULL;
  SysSize len = 0;
  SysBool last_was_i = false;

  while ((max_len < 0 || p < str + max_len) && *p)
    {
      SysUniChar c = sys_utf8_get_char (p);
      int t = TYPE (c);
      SysUniChar val;

      last = p;
      p = sys_utf8_next_char (p);

      if (locale_type == LOCALE_LITHUANIAN)
        {
          if (c == 'i')
            last_was_i = true;
          else 
            {
              if (last_was_i)
                {
                  /* Nasty, need to remove any dot above. Though
                   * I think only E WITH DOT ABOVE occurs in practice
                   * which could simplify this considerably.
                   */
                  SysSize decomp_len, i;
                  SysUniChar decomp[SYS_UNICHAR_MAX_DECOMPOSITION_LENGTH];

                  decomp_len = sys_unichar_fully_decompose (c, false, decomp, ARRAY_SIZE (decomp));
                  for (i=0; i < decomp_len; i++)
                    {
                      if (decomp[i] != 0x307 /* COMBINING DOT ABOVE */)
                        len += sys_unichar_to_utf8 (sys_unichar_toupper (decomp[i]), out_buffer ? out_buffer + len : NULL);
                    }

                  len += output_marks (&p, out_buffer ? out_buffer + len : NULL, true);

                  continue;
                }

              if (!ISMARK (t))
                last_was_i = false;
            }
        }

      if (locale_type == LOCALE_TURKIC && c == 'i')
        {
          /* i => LATIN CAPITAL LETTER I WITH DOT ABOVE */
          len += sys_unichar_to_utf8 (0x130, out_buffer ? out_buffer + len : NULL); 
        }
      else if (c == 0x0345)        /* COMBINING GREEK YPOGEGRAMMENI */
        {
          /* Nasty, need to move it after other combining marks .. this would go away if
           * we normalized first.
           */
          len += output_marks (&p, out_buffer ? out_buffer + len : NULL, false);

          /* And output as GREEK CAPITAL LETTER IOTA */
          len += sys_unichar_to_utf8 (0x399, out_buffer ? out_buffer + len : NULL);           
        }
      else if (IS (t,
                   OR (SYS_UNICODE_LOWERCASE_LETTER,
                   OR (SYS_UNICODE_TITLECASE_LETTER,
                  0))))
        {
          val = ATTTABLE (c >> 8, c & 0xff);

          if (val >= 0x1000000)
            {
              len += output_special_case (out_buffer ? out_buffer + len : NULL, val - 0x1000000, t,
                                          t == SYS_UNICODE_LOWERCASE_LETTER ? 0 : 1);
            }
          else
            {
              if (t == SYS_UNICODE_TITLECASE_LETTER)
                {
                  unsigned int i;
                  for (i = 0; i < ARRAY_SIZE (title_table); ++i)
                    {
                      if (title_table[i][0] == c)
                        {
                          val = title_table[i][1];
                          break;
                        }
                    }
                }

              /* Some lowercase letters, e.g., U+000AA, FEMININE ORDINAL INDICATOR,
               * do not have an uppercase equivalent, in which case val will be
               * zero. */
              len += sys_unichar_to_utf8 (val ? val : c, out_buffer ? out_buffer + len : NULL);
            }
        }
      else
        {
          SysSize char_len = sys_utf8_skip[*(SysUChar *)last];

          if (out_buffer)
            memcpy (out_buffer + len, last, char_len);

          len += char_len;
        }

    }

  return len;
}

/* traverses the string checking for characters with combining class == 230
 * until a base character is found */
static SysBool
has_more_above (const SysChar *str)
{
  const SysChar *p = str;
  SysInt combininsys_class;

  while (*p)
    {
      combininsys_class = sys_unichar_combining_class (sys_utf8_get_char (p));
      if (combininsys_class == 230)
        return true;
      else if (combininsys_class == 0)
        break;

      p = sys_utf8_next_char (p);
    }

  return false;
}

SysSize sys_unicode_real_tolower (const SysChar *str,
              SysSize       max_len,
              SysChar       *out_buffer,
              LocaleType   locale_type)
{
  const SysChar *p = str;
  const char *last = NULL;
  SysSize len = 0;

  while ((max_len < 0 || p < str + max_len) && *p)
    {
      SysUniChar c = sys_utf8_get_char (p);
      int t = TYPE (c);
      SysUniChar val;

      last = p;
      p = sys_utf8_next_char (p);

      if (locale_type == LOCALE_TURKIC && (c == 'I' || c == 0x130 ||
                                           c == SYS_UNICHAR_FULLWIDTH_I))
        {
          SysBool combininsys_dot = (c == 'I' || c == SYS_UNICHAR_FULLWIDTH_I) &&
                                   sys_utf8_get_char (p) == 0x0307;
          if (combininsys_dot || c == 0x130)
            {
              /* I + COMBINING DOT ABOVE => i (U+0069)
               * LATIN CAPITAL LETTER I WITH DOT ABOVE => i (U+0069) */
              len += sys_unichar_to_utf8 (0x0069, out_buffer ? out_buffer + len : NULL);
              if (combininsys_dot)
                p = sys_utf8_next_char (p);
            }
          else
            {
              /* I => LATIN SMALL LETTER DOTLESS I */
              len += sys_unichar_to_utf8 (0x131, out_buffer ? out_buffer + len : NULL); 
            }
        }
      /* Introduce an explicit dot above when lowercasing capital I's and J's
       * whenever there are more accents above. [SpecialCasing.txt] */
      else if (locale_type == LOCALE_LITHUANIAN && 
               (c == 0x00cc || c == 0x00cd || c == 0x0128))
        {
          len += sys_unichar_to_utf8 (0x0069, out_buffer ? out_buffer + len : NULL); 
          len += sys_unichar_to_utf8 (0x0307, out_buffer ? out_buffer + len : NULL); 

          switch (c)
            {
            case 0x00cc: 
              len += sys_unichar_to_utf8 (0x0300, out_buffer ? out_buffer + len : NULL); 
              break;
            case 0x00cd: 
              len += sys_unichar_to_utf8 (0x0301, out_buffer ? out_buffer + len : NULL); 
              break;
            case 0x0128: 
              len += sys_unichar_to_utf8 (0x0303, out_buffer ? out_buffer + len : NULL); 
              break;
            }
        }
      else if (locale_type == LOCALE_LITHUANIAN && 
               (c == 'I' || c == SYS_UNICHAR_FULLWIDTH_I ||
                c == 'J' || c == SYS_UNICHAR_FULLWIDTH_J || c == 0x012e) &&
               has_more_above (p))
        {
          len += sys_unichar_to_utf8 (sys_unichar_tolower (c), out_buffer ? out_buffer + len : NULL); 
          len += sys_unichar_to_utf8 (0x0307, out_buffer ? out_buffer + len : NULL); 
        }
      else if (c == 0x03A3)        /* GREEK CAPITAL LETTER SIGMA */
        {
          if ((max_len < 0 || p < str + max_len) && *p)
            {
              SysUniChar next_c = sys_utf8_get_char (p);
              int next_type = TYPE(next_c);

              /* SIGMA mapps differently depending on whether it is
               * final or not. The following simplified test would
               * fail in the case of combining marks following the
               * sigma, but I don't think that occurs in real text.
               * The test here matches that in ICU.
               */
              if (ISALPHA (next_type)) /* Lu,Ll,Lt,Lm,Lo */
                val = 0x3c3;        /* GREEK SMALL SIGMA */
              else
                val = 0x3c2;        /* GREEK SMALL FINAL SIGMA */
            }
          else
            val = 0x3c2;        /* GREEK SMALL FINAL SIGMA */

          len += sys_unichar_to_utf8 (val, out_buffer ? out_buffer + len : NULL);
        }
      else if (IS (t,
                   OR (SYS_UNICODE_UPPERCASE_LETTER,
                   OR (SYS_UNICODE_TITLECASE_LETTER,
                  0))))
        {
          val = ATTTABLE (c >> 8, c & 0xff);

          if (val >= 0x1000000)
            {
              len += output_special_case (out_buffer ? out_buffer + len : NULL, val - 0x1000000, t, 0);
            }
          else
            {
              if (t == SYS_UNICODE_TITLECASE_LETTER)
                {
                  unsigned int i;
                  for (i = 0; i < ARRAY_SIZE (title_table); ++i)
                    {
                      if (title_table[i][0] == c)
                        {
                          val = title_table[i][2];
                          break;
                        }
                    }
                }

              /* Not all uppercase letters are guaranteed to have a lowercase
               * equivalent.  If this is the case, val will be zero. */
              len += sys_unichar_to_utf8 (val ? val : c, out_buffer ? out_buffer + len : NULL);
            }
        }
      else
        {
          SysSize char_len = sys_utf8_skip[*(SysUChar *)last];

          if (out_buffer)
            memcpy (out_buffer + len, last, char_len);

          len += char_len;
        }

    }

  return len;
}


/**
 * sys_unichar_get_mirror_char:
 * @ch: a Unicode character
 * @mirrored_ch: location to store the mirrored character
 * 
 * In Unicode, some characters are "mirrored". This means that their
 * images are mirrored horizontally in text that is laid out from right
 * to left. For instance, "(" would become its mirror image, ")", in
 * right-to-left text.
 *
 * If @ch has the Unicode mirrored property and there is another unicode
 * character that typically has a glyph that is the mirror image of @ch's
 * glyph and @mirrored_ch is set, it puts that character in the address
 * pointed to by @mirrored_ch.  Otherwise the original character is put.
 *
 * Returns: %true if @ch has a mirrored character, %false otherwise
 *
 * Since: 2.4
 **/
SysBool
sys_unichar_get_mirror_char (SysUniChar ch,
                           SysUniChar *mirrored_ch)
{
  SysBool found;
  SysUniChar mirrored;

  mirrored = SYS_GET_MIRRORING(ch);

  found = ch != mirrored;
  if (mirrored_ch)
    *mirrored_ch = mirrored;

  return found;

}

#define SYS_SCRIPT_TABLE_MIDPOINT (ARRAY_SIZE (sys_script_table) / 2)

static inline SysUnicodeScript
sys_unichar_get_script_bsearch (SysUniChar ch)
{
  int lower = 0;
  int upper = ARRAY_SIZE (sys_script_table) - 1;
  static int saved_mid = SYS_SCRIPT_TABLE_MIDPOINT;
  int mid = saved_mid;

  do 
    {
      if (ch < sys_script_table[mid].start)
        upper = mid - 1;
      else if (ch >= sys_script_table[mid].start + sys_script_table[mid].chars)
        lower = mid + 1;
      else
        return sys_script_table[saved_mid = mid].script;

      mid = (lower + upper) / 2;
    }
  while (lower <= upper);

  return SYS_UNICODE_SCRIPT_UNKNOWN;
}

/**
 * sys_unichar_get_script:
 * @ch: a Unicode character
 * 
 * Looks up the #SysUnicodeScript for a particular character (as defined 
 * by Unicode Standard Annex \#24). No check is made for @ch being a
 * valid Unicode character; if you pass in invalid character, the
 * result is undefined.
 *
 * This function is equivalent to pango_script_for_unichar() and the
 * two are interchangeable.
 * 
 * Returns: the #SysUnicodeScript for the character.
 *
 * Since: 2.14
 */
SysUnicodeScript
sys_unichar_get_script (SysUniChar ch)
{
  if (ch < SYS_EASY_SCRIPTS_RANGE)
    return sys_script_easy_table[ch];
  else 
    return sys_unichar_get_script_bsearch (ch); 
}


/* http://unicode.org/iso15924/ */
static const SysUInt32 iso15924_tags[] =
{
#define PACK(a,b,c,d) ((SysUInt32)((((SysUInt8)(a))<<24)|(((SysUInt8)(b))<<16)|(((SysUInt8)(c))<<8)|((SysUInt8)(d))))

    PACK ('Z','y','y','y'), /* SYS_UNICODE_SCRIPT_COMMON */
    PACK ('Z','i','n','h'), /* SYS_UNICODE_SCRIPT_INHERITED */
    PACK ('A','r','a','b'), /* SYS_UNICODE_SCRIPT_ARABIC */
    PACK ('A','r','m','n'), /* SYS_UNICODE_SCRIPT_ARMENIAN */
    PACK ('B','e','n','g'), /* SYS_UNICODE_SCRIPT_BENGALI */
    PACK ('B','o','p','o'), /* SYS_UNICODE_SCRIPT_BOPOMOFO */
    PACK ('C','h','e','r'), /* SYS_UNICODE_SCRIPT_CHEROKEE */
    PACK ('C','o','p','t'), /* SYS_UNICODE_SCRIPT_COPTIC */
    PACK ('C','y','r','l'), /* SYS_UNICODE_SCRIPT_CYRILLIC */
    PACK ('D','s','r','t'), /* SYS_UNICODE_SCRIPT_DESERET */
    PACK ('D','e','v','a'), /* SYS_UNICODE_SCRIPT_DEVANAGARI */
    PACK ('E','t','h','i'), /* SYS_UNICODE_SCRIPT_ETHIOPIC */
    PACK ('G','e','o','r'), /* SYS_UNICODE_SCRIPT_GEORGIAN */
    PACK ('G','o','t','h'), /* SYS_UNICODE_SCRIPT_GOTHIC */
    PACK ('G','r','e','k'), /* SYS_UNICODE_SCRIPT_GREEK */
    PACK ('G','u','j','r'), /* SYS_UNICODE_SCRIPT_GUJARATI */
    PACK ('G','u','r','u'), /* SYS_UNICODE_SCRIPT_GURMUKHI */
    PACK ('H','a','n','i'), /* SYS_UNICODE_SCRIPT_HAN */
    PACK ('H','a','n','g'), /* SYS_UNICODE_SCRIPT_HANGUL */
    PACK ('H','e','b','r'), /* SYS_UNICODE_SCRIPT_HEBREW */
    PACK ('H','i','r','a'), /* SYS_UNICODE_SCRIPT_HIRAGANA */
    PACK ('K','n','d','a'), /* SYS_UNICODE_SCRIPT_KANNADA */
    PACK ('K','a','n','a'), /* SYS_UNICODE_SCRIPT_KATAKANA */
    PACK ('K','h','m','r'), /* SYS_UNICODE_SCRIPT_KHMER */
    PACK ('L','a','o','o'), /* SYS_UNICODE_SCRIPT_LAO */
    PACK ('L','a','t','n'), /* SYS_UNICODE_SCRIPT_LATIN */
    PACK ('M','l','y','m'), /* SYS_UNICODE_SCRIPT_MALAYALAM */
    PACK ('M','o','n','g'), /* SYS_UNICODE_SCRIPT_MONGOLIAN */
    PACK ('M','y','m','r'), /* SYS_UNICODE_SCRIPT_MYANMAR */
    PACK ('O','g','a','m'), /* SYS_UNICODE_SCRIPT_OGHAM */
    PACK ('I','t','a','l'), /* SYS_UNICODE_SCRIPT_OLD_ITALIC */
    PACK ('O','r','y','a'), /* SYS_UNICODE_SCRIPT_ORIYA */
    PACK ('R','u','n','r'), /* SYS_UNICODE_SCRIPT_RUNIC */
    PACK ('S','i','n','h'), /* SYS_UNICODE_SCRIPT_SINHALA */
    PACK ('S','y','r','c'), /* SYS_UNICODE_SCRIPT_SYRIAC */
    PACK ('T','a','m','l'), /* SYS_UNICODE_SCRIPT_TAMIL */
    PACK ('T','e','l','u'), /* SYS_UNICODE_SCRIPT_TELUGU */
    PACK ('T','h','a','a'), /* SYS_UNICODE_SCRIPT_THAANA */
    PACK ('T','h','a','i'), /* SYS_UNICODE_SCRIPT_THAI */
    PACK ('T','i','b','t'), /* SYS_UNICODE_SCRIPT_TIBETAN */
    PACK ('C','a','n','s'), /* SYS_UNICODE_SCRIPT_CANADIAN_ABORIGINAL */
    PACK ('Y','i','i','i'), /* SYS_UNICODE_SCRIPT_YI */
    PACK ('T','g','l','g'), /* SYS_UNICODE_SCRIPT_TAGALOG */
    PACK ('H','a','n','o'), /* SYS_UNICODE_SCRIPT_HANUNOO */
    PACK ('B','u','h','d'), /* SYS_UNICODE_SCRIPT_BUHID */
    PACK ('T','a','g','b'), /* SYS_UNICODE_SCRIPT_TAGBANWA */

  /* Unicode-4.0 additions */
    PACK ('B','r','a','i'), /* SYS_UNICODE_SCRIPT_BRAILLE */
    PACK ('C','p','r','t'), /* SYS_UNICODE_SCRIPT_CYPRIOT */
    PACK ('L','i','m','b'), /* SYS_UNICODE_SCRIPT_LIMBU */
    PACK ('O','s','m','a'), /* SYS_UNICODE_SCRIPT_OSMANYA */
    PACK ('S','h','a','w'), /* SYS_UNICODE_SCRIPT_SHAVIAN */
    PACK ('L','i','n','b'), /* SYS_UNICODE_SCRIPT_LINEAR_B */
    PACK ('T','a','l','e'), /* SYS_UNICODE_SCRIPT_TAI_LE */
    PACK ('U','g','a','r'), /* SYS_UNICODE_SCRIPT_UGARITIC */

  /* Unicode-4.1 additions */
    PACK ('T','a','l','u'), /* SYS_UNICODE_SCRIPT_NEW_TAI_LUE */
    PACK ('B','u','g','i'), /* SYS_UNICODE_SCRIPT_BUGINESE */
    PACK ('G','l','a','g'), /* SYS_UNICODE_SCRIPT_GLAGOLITIC */
    PACK ('T','f','n','g'), /* SYS_UNICODE_SCRIPT_TIFINAGH */
    PACK ('S','y','l','o'), /* SYS_UNICODE_SCRIPT_SYLOTI_NAGRI */
    PACK ('X','p','e','o'), /* SYS_UNICODE_SCRIPT_OLD_PERSIAN */
    PACK ('K','h','a','r'), /* SYS_UNICODE_SCRIPT_KHAROSHTHI */

  /* Unicode-5.0 additions */
    PACK ('Z','z','z','z'), /* SYS_UNICODE_SCRIPT_UNKNOWN */
    PACK ('B','a','l','i'), /* SYS_UNICODE_SCRIPT_BALINESE */
    PACK ('X','s','u','x'), /* SYS_UNICODE_SCRIPT_CUNEIFORM */
    PACK ('P','h','n','x'), /* SYS_UNICODE_SCRIPT_PHOENICIAN */
    PACK ('P','h','a','g'), /* SYS_UNICODE_SCRIPT_PHAGS_PA */
    PACK ('N','k','o','o'), /* SYS_UNICODE_SCRIPT_NKO */

  /* Unicode-5.1 additions */
    PACK ('K','a','l','i'), /* SYS_UNICODE_SCRIPT_KAYAH_LI */
    PACK ('L','e','p','c'), /* SYS_UNICODE_SCRIPT_LEPCHA */
    PACK ('R','j','n','g'), /* SYS_UNICODE_SCRIPT_REJANG */
    PACK ('S','u','n','d'), /* SYS_UNICODE_SCRIPT_SUNDANESE */
    PACK ('S','a','u','r'), /* SYS_UNICODE_SCRIPT_SAURASHTRA */
    PACK ('C','h','a','m'), /* SYS_UNICODE_SCRIPT_CHAM */
    PACK ('O','l','c','k'), /* SYS_UNICODE_SCRIPT_OL_CHIKI */
    PACK ('V','a','i','i'), /* SYS_UNICODE_SCRIPT_VAI */
    PACK ('C','a','r','i'), /* SYS_UNICODE_SCRIPT_CARIAN */
    PACK ('L','y','c','i'), /* SYS_UNICODE_SCRIPT_LYCIAN */
    PACK ('L','y','d','i'), /* SYS_UNICODE_SCRIPT_LYDIAN */

  /* Unicode-5.2 additions */
    PACK ('A','v','s','t'), /* SYS_UNICODE_SCRIPT_AVESTAN */
    PACK ('B','a','m','u'), /* SYS_UNICODE_SCRIPT_BAMUM */
    PACK ('E','g','y','p'), /* SYS_UNICODE_SCRIPT_EGYPTIAN_HIEROGLYPHS */
    PACK ('A','r','m','i'), /* SYS_UNICODE_SCRIPT_IMPERIAL_ARAMAIC */
    PACK ('P','h','l','i'), /* SYS_UNICODE_SCRIPT_INSCRIPTIONAL_PAHLAVI */
    PACK ('P','r','t','i'), /* SYS_UNICODE_SCRIPT_INSCRIPTIONAL_PARTHIAN */
    PACK ('J','a','v','a'), /* SYS_UNICODE_SCRIPT_JAVANESE */
    PACK ('K','t','h','i'), /* SYS_UNICODE_SCRIPT_KAITHI */
    PACK ('L','i','s','u'), /* SYS_UNICODE_SCRIPT_LISU */
    PACK ('M','t','e','i'), /* SYS_UNICODE_SCRIPT_MEETEI_MAYEK */
    PACK ('S','a','r','b'), /* SYS_UNICODE_SCRIPT_OLD_SOUTH_ARABIAN */
    PACK ('O','r','k','h'), /* SYS_UNICODE_SCRIPT_OLD_TURKIC */
    PACK ('S','a','m','r'), /* SYS_UNICODE_SCRIPT_SAMARITAN */
    PACK ('L','a','n','a'), /* SYS_UNICODE_SCRIPT_TAI_THAM */
    PACK ('T','a','v','t'), /* SYS_UNICODE_SCRIPT_TAI_VIET */

  /* Unicode-6.0 additions */
    PACK ('B','a','t','k'), /* SYS_UNICODE_SCRIPT_BATAK */
    PACK ('B','r','a','h'), /* SYS_UNICODE_SCRIPT_BRAHMI */
    PACK ('M','a','n','d'), /* SYS_UNICODE_SCRIPT_MANDAIC */

  /* Unicode-6.1 additions */
    PACK ('C','a','k','m'), /* SYS_UNICODE_SCRIPT_CHAKMA */
    PACK ('M','e','r','c'), /* SYS_UNICODE_SCRIPT_MEROITIC_CURSIVE */
    PACK ('M','e','r','o'), /* SYS_UNICODE_SCRIPT_MEROITIC_HIEROGLYPHS */
    PACK ('P','l','r','d'), /* SYS_UNICODE_SCRIPT_MIAO */
    PACK ('S','h','r','d'), /* SYS_UNICODE_SCRIPT_SHARADA */
    PACK ('S','o','r','a'), /* SYS_UNICODE_SCRIPT_SORA_SOMPENG */
    PACK ('T','a','k','r'), /* SYS_UNICODE_SCRIPT_TAKRI */

  /* Unicode 7.0 additions */
    PACK ('B','a','s','s'), /* SYS_UNICODE_SCRIPT_BASSA_VAH */
    PACK ('A','g','h','b'), /* SYS_UNICODE_SCRIPT_CAUCASIAN_ALBANIAN */
    PACK ('D','u','p','l'), /* SYS_UNICODE_SCRIPT_DUPLOYAN */
    PACK ('E','l','b','a'), /* SYS_UNICODE_SCRIPT_ELBASAN */
    PACK ('G','r','a','n'), /* SYS_UNICODE_SCRIPT_GRANTHA */
    PACK ('K','h','o','j'), /* SYS_UNICODE_SCRIPT_KHOJKI*/
    PACK ('S','i','n','d'), /* SYS_UNICODE_SCRIPT_KHUDAWADI */
    PACK ('L','i','n','a'), /* SYS_UNICODE_SCRIPT_LINEAR_A */
    PACK ('M','a','h','j'), /* SYS_UNICODE_SCRIPT_MAHAJANI */
    PACK ('M','a','n','i'), /* SYS_UNICODE_SCRIPT_MANICHAEAN */
    PACK ('M','e','n','d'), /* SYS_UNICODE_SCRIPT_MENDE_KIKAKUI */
    PACK ('M','o','d','i'), /* SYS_UNICODE_SCRIPT_MODI */
    PACK ('M','r','o','o'), /* SYS_UNICODE_SCRIPT_MRO */
    PACK ('N','b','a','t'), /* SYS_UNICODE_SCRIPT_NABATAEAN */
    PACK ('N','a','r','b'), /* SYS_UNICODE_SCRIPT_OLD_NORTH_ARABIAN */
    PACK ('P','e','r','m'), /* SYS_UNICODE_SCRIPT_OLD_PERMIC */
    PACK ('H','m','n','g'), /* SYS_UNICODE_SCRIPT_PAHAWH_HMONG */
    PACK ('P','a','l','m'), /* SYS_UNICODE_SCRIPT_PALMYRENE */
    PACK ('P','a','u','c'), /* SYS_UNICODE_SCRIPT_PAU_CIN_HAU */
    PACK ('P','h','l','p'), /* SYS_UNICODE_SCRIPT_PSALTER_PAHLAVI */
    PACK ('S','i','d','d'), /* SYS_UNICODE_SCRIPT_SIDDHAM */
    PACK ('T','i','r','h'), /* SYS_UNICODE_SCRIPT_TIRHUTA */
    PACK ('W','a','r','a'), /* SYS_UNICODE_SCRIPT_WARANG_CITI */

  /* Unicode 8.0 additions */
    PACK ('A','h','o','m'), /* SYS_UNICODE_SCRIPT_AHOM */
    PACK ('H','l','u','w'), /* SYS_UNICODE_SCRIPT_ANATOLIAN_HIEROGLYPHS */
    PACK ('H','a','t','r'), /* SYS_UNICODE_SCRIPT_HATRAN */
    PACK ('M','u','l','t'), /* SYS_UNICODE_SCRIPT_MULTANI */
    PACK ('H','u','n','g'), /* SYS_UNICODE_SCRIPT_OLD_HUNGARIAN */
    PACK ('S','g','n','w'), /* SYS_UNICODE_SCRIPT_SIGNWRITING */

  /* Unicode 9.0 additions */
    PACK ('A','d','l','m'), /* SYS_UNICODE_SCRIPT_ADLAM */
    PACK ('B','h','k','s'), /* SYS_UNICODE_SCRIPT_BHAIKSUKI */
    PACK ('M','a','r','c'), /* SYS_UNICODE_SCRIPT_MARCHEN */
    PACK ('N','e','w','a'), /* SYS_UNICODE_SCRIPT_NEWA */
    PACK ('O','s','g','e'), /* SYS_UNICODE_SCRIPT_OSAGE */
    PACK ('T','a','n','g'), /* SYS_UNICODE_SCRIPT_TANGUT */

  /* Unicode 10.0 additions */
    PACK ('G','o','n','m'), /* SYS_UNICODE_SCRIPT_MASARAM_GONDI */
    PACK ('N','s','h','u'), /* SYS_UNICODE_SCRIPT_NUSHU */
    PACK ('S','o','y','o'), /* SYS_UNICODE_SCRIPT_SOYOMBO */
    PACK ('Z','a','n','b'), /* SYS_UNICODE_SCRIPT_ZANABAZAR_SQUARE */

  /* Unicode 11.0 additions */
    PACK ('D','o','g','r'), /* SYS_UNICODE_SCRIPT_DOGRA */
    PACK ('G','o','n','g'), /* SYS_UNICODE_SCRIPT_GUNJALA_GONDI */
    PACK ('R','o','h','g'), /* SYS_UNICODE_SCRIPT_HANIFI_ROHINGYA */
    PACK ('M','a','k','a'), /* SYS_UNICODE_SCRIPT_MAKASAR */
    PACK ('M','e','d','f'), /* SYS_UNICODE_SCRIPT_MEDEFAIDRIN */
    PACK ('S','o','g','o'), /* SYS_UNICODE_SCRIPT_OLD_SOGDIAN */
    PACK ('S','o','g','d'), /* SYS_UNICODE_SCRIPT_SOGDIAN */

  /* Unicode 12.0 additions */
    PACK ('E','l','y','m'), /* SYS_UNICODE_SCRIPT_ELYMAIC */
    PACK ('N','a','n','d'), /* SYS_UNICODE_SCRIPT_NANDINAGARI */
    PACK ('H','m','n','p'), /* SYS_UNICODE_SCRIPT_NYIAKENG_PUACHUE_HMONG */
    PACK ('W','c','h','o'), /* SYS_UNICODE_SCRIPT_WANCHO */

  /* Unicode 13.0 additions */
    PACK ('C', 'h', 'r', 's'), /* SYS_UNICODE_SCRIPT_CHORASMIAN */
    PACK ('D', 'i', 'a', 'k'), /* SYS_UNICODE_SCRIPT_DIVES_AKURU */
    PACK ('K', 'i', 't', 's'), /* SYS_UNICODE_SCRIPT_KHITAN_SMALL_SCRIPT */
    PACK ('Y', 'e', 'z', 'i'), /* SYS_UNICODE_SCRIPT_YEZIDI */

  /* Unicode 14.0 additions */
    PACK ('C', 'p', 'm', 'n'), /* SYS_UNICODE_SCRIPT_CYPRO_MINOAN */
    PACK ('O', 'u', 'g', 'r'), /* SYS_UNICODE_SCRIPT_OLD_UYHUR */
    PACK ('T', 'n', 's', 'a'), /* SYS_UNICODE_SCRIPT_TANGSA */
    PACK ('T', 'o', 't', 'o'), /* SYS_UNICODE_SCRIPT_TOTO */
    PACK ('V', 'i', 't', 'h'), /* SYS_UNICODE_SCRIPT_VITHKUQI */

  /* not really a Unicode script, but part of ISO 15924 */
    PACK ('Z', 'm', 't', 'h'), /* SYS_UNICODE_SCRIPT_MATH */

    /* Unicode 15.0 additions */
    PACK ('K', 'a', 'w', 'i'), /* SYS_UNICODE_SCRIPT_KAWI */
    PACK ('N', 'a', 'g', 'm'), /* SYS_UNICODE_SCRIPT_NAG_MUNDARI */

#undef PACK
};

/**
 * sys_unicode_script_to_iso15924:
 * @script: a Unicode script
 *
 * Looks up the ISO 15924 code for @script.  ISO 15924 assigns four-letter
 * codes to scripts.  For example, the code for Arabic is 'Arab'.  The
 * four letter codes are encoded as a @SysUInt32 by this function in a
 * big-endian fashion.  That is, the code returned for Arabic is
 * 0x41726162 (0x41 is ASCII code for 'A', 0x72 is ASCII code for 'r', etc).
 *
 * See
 * [Codes for the representation of names of scripts](http://unicode.org/iso15924/codelists.html)
 * for details.
 *
 * Returns: the ISO 15924 code for @script, encoded as an integer,
 *   of zero if @script is %SYS_UNICODE_SCRIPT_INVALID_CODE or
 *   ISO 15924 code 'Zzzz' (script code for UNKNOWN) if @script is not understood.
 *
 * Since: 2.30
 */
SysUInt32
sys_unicode_script_to_iso15924 (SysUnicodeScript script)
{
  if (SYS_UNLIKELY (script == SYS_UNICODE_SCRIPT_INVALID_CODE))
    return 0;

  if (SYS_UNLIKELY (script < 0 || script >= (int) ARRAY_SIZE (iso15924_tags)))
    return 0x5A7A7A7A;

  return iso15924_tags[script];
}

/**
 * sys_unicode_script_from_iso15924:
 * @iso15924: a Unicode script
 *
 * Looks up the Unicode script for @iso15924.  ISO 15924 assigns four-letter
 * codes to scripts.  For example, the code for Arabic is 'Arab'.
 * This function accepts four letter codes encoded as a @SysUInt32 in a
 * big-endian fashion.  That is, the code expected for Arabic is
 * 0x41726162 (0x41 is ASCII code for 'A', 0x72 is ASCII code for 'r', etc).
 *
 * See
 * [Codes for the representation of names of scripts](http://unicode.org/iso15924/codelists.html)
 * for details.
 *
 * Returns: the Unicode script for @iso15924, or
 *   of %SYS_UNICODE_SCRIPT_INVALID_CODE if @iso15924 is zero and
 *   %SYS_UNICODE_SCRIPT_UNKNOWN if @iso15924 is unknown.
 *
 * Since: 2.30
 */
SysUnicodeScript
sys_unicode_script_from_iso15924 (SysUInt32 iso15924)
{
  unsigned int i;

   if (!iso15924)
     return SYS_UNICODE_SCRIPT_INVALID_CODE;

  for (i = 0; i < ARRAY_SIZE (iso15924_tags); i++)
    if (iso15924_tags[i] == iso15924)
      return (SysUnicodeScript) i;

  return SYS_UNICODE_SCRIPT_UNKNOWN;
}
