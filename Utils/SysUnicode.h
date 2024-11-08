#ifndef __SYS_UNICODE_H__
#define __SYS_UNICODE_H__

#include <System/Fundamental/SysCommonCore.h>

SYS_BEGIN_DECLS

/**
 * SysUnicodeType:
 * @SYS_UNICODE_CONTROL: General category "Other, Control" (Cc)
 * @SYS_UNICODE_FORMAT: General category "Other, Format" (Cf)
 * @SYS_UNICODE_UNASSIGNED: General category "Other, Not Assigned" (Cn)
 * @SYS_UNICODE_PRIVATE_USE: General category "Other, Private Use" (Co)
 * @SYS_UNICODE_SURROGATE: General category "Other, Surrogate" (Cs)
 * @SYS_UNICODE_LOWERCASE_LETTER: General category "Letter, Lowercase" (Ll)
 * @SYS_UNICODE_MODIFIER_LETTER: General category "Letter, Modifier" (Lm)
 * @SYS_UNICODE_OTHER_LETTER: General category "Letter, Other" (Lo)
 * @SYS_UNICODE_TITLECASE_LETTER: General category "Letter, Titlecase" (Lt)
 * @SYS_UNICODE_UPPERCASE_LETTER: General category "Letter, Uppercase" (Lu)
 * @SYS_UNICODE_SPACING_MARK: General category "Mark, Spacing" (Mc)
 * @SYS_UNICODE_ENCLOSING_MARK: General category "Mark, Enclosing" (Me)
 * @SYS_UNICODE_NON_SPACING_MARK: General category "Mark, Nonspacing" (Mn)
 * @SYS_UNICODE_DECIMAL_NUMBER: General category "Number, Decimal Digit" (Nd)
 * @SYS_UNICODE_LETTER_NUMBER: General category "Number, Letter" (Nl)
 * @SYS_UNICODE_OTHER_NUMBER: General category "Number, Other" (No)
 * @SYS_UNICODE_CONNECT_PUNCTUATION: General category "Punctuation, Connector" (Pc)
 * @SYS_UNICODE_DASH_PUNCTUATION: General category "Punctuation, Dash" (Pd)
 * @SYS_UNICODE_CLOSE_PUNCTUATION: General category "Punctuation, Close" (Pe)
 * @SYS_UNICODE_FINAL_PUNCTUATION: General category "Punctuation, Final quote" (Pf)
 * @SYS_UNICODE_INITIAL_PUNCTUATION: General category "Punctuation, Initial quote" (Pi)
 * @SYS_UNICODE_OTHER_PUNCTUATION: General category "Punctuation, Other" (Po)
 * @SYS_UNICODE_OPEN_PUNCTUATION: General category "Punctuation, Open" (Ps)
 * @SYS_UNICODE_CURRENCY_SYMBOL: General category "Symbol, Currency" (Sc)
 * @SYS_UNICODE_MODIFIER_SYMBOL: General category "Symbol, Modifier" (Sk)
 * @SYS_UNICODE_MATH_SYMBOL: General category "Symbol, Math" (Sm)
 * @SYS_UNICODE_OTHER_SYMBOL: General category "Symbol, Other" (So)
 * @SYS_UNICODE_LINE_SEPARATOR: General category "Separator, Line" (Zl)
 * @SYS_UNICODE_PARAGRAPH_SEPARATOR: General category "Separator, Paragraph" (Zp)
 * @SYS_UNICODE_SPACE_SEPARATOR: General category "Separator, Space" (Zs)
 *
 * These are the possible character classifications from the
 * Unicode specification.
 * See [Unicode Character Database](http://www.unicode.org/reports/tr44/#General_Category_Values).
 */
typedef enum
{
  SYS_UNICODE_CONTROL,
  SYS_UNICODE_FORMAT,
  SYS_UNICODE_UNASSIGNED,
  SYS_UNICODE_PRIVATE_USE,
  SYS_UNICODE_SURROGATE,
  SYS_UNICODE_LOWERCASE_LETTER,
  SYS_UNICODE_MODIFIER_LETTER,
  SYS_UNICODE_OTHER_LETTER,
  SYS_UNICODE_TITLECASE_LETTER,
  SYS_UNICODE_UPPERCASE_LETTER,
  SYS_UNICODE_SPACING_MARK,
  SYS_UNICODE_ENCLOSING_MARK,
  SYS_UNICODE_NON_SPACING_MARK,
  SYS_UNICODE_DECIMAL_NUMBER,
  SYS_UNICODE_LETTER_NUMBER,
  SYS_UNICODE_OTHER_NUMBER,
  SYS_UNICODE_CONNECT_PUNCTUATION,
  SYS_UNICODE_DASH_PUNCTUATION,
  SYS_UNICODE_CLOSE_PUNCTUATION,
  SYS_UNICODE_FINAL_PUNCTUATION,
  SYS_UNICODE_INITIAL_PUNCTUATION,
  SYS_UNICODE_OTHER_PUNCTUATION,
  SYS_UNICODE_OPEN_PUNCTUATION,
  SYS_UNICODE_CURRENCY_SYMBOL,
  SYS_UNICODE_MODIFIER_SYMBOL,
  SYS_UNICODE_MATH_SYMBOL,
  SYS_UNICODE_OTHER_SYMBOL,
  SYS_UNICODE_LINE_SEPARATOR,
  SYS_UNICODE_PARAGRAPH_SEPARATOR,
  SYS_UNICODE_SPACE_SEPARATOR
} SysUnicodeType;

/**
 * SysUnicodeBreakType:
 * @SYS_UNICODE_BREAK_MANDATORY: Mandatory Break (BK)
 * @SYS_UNICODE_BREAK_CARRIAGE_RETURN: Carriage Return (CR)
 * @SYS_UNICODE_BREAK_LINE_FEED: Line Feed (LF)
 * @SYS_UNICODE_BREAK_COMBINING_MARK: Attached Characters and Combining Marks (CM)
 * @SYS_UNICODE_BREAK_SURROGATE: Surrogates (SG)
 * @SYS_UNICODE_BREAK_ZERO_WIDTH_SPACE: Zero Width Space (ZW)
 * @SYS_UNICODE_BREAK_INSEPARABLE: Inseparable (IN)
 * @SYS_UNICODE_BREAK_NON_BREAKING_GLUE: Non-breaking ("Glue") (GL)
 * @SYS_UNICODE_BREAK_CONTINGENT: Contingent Break Opportunity (CB)
 * @SYS_UNICODE_BREAK_SPACE: Space (SP)
 * @SYS_UNICODE_BREAK_AFTER: Break Opportunity After (BA)
 * @SYS_UNICODE_BREAK_BEFORE: Break Opportunity Before (BB)
 * @SYS_UNICODE_BREAK_BEFORE_AND_AFTER: Break Opportunity Before and After (B2)
 * @SYS_UNICODE_BREAK_HYPHEN: Hyphen (HY)
 * @SYS_UNICODE_BREAK_NON_STARTER: Nonstarter (NS)
 * @SYS_UNICODE_BREAK_OPEN_PUNCTUATION: Opening Punctuation (OP)
 * @SYS_UNICODE_BREAK_CLOSE_PUNCTUATION: Closing Punctuation (CL)
 * @SYS_UNICODE_BREAK_QUOTATION: Ambiguous Quotation (QU)
 * @SYS_UNICODE_BREAK_EXCLAMATION: Exclamation/Interrogation (EX)
 * @SYS_UNICODE_BREAK_IDEOGRAPHIC: Ideographic (ID)
 * @SYS_UNICODE_BREAK_NUMERIC: Numeric (NU)
 * @SYS_UNICODE_BREAK_INFIX_SEPARATOR: Infix Separator (Numeric) (IS)
 * @SYS_UNICODE_BREAK_SYMBOL: Symbols Allowing Break After (SY)
 * @SYS_UNICODE_BREAK_ALPHABETIC: Ordinary Alphabetic and Symbol Characters (AL)
 * @SYS_UNICODE_BREAK_PREFIX: Prefix (Numeric) (PR)
 * @SYS_UNICODE_BREAK_POSTFIX: Postfix (Numeric) (PO)
 * @SYS_UNICODE_BREAK_COMPLEX_CONTEXT: Complex Content Dependent (South East Asian) (SA)
 * @SYS_UNICODE_BREAK_AMBIGUOUS: Ambiguous (Alphabetic or Ideographic) (AI)
 * @SYS_UNICODE_BREAK_UNKNOWN: Unknown (XX)
 * @SYS_UNICODE_BREAK_NEXT_LINE: Next Line (NL)
 * @SYS_UNICODE_BREAK_WORD_JOINER: Word Joiner (WJ)
 * @SYS_UNICODE_BREAK_HANGUL_L_JAMO: Hangul L Jamo (JL)
 * @SYS_UNICODE_BREAK_HANGUL_V_JAMO: Hangul V Jamo (JV)
 * @SYS_UNICODE_BREAK_HANGUL_T_JAMO: Hangul T Jamo (JT)
 * @SYS_UNICODE_BREAK_HANGUL_LV_SYLLABLE: Hangul LV Syllable (H2)
 * @SYS_UNICODE_BREAK_HANGUL_LVT_SYLLABLE: Hangul LVT Syllable (H3)
 * @SYS_UNICODE_BREAK_CLOSE_PARANTHESIS: Closing Parenthesis (CP). Since 2.28. Deprecated: 2.70: Use %SYS_UNICODE_BREAK_CLOSE_PARENTHESIS instead.
 * @SYS_UNICODE_BREAK_CLOSE_PARENTHESIS: Closing Parenthesis (CP). Since 2.70
 * @SYS_UNICODE_BREAK_CONDITIONAL_JAPANESE_STARTER: Conditional Japanese Starter (CJ). Since: 2.32
 * @SYS_UNICODE_BREAK_HEBREW_LETTER: Hebrew Letter (HL). Since: 2.32
 * @SYS_UNICODE_BREAK_REGIONAL_INDICATOR: Regional Indicator (RI). Since: 2.36
 * @SYS_UNICODE_BREAK_EMOJI_BASE: Emoji Base (EB). Since: 2.50
 * @SYS_UNICODE_BREAK_EMOJI_MODIFIER: Emoji Modifier (EM). Since: 2.50
 * @SYS_UNICODE_BREAK_ZERO_WIDTH_JOINER: Zero Width Joiner (ZWJ). Since: 2.50
 *
 * These are the possible line break classifications.
 *
 * Since new unicode versions may add new types here, applications should be ready 
 * to handle unknown values. They may be regarded as %SYS_UNICODE_BREAK_UNKNOWN.
 *
 * See [Unicode Line Breaking Algorithm](http://www.unicode.org/unicode/reports/tr14/).
 */
typedef enum
{
  SYS_UNICODE_BREAK_MANDATORY,
  SYS_UNICODE_BREAK_CARRIAGE_RETURN,
  SYS_UNICODE_BREAK_LINE_FEED,
  SYS_UNICODE_BREAK_COMBINING_MARK,
  SYS_UNICODE_BREAK_SURROGATE,
  SYS_UNICODE_BREAK_ZERO_WIDTH_SPACE,
  SYS_UNICODE_BREAK_INSEPARABLE,
  SYS_UNICODE_BREAK_NON_BREAKING_GLUE,
  SYS_UNICODE_BREAK_CONTINGENT,
  SYS_UNICODE_BREAK_SPACE,
  SYS_UNICODE_BREAK_AFTER,
  SYS_UNICODE_BREAK_BEFORE,
  SYS_UNICODE_BREAK_BEFORE_AND_AFTER,
  SYS_UNICODE_BREAK_HYPHEN,
  SYS_UNICODE_BREAK_NON_STARTER,
  SYS_UNICODE_BREAK_OPEN_PUNCTUATION,
  SYS_UNICODE_BREAK_CLOSE_PUNCTUATION,
  SYS_UNICODE_BREAK_QUOTATION,
  SYS_UNICODE_BREAK_EXCLAMATION,
  SYS_UNICODE_BREAK_IDEOGRAPHIC,
  SYS_UNICODE_BREAK_NUMERIC,
  SYS_UNICODE_BREAK_INFIX_SEPARATOR,
  SYS_UNICODE_BREAK_SYMBOL,
  SYS_UNICODE_BREAK_ALPHABETIC,
  SYS_UNICODE_BREAK_PREFIX,
  SYS_UNICODE_BREAK_POSTFIX,
  SYS_UNICODE_BREAK_COMPLEX_CONTEXT,
  SYS_UNICODE_BREAK_AMBIGUOUS,
  SYS_UNICODE_BREAK_UNKNOWN,
  SYS_UNICODE_BREAK_NEXT_LINE,
  SYS_UNICODE_BREAK_WORD_JOINER,
  SYS_UNICODE_BREAK_HANGUL_L_JAMO,
  SYS_UNICODE_BREAK_HANGUL_V_JAMO,
  SYS_UNICODE_BREAK_HANGUL_T_JAMO,
  SYS_UNICODE_BREAK_HANGUL_LV_SYLLABLE,
  SYS_UNICODE_BREAK_HANGUL_LVT_SYLLABLE,
  SYS_UNICODE_BREAK_CLOSE_PARANTHESIS,
  SYS_UNICODE_BREAK_CLOSE_PARENTHESIS = SYS_UNICODE_BREAK_CLOSE_PARANTHESIS,
  SYS_UNICODE_BREAK_CONDITIONAL_JAPANESE_STARTER,
  SYS_UNICODE_BREAK_HEBREW_LETTER,
  SYS_UNICODE_BREAK_REGIONAL_INDICATOR,
  SYS_UNICODE_BREAK_EMOJI_BASE,
  SYS_UNICODE_BREAK_EMOJI_MODIFIER,
  SYS_UNICODE_BREAK_ZERO_WIDTH_JOINER
} SysUnicodeBreakType;

/**
 * SysUnicodeScript:
 * @SYS_UNICODE_SCRIPT_INVALID_CODE:
 *                               a value never returned from sys_unichar_get_script()
 * @SYS_UNICODE_SCRIPT_COMMON:     a character used by multiple different scripts
 * @SYS_UNICODE_SCRIPT_INHERITED:  a mark glyph that takes its script from the
 *                               base glyph to which it is attached
 * @SYS_UNICODE_SCRIPT_ARABIC:     Arabic
 * @SYS_UNICODE_SCRIPT_ARMENIAN:   Armenian
 * @SYS_UNICODE_SCRIPT_BENGALI:    Bengali
 * @SYS_UNICODE_SCRIPT_BOPOMOFO:   Bopomofo
 * @SYS_UNICODE_SCRIPT_CHEROKEE:   Cherokee
 * @SYS_UNICODE_SCRIPT_COPTIC:     Coptic
 * @SYS_UNICODE_SCRIPT_CYRILLIC:   Cyrillic
 * @SYS_UNICODE_SCRIPT_DESERET:    Deseret
 * @SYS_UNICODE_SCRIPT_DEVANAGARI: Devanagari
 * @SYS_UNICODE_SCRIPT_ETHIOPIC:   Ethiopic
 * @SYS_UNICODE_SCRIPT_GEORGIAN:   Georgian
 * @SYS_UNICODE_SCRIPT_GOTHIC:     Gothic
 * @SYS_UNICODE_SCRIPT_GREEK:      Greek
 * @SYS_UNICODE_SCRIPT_GUJARATI:   Gujarati
 * @SYS_UNICODE_SCRIPT_GURMUKHI:   Gurmukhi
 * @SYS_UNICODE_SCRIPT_HAN:        Han
 * @SYS_UNICODE_SCRIPT_HANGUL:     Hangul
 * @SYS_UNICODE_SCRIPT_HEBREW:     Hebrew
 * @SYS_UNICODE_SCRIPT_HIRAGANA:   Hiragana
 * @SYS_UNICODE_SCRIPT_KANNADA:    Kannada
 * @SYS_UNICODE_SCRIPT_KATAKANA:   Katakana
 * @SYS_UNICODE_SCRIPT_KHMER:      Khmer
 * @SYS_UNICODE_SCRIPT_LAO:        Lao
 * @SYS_UNICODE_SCRIPT_LATIN:      Latin
 * @SYS_UNICODE_SCRIPT_MALAYALAM:  Malayalam
 * @SYS_UNICODE_SCRIPT_MONGOLIAN:  Mongolian
 * @SYS_UNICODE_SCRIPT_MYANMAR:    Myanmar
 * @SYS_UNICODE_SCRIPT_OGHAM:      Ogham
 * @SYS_UNICODE_SCRIPT_OLD_ITALIC: Old Italic
 * @SYS_UNICODE_SCRIPT_ORIYA:      Oriya
 * @SYS_UNICODE_SCRIPT_RUNIC:      Runic
 * @SYS_UNICODE_SCRIPT_SINHALA:    Sinhala
 * @SYS_UNICODE_SCRIPT_SYRIAC:     Syriac
 * @SYS_UNICODE_SCRIPT_TAMIL:      Tamil
 * @SYS_UNICODE_SCRIPT_TELUGU:     Telugu
 * @SYS_UNICODE_SCRIPT_THAANA:     Thaana
 * @SYS_UNICODE_SCRIPT_THAI:       Thai
 * @SYS_UNICODE_SCRIPT_TIBETAN:    Tibetan
 * @SYS_UNICODE_SCRIPT_CANADIAN_ABORIGINAL:
 *                               Canadian Aboriginal
 * @SYS_UNICODE_SCRIPT_YI:         Yi
 * @SYS_UNICODE_SCRIPT_TAGALOG:    Tagalog
 * @SYS_UNICODE_SCRIPT_HANUNOO:    Hanunoo
 * @SYS_UNICODE_SCRIPT_BUHID:      Buhid
 * @SYS_UNICODE_SCRIPT_TAGBANWA:   Tagbanwa
 * @SYS_UNICODE_SCRIPT_BRAILLE:    Braille
 * @SYS_UNICODE_SCRIPT_CYPRIOT:    Cypriot
 * @SYS_UNICODE_SCRIPT_LIMBU:      Limbu
 * @SYS_UNICODE_SCRIPT_OSMANYA:    Osmanya
 * @SYS_UNICODE_SCRIPT_SHAVIAN:    Shavian
 * @SYS_UNICODE_SCRIPT_LINEAR_B:   Linear B
 * @SYS_UNICODE_SCRIPT_TAI_LE:     Tai Le
 * @SYS_UNICODE_SCRIPT_UGARITIC:   Ugaritic
 * @SYS_UNICODE_SCRIPT_NEW_TAI_LUE:
 *                               New Tai Lue
 * @SYS_UNICODE_SCRIPT_BUGINESE:   Buginese
 * @SYS_UNICODE_SCRIPT_GLAGOLITIC: Glagolitic
 * @SYS_UNICODE_SCRIPT_TIFINAGH:   Tifinagh
 * @SYS_UNICODE_SCRIPT_SYLOTI_NAGRI:
 *                               Syloti Nagri
 * @SYS_UNICODE_SCRIPT_OLD_PERSIAN:
 *                               Old Persian
 * @SYS_UNICODE_SCRIPT_KHAROSHTHI: Kharoshthi
 * @SYS_UNICODE_SCRIPT_UNKNOWN:    an unassigned code point
 * @SYS_UNICODE_SCRIPT_BALINESE:   Balinese
 * @SYS_UNICODE_SCRIPT_CUNEIFORM:  Cuneiform
 * @SYS_UNICODE_SCRIPT_PHOENICIAN: Phoenician
 * @SYS_UNICODE_SCRIPT_PHAGS_PA:   Phags-pa
 * @SYS_UNICODE_SCRIPT_NKO:        N'Ko
 * @SYS_UNICODE_SCRIPT_KAYAH_LI:   Kayah Li. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_LEPCHA:     Lepcha. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_REJANG:     Rejang. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_SUNDANESE:  Sundanese. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_SAURASHTRA: Saurashtra. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_CHAM:       Cham. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_OL_CHIKI:   Ol Chiki. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_VAI:        Vai. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_CARIAN:     Carian. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_LYCIAN:     Lycian. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_LYDIAN:     Lydian. Since 2.16.3
 * @SYS_UNICODE_SCRIPT_AVESTAN:    Avestan. Since 2.26
 * @SYS_UNICODE_SCRIPT_BAMUM:      Bamum. Since 2.26
 * @SYS_UNICODE_SCRIPT_EGYPTIAN_HIEROGLYPHS:
 *                               Egyptian Hieroglpyhs. Since 2.26
 * @SYS_UNICODE_SCRIPT_IMPERIAL_ARAMAIC:
 *                               Imperial Aramaic. Since 2.26
 * @SYS_UNICODE_SCRIPT_INSCRIPTIONAL_PAHLAVI:
 *                               Inscriptional Pahlavi. Since 2.26
 * @SYS_UNICODE_SCRIPT_INSCRIPTIONAL_PARTHIAN:
 *                               Inscriptional Parthian. Since 2.26
 * @SYS_UNICODE_SCRIPT_JAVANESE:   Javanese. Since 2.26
 * @SYS_UNICODE_SCRIPT_KAITHI:     Kaithi. Since 2.26
 * @SYS_UNICODE_SCRIPT_LISU:       Lisu. Since 2.26
 * @SYS_UNICODE_SCRIPT_MEETEI_MAYEK:
 *                               Meetei Mayek. Since 2.26
 * @SYS_UNICODE_SCRIPT_OLD_SOUTH_ARABIAN:
 *                               Old South Arabian. Since 2.26
 * @SYS_UNICODE_SCRIPT_OLD_TURKIC: Old Turkic. Since 2.28
 * @SYS_UNICODE_SCRIPT_SAMARITAN:  Samaritan. Since 2.26
 * @SYS_UNICODE_SCRIPT_TAI_THAM:   Tai Tham. Since 2.26
 * @SYS_UNICODE_SCRIPT_TAI_VIET:   Tai Viet. Since 2.26
 * @SYS_UNICODE_SCRIPT_BATAK:      Batak. Since 2.28
 * @SYS_UNICODE_SCRIPT_BRAHMI:     Brahmi. Since 2.28
 * @SYS_UNICODE_SCRIPT_MANDAIC:    Mandaic. Since 2.28
 * @SYS_UNICODE_SCRIPT_CHAKMA:               Chakma. Since: 2.32
 * @SYS_UNICODE_SCRIPT_MEROITIC_CURSIVE:     Meroitic Cursive. Since: 2.32
 * @SYS_UNICODE_SCRIPT_MEROITIC_HIEROGLYPHS: Meroitic Hieroglyphs. Since: 2.32
 * @SYS_UNICODE_SCRIPT_MIAO:                 Miao. Since: 2.32
 * @SYS_UNICODE_SCRIPT_SHARADA:              Sharada. Since: 2.32
 * @SYS_UNICODE_SCRIPT_SORA_SOMPENG:         Sora Sompeng. Since: 2.32
 * @SYS_UNICODE_SCRIPT_TAKRI:                Takri. Since: 2.32
 * @SYS_UNICODE_SCRIPT_BASSA_VAH:            Bassa. Since: 2.42
 * @SYS_UNICODE_SCRIPT_CAUCASIAN_ALBANIAN:   Caucasian Albanian. Since: 2.42
 * @SYS_UNICODE_SCRIPT_DUPLOYAN:             Duployan. Since: 2.42
 * @SYS_UNICODE_SCRIPT_ELBASAN:              Elbasan. Since: 2.42
 * @SYS_UNICODE_SCRIPT_GRANTHA:              Grantha. Since: 2.42
 * @SYS_UNICODE_SCRIPT_KHOJKI:               Kjohki. Since: 2.42
 * @SYS_UNICODE_SCRIPT_KHUDAWADI:            Khudawadi, Sindhi. Since: 2.42
 * @SYS_UNICODE_SCRIPT_LINEAR_A:             Linear A. Since: 2.42
 * @SYS_UNICODE_SCRIPT_MAHAJANI:             Mahajani. Since: 2.42
 * @SYS_UNICODE_SCRIPT_MANICHAEAN:           Manichaean. Since: 2.42
 * @SYS_UNICODE_SCRIPT_MENDE_KIKAKUI:        Mende Kikakui. Since: 2.42
 * @SYS_UNICODE_SCRIPT_MODI:                 Modi. Since: 2.42
 * @SYS_UNICODE_SCRIPT_MRO:                  Mro. Since: 2.42
 * @SYS_UNICODE_SCRIPT_NABATAEAN:            Nabataean. Since: 2.42
 * @SYS_UNICODE_SCRIPT_OLD_NORTH_ARABIAN:    Old North Arabian. Since: 2.42
 * @SYS_UNICODE_SCRIPT_OLD_PERMIC:           Old Permic. Since: 2.42
 * @SYS_UNICODE_SCRIPT_PAHAWH_HMONG:         Pahawh Hmong. Since: 2.42
 * @SYS_UNICODE_SCRIPT_PALMYRENE:            Palmyrene. Since: 2.42
 * @SYS_UNICODE_SCRIPT_PAU_CIN_HAU:          Pau Cin Hau. Since: 2.42
 * @SYS_UNICODE_SCRIPT_PSALTER_PAHLAVI:      Psalter Pahlavi. Since: 2.42
 * @SYS_UNICODE_SCRIPT_SIDDHAM:              Siddham. Since: 2.42
 * @SYS_UNICODE_SCRIPT_TIRHUTA:              Tirhuta. Since: 2.42
 * @SYS_UNICODE_SCRIPT_WARANG_CITI:          Warang Citi. Since: 2.42
 * @SYS_UNICODE_SCRIPT_AHOM:                 Ahom. Since: 2.48
 * @SYS_UNICODE_SCRIPT_ANATOLIAN_HIEROGLYPHS: Anatolian Hieroglyphs. Since: 2.48
 * @SYS_UNICODE_SCRIPT_HATRAN:               Hatran. Since: 2.48
 * @SYS_UNICODE_SCRIPT_MULTANI:              Multani. Since: 2.48
 * @SYS_UNICODE_SCRIPT_OLD_HUNGARIAN:        Old Hungarian. Since: 2.48
 * @SYS_UNICODE_SCRIPT_SIGNWRITING:          Signwriting. Since: 2.48
 * @SYS_UNICODE_SCRIPT_ADLAM:                Adlam. Since: 2.50
 * @SYS_UNICODE_SCRIPT_BHAIKSUKI:            Bhaiksuki. Since: 2.50
 * @SYS_UNICODE_SCRIPT_MARCHEN:              Marchen. Since: 2.50
 * @SYS_UNICODE_SCRIPT_NEWA:                 Newa. Since: 2.50
 * @SYS_UNICODE_SCRIPT_OSAGE:                Osage. Since: 2.50
 * @SYS_UNICODE_SCRIPT_TANGUT:               Tangut. Since: 2.50
 * @SYS_UNICODE_SCRIPT_MASARAM_GONDI:        Masaram Gondi. Since: 2.54
 * @SYS_UNICODE_SCRIPT_NUSHU:                Nushu. Since: 2.54
 * @SYS_UNICODE_SCRIPT_SOYOMBO:              Soyombo. Since: 2.54
 * @SYS_UNICODE_SCRIPT_ZANABAZAR_SQUARE:     Zanabazar Square. Since: 2.54
 * @SYS_UNICODE_SCRIPT_DOGRA:                Dogra. Since: 2.58
 * @SYS_UNICODE_SCRIPT_GUNJALA_GONDI:        Gunjala Gondi. Since: 2.58
 * @SYS_UNICODE_SCRIPT_HANIFI_ROHINGYA:      Hanifi Rohingya. Since: 2.58
 * @SYS_UNICODE_SCRIPT_MAKASAR:              Makasar. Since: 2.58
 * @SYS_UNICODE_SCRIPT_MEDEFAIDRIN:          Medefaidrin. Since: 2.58
 * @SYS_UNICODE_SCRIPT_OLD_SOGDIAN:          Old Sogdian. Since: 2.58
 * @SYS_UNICODE_SCRIPT_SOGDIAN:              Sogdian. Since: 2.58
 * @SYS_UNICODE_SCRIPT_ELYMAIC:              Elym. Since: 2.62
 * @SYS_UNICODE_SCRIPT_NANDINAGARI:          Nand. Since: 2.62
 * @SYS_UNICODE_SCRIPT_NYIAKENG_PUACHUE_HMONG: Rohg. Since: 2.62
 * @SYS_UNICODE_SCRIPT_WANCHO:               Wcho. Since: 2.62
 * @SYS_UNICODE_SCRIPT_CHORASMIAN:           Chorasmian. Since: 2.66
 * @SYS_UNICODE_SCRIPT_DIVES_AKURU:          Dives Akuru. Since: 2.66
 * @SYS_UNICODE_SCRIPT_KHITAN_SMALL_SCRIPT:  Khitan small script. Since: 2.66
 * @SYS_UNICODE_SCRIPT_YEZIDI:               Yezidi. Since: 2.66
 * @SYS_UNICODE_SCRIPT_CYPRO_MINOAN:         Cypro-Minoan. Since: 2.72
 * @SYS_UNICODE_SCRIPT_OLD_UYGHUR:           Old Uyghur. Since: 2.72
 * @SYS_UNICODE_SCRIPT_TANGSA:               Tangsa. Since: 2.72
 * @SYS_UNICODE_SCRIPT_TOTO:                 Toto. Since: 2.72
 * @SYS_UNICODE_SCRIPT_VITHKUQI:             Vithkuqi. Since: 2.72
 * @SYS_UNICODE_SCRIPT_MATH:                 Mathematical notation. Since: 2.72
 * @SYS_UNICODE_SCRIPT_KAWI:                 Kawi. Since 2.74
 * @SYS_UNICODE_SCRIPT_NAG_MUNDARI:          Nag Mundari. Since 2.74
 *
 * The #SysUnicodeScript enumeration identifies different writing
 * systems. The values correspond to the names as defined in the
 * Unicode standard. The enumeration has been added in GLib 2.14,
 * and is interchangeable with #PangoScript.
 *
 * Note that new types may be added in the future. Applications
 * should be ready to handle unknown values.
 * See [Unicode Standard Annex #24: Script names](http://www.unicode.org/reports/tr24/).
 */
typedef enum
{                         /* ISO 15924 code */
  SYS_UNICODE_SCRIPT_INVALID_CODE = -1,
  SYS_UNICODE_SCRIPT_COMMON       = 0,   /* Zyyy */
  SYS_UNICODE_SCRIPT_INHERITED,          /* Zinh (Qaai) */
  SYS_UNICODE_SCRIPT_ARABIC,             /* Arab */
  SYS_UNICODE_SCRIPT_ARMENIAN,           /* Armn */
  SYS_UNICODE_SCRIPT_BENGALI,            /* Beng */
  SYS_UNICODE_SCRIPT_BOPOMOFO,           /* Bopo */
  SYS_UNICODE_SCRIPT_CHEROKEE,           /* Cher */
  SYS_UNICODE_SCRIPT_COPTIC,             /* Copt (Qaac) */
  SYS_UNICODE_SCRIPT_CYRILLIC,           /* Cyrl (Cyrs) */
  SYS_UNICODE_SCRIPT_DESERET,            /* Dsrt */
  SYS_UNICODE_SCRIPT_DEVANAGARI,         /* Deva */
  SYS_UNICODE_SCRIPT_ETHIOPIC,           /* Ethi */
  SYS_UNICODE_SCRIPT_GEORGIAN,           /* Geor (Geon, Geoa) */
  SYS_UNICODE_SCRIPT_GOTHIC,             /* Goth */
  SYS_UNICODE_SCRIPT_GREEK,              /* Grek */
  SYS_UNICODE_SCRIPT_GUJARATI,           /* Gujr */
  SYS_UNICODE_SCRIPT_GURMUKHI,           /* Guru */
  SYS_UNICODE_SCRIPT_HAN,                /* Hani */
  SYS_UNICODE_SCRIPT_HANGUL,             /* Hang */
  SYS_UNICODE_SCRIPT_HEBREW,             /* Hebr */
  SYS_UNICODE_SCRIPT_HIRAGANA,           /* Hira */
  SYS_UNICODE_SCRIPT_KANNADA,            /* Knda */
  SYS_UNICODE_SCRIPT_KATAKANA,           /* Kana */
  SYS_UNICODE_SCRIPT_KHMER,              /* Khmr */
  SYS_UNICODE_SCRIPT_LAO,                /* Laoo */
  SYS_UNICODE_SCRIPT_LATIN,              /* Latn (Latf, Latg) */
  SYS_UNICODE_SCRIPT_MALAYALAM,          /* Mlym */
  SYS_UNICODE_SCRIPT_MONGOLIAN,          /* Mong */
  SYS_UNICODE_SCRIPT_MYANMAR,            /* Mymr */
  SYS_UNICODE_SCRIPT_OGHAM,              /* Ogam */
  SYS_UNICODE_SCRIPT_OLD_ITALIC,         /* Ital */
  SYS_UNICODE_SCRIPT_ORIYA,              /* Orya */
  SYS_UNICODE_SCRIPT_RUNIC,              /* Runr */
  SYS_UNICODE_SCRIPT_SINHALA,            /* Sinh */
  SYS_UNICODE_SCRIPT_SYRIAC,             /* Syrc (Syrj, Syrn, Syre) */
  SYS_UNICODE_SCRIPT_TAMIL,              /* Taml */
  SYS_UNICODE_SCRIPT_TELUGU,             /* Telu */
  SYS_UNICODE_SCRIPT_THAANA,             /* Thaa */
  SYS_UNICODE_SCRIPT_THAI,               /* Thai */
  SYS_UNICODE_SCRIPT_TIBETAN,            /* Tibt */
  SYS_UNICODE_SCRIPT_CANADIAN_ABORIGINAL, /* Cans */
  SYS_UNICODE_SCRIPT_YI,                 /* Yiii */
  SYS_UNICODE_SCRIPT_TAGALOG,            /* Tglg */
  SYS_UNICODE_SCRIPT_HANUNOO,            /* Hano */
  SYS_UNICODE_SCRIPT_BUHID,              /* Buhd */
  SYS_UNICODE_SCRIPT_TAGBANWA,           /* Tagb */

  /* Unicode-4.0 additions */
  SYS_UNICODE_SCRIPT_BRAILLE,            /* Brai */
  SYS_UNICODE_SCRIPT_CYPRIOT,            /* Cprt */
  SYS_UNICODE_SCRIPT_LIMBU,              /* Limb */
  SYS_UNICODE_SCRIPT_OSMANYA,            /* Osma */
  SYS_UNICODE_SCRIPT_SHAVIAN,            /* Shaw */
  SYS_UNICODE_SCRIPT_LINEAR_B,           /* Linb */
  SYS_UNICODE_SCRIPT_TAI_LE,             /* Tale */
  SYS_UNICODE_SCRIPT_UGARITIC,           /* Ugar */

  /* Unicode-4.1 additions */
  SYS_UNICODE_SCRIPT_NEW_TAI_LUE,        /* Talu */
  SYS_UNICODE_SCRIPT_BUGINESE,           /* Bugi */
  SYS_UNICODE_SCRIPT_GLAGOLITIC,         /* Glag */
  SYS_UNICODE_SCRIPT_TIFINAGH,           /* Tfng */
  SYS_UNICODE_SCRIPT_SYLOTI_NAGRI,       /* Sylo */
  SYS_UNICODE_SCRIPT_OLD_PERSIAN,        /* Xpeo */
  SYS_UNICODE_SCRIPT_KHAROSHTHI,         /* Khar */

  /* Unicode-5.0 additions */
  SYS_UNICODE_SCRIPT_UNKNOWN,            /* Zzzz */
  SYS_UNICODE_SCRIPT_BALINESE,           /* Bali */
  SYS_UNICODE_SCRIPT_CUNEIFORM,          /* Xsux */
  SYS_UNICODE_SCRIPT_PHOENICIAN,         /* Phnx */
  SYS_UNICODE_SCRIPT_PHAGS_PA,           /* Phag */
  SYS_UNICODE_SCRIPT_NKO,                /* Nkoo */

  /* Unicode-5.1 additions */
  SYS_UNICODE_SCRIPT_KAYAH_LI,           /* Kali */
  SYS_UNICODE_SCRIPT_LEPCHA,             /* Lepc */
  SYS_UNICODE_SCRIPT_REJANG,             /* Rjng */
  SYS_UNICODE_SCRIPT_SUNDANESE,          /* Sund */
  SYS_UNICODE_SCRIPT_SAURASHTRA,         /* Saur */
  SYS_UNICODE_SCRIPT_CHAM,               /* Cham */
  SYS_UNICODE_SCRIPT_OL_CHIKI,           /* Olck */
  SYS_UNICODE_SCRIPT_VAI,                /* Vaii */
  SYS_UNICODE_SCRIPT_CARIAN,             /* Cari */
  SYS_UNICODE_SCRIPT_LYCIAN,             /* Lyci */
  SYS_UNICODE_SCRIPT_LYDIAN,             /* Lydi */

  /* Unicode-5.2 additions */
  SYS_UNICODE_SCRIPT_AVESTAN,                /* Avst */
  SYS_UNICODE_SCRIPT_BAMUM,                  /* Bamu */
  SYS_UNICODE_SCRIPT_EGYPTIAN_HIEROGLYPHS,   /* Egyp */
  SYS_UNICODE_SCRIPT_IMPERIAL_ARAMAIC,       /* Armi */
  SYS_UNICODE_SCRIPT_INSCRIPTIONAL_PAHLAVI,  /* Phli */
  SYS_UNICODE_SCRIPT_INSCRIPTIONAL_PARTHIAN, /* Prti */
  SYS_UNICODE_SCRIPT_JAVANESE,               /* Java */
  SYS_UNICODE_SCRIPT_KAITHI,                 /* Kthi */
  SYS_UNICODE_SCRIPT_LISU,                   /* Lisu */
  SYS_UNICODE_SCRIPT_MEETEI_MAYEK,           /* Mtei */
  SYS_UNICODE_SCRIPT_OLD_SOUTH_ARABIAN,      /* Sarb */
  SYS_UNICODE_SCRIPT_OLD_TURKIC,             /* Orkh */
  SYS_UNICODE_SCRIPT_SAMARITAN,              /* Samr */
  SYS_UNICODE_SCRIPT_TAI_THAM,               /* Lana */
  SYS_UNICODE_SCRIPT_TAI_VIET,               /* Tavt */

  /* Unicode-6.0 additions */
  SYS_UNICODE_SCRIPT_BATAK,                  /* Batk */
  SYS_UNICODE_SCRIPT_BRAHMI,                 /* Brah */
  SYS_UNICODE_SCRIPT_MANDAIC,                /* Mand */

  /* Unicode-6.1 additions */
  SYS_UNICODE_SCRIPT_CHAKMA,                 /* Cakm */
  SYS_UNICODE_SCRIPT_MEROITIC_CURSIVE,       /* Merc */
  SYS_UNICODE_SCRIPT_MEROITIC_HIEROGLYPHS,   /* Mero */
  SYS_UNICODE_SCRIPT_MIAO,                   /* Plrd */
  SYS_UNICODE_SCRIPT_SHARADA,                /* Shrd */
  SYS_UNICODE_SCRIPT_SORA_SOMPENG,           /* Sora */
  SYS_UNICODE_SCRIPT_TAKRI,                  /* Takr */

  /* Unicode 7.0 additions */
  SYS_UNICODE_SCRIPT_BASSA_VAH,              /* Bass */
  SYS_UNICODE_SCRIPT_CAUCASIAN_ALBANIAN,     /* Aghb */
  SYS_UNICODE_SCRIPT_DUPLOYAN,               /* Dupl */
  SYS_UNICODE_SCRIPT_ELBASAN,                /* Elba */
  SYS_UNICODE_SCRIPT_GRANTHA,                /* Gran */
  SYS_UNICODE_SCRIPT_KHOJKI,                 /* Khoj */
  SYS_UNICODE_SCRIPT_KHUDAWADI,              /* Sind */
  SYS_UNICODE_SCRIPT_LINEAR_A,               /* Lina */
  SYS_UNICODE_SCRIPT_MAHAJANI,               /* Mahj */
  SYS_UNICODE_SCRIPT_MANICHAEAN,             /* Mani */
  SYS_UNICODE_SCRIPT_MENDE_KIKAKUI,          /* Mend */
  SYS_UNICODE_SCRIPT_MODI,                   /* Modi */
  SYS_UNICODE_SCRIPT_MRO,                    /* Mroo */
  SYS_UNICODE_SCRIPT_NABATAEAN,              /* Nbat */
  SYS_UNICODE_SCRIPT_OLD_NORTH_ARABIAN,      /* Narb */
  SYS_UNICODE_SCRIPT_OLD_PERMIC,             /* Perm */
  SYS_UNICODE_SCRIPT_PAHAWH_HMONG,           /* Hmng */
  SYS_UNICODE_SCRIPT_PALMYRENE,              /* Palm */
  SYS_UNICODE_SCRIPT_PAU_CIN_HAU,            /* Pauc */
  SYS_UNICODE_SCRIPT_PSALTER_PAHLAVI,        /* Phlp */
  SYS_UNICODE_SCRIPT_SIDDHAM,                /* Sidd */
  SYS_UNICODE_SCRIPT_TIRHUTA,                /* Tirh */
  SYS_UNICODE_SCRIPT_WARANG_CITI,            /* Wara */

  /* Unicode 8.0 additions */
  SYS_UNICODE_SCRIPT_AHOM,                   /* Ahom */
  SYS_UNICODE_SCRIPT_ANATOLIAN_HIEROGLYPHS,  /* Hluw */
  SYS_UNICODE_SCRIPT_HATRAN,                 /* Hatr */
  SYS_UNICODE_SCRIPT_MULTANI,                /* Mult */
  SYS_UNICODE_SCRIPT_OLD_HUNGARIAN,          /* Hung */
  SYS_UNICODE_SCRIPT_SIGNWRITING,            /* Sgnw */

  /* Unicode 9.0 additions */
  SYS_UNICODE_SCRIPT_ADLAM,                  /* Adlm */
  SYS_UNICODE_SCRIPT_BHAIKSUKI,              /* Bhks */
  SYS_UNICODE_SCRIPT_MARCHEN,                /* Marc */
  SYS_UNICODE_SCRIPT_NEWA,                   /* Newa */
  SYS_UNICODE_SCRIPT_OSAGE,                  /* Osge */
  SYS_UNICODE_SCRIPT_TANGUT,                 /* Tang */

  /* Unicode 10.0 additions */
  SYS_UNICODE_SCRIPT_MASARAM_GONDI,          /* Gonm */
  SYS_UNICODE_SCRIPT_NUSHU,                  /* Nshu */
  SYS_UNICODE_SCRIPT_SOYOMBO,                /* Soyo */
  SYS_UNICODE_SCRIPT_ZANABAZAR_SQUARE,       /* Zanb */

  /* Unicode 11.0 additions */
  SYS_UNICODE_SCRIPT_DOGRA,                  /* Dogr */
  SYS_UNICODE_SCRIPT_GUNJALA_GONDI,          /* Gong */
  SYS_UNICODE_SCRIPT_HANIFI_ROHINGYA,        /* Rohg */
  SYS_UNICODE_SCRIPT_MAKASAR,                /* Maka */
  SYS_UNICODE_SCRIPT_MEDEFAIDRIN,            /* Medf */
  SYS_UNICODE_SCRIPT_OLD_SOGDIAN,            /* Sogo */
  SYS_UNICODE_SCRIPT_SOGDIAN,                /* Sogd */

  /* Unicode 12.0 additions */
  SYS_UNICODE_SCRIPT_ELYMAIC,                /* Elym */
  SYS_UNICODE_SCRIPT_NANDINAGARI,            /* Nand */
  SYS_UNICODE_SCRIPT_NYIAKENG_PUACHUE_HMONG, /* Rohg */
  SYS_UNICODE_SCRIPT_WANCHO,                 /* Wcho */

  /* Unicode 13.0 additions */
  SYS_UNICODE_SCRIPT_CHORASMIAN,             /* Chrs */
  SYS_UNICODE_SCRIPT_DIVES_AKURU,            /* Diak */
  SYS_UNICODE_SCRIPT_KHITAN_SMALL_SCRIPT,    /* Kits */
  SYS_UNICODE_SCRIPT_YEZIDI,                 /* Yezi */

  /* Unicode 14.0 additions */
  SYS_UNICODE_SCRIPT_CYPRO_MINOAN,           /* Cpmn */
  SYS_UNICODE_SCRIPT_OLD_UYGHUR,             /* Ougr */
  SYS_UNICODE_SCRIPT_TANGSA,                 /* Tnsa */
  SYS_UNICODE_SCRIPT_TOTO,                   /* Toto */
  SYS_UNICODE_SCRIPT_VITHKUQI,               /* Vith */

  /* not really a Unicode script, but part of ISO 15924 */
  SYS_UNICODE_SCRIPT_MATH,                   /* Zmth */

  /* Unicode 15.0 additions */
  SYS_UNICODE_SCRIPT_KAWI ,          /* Kawi */
  SYS_UNICODE_SCRIPT_NAG_MUNDARI ,   /* Nag Mundari */
} SysUnicodeScript;

typedef enum {
  LOCALE_NORMAL,
  LOCALE_TURKIC,
  LOCALE_LITHUANIAN
} LocaleType;

SysUInt32        sys_unicode_script_to_iso15924   (SysUnicodeScript script);
SysUnicodeScript sys_unicode_script_from_iso15924 (SysUInt32        iso15924);

/* These are all analogs of the <ctype.h> functions.
 */
SysBool sys_unichar_isalnum   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_isalpha   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_iscntrl   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_isdigit   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_isgraph   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_islower   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_isprint   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_ispunct   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_isspace   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_isupper   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_isxdigit  (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_istitle   (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_isdefined (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_iswide    (SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_iswide_cjk(SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_iszerowidth(SysUniChar c) SYS_GNUC_CONST;
SysBool sys_unichar_ismark    (SysUniChar c) SYS_GNUC_CONST;

/* More <ctype.h> functions.  These convert between the three cases.
 * See the Unicode book to understand title case.  */
SysUniChar sys_unichar_toupper (SysUniChar c) SYS_GNUC_CONST;
SysUniChar sys_unichar_tolower (SysUniChar c) SYS_GNUC_CONST;
SysUniChar sys_unichar_totitle (SysUniChar c) SYS_GNUC_CONST;

/* If C is a digit (according to 'g_unichar_isdigit'), then return its
   numeric value.  Otherwise return -1.  */
SysInt sys_unichar_digit_value (SysUniChar c) SYS_GNUC_CONST;

SysInt sys_unichar_xdigit_value (SysUniChar c) SYS_GNUC_CONST;

/* Return the Unicode character type of a given character.  */
SysUnicodeType sys_unichar_type (SysUniChar c) SYS_GNUC_CONST;

/* Return the line break property for a given character */
SysUnicodeBreakType sys_unichar_break_type (SysUniChar c) SYS_GNUC_CONST;

/* Returns the combining class for a given character */
SysInt sys_unichar_combining_class (SysUniChar uc) SYS_GNUC_CONST;

SysBool sys_unichar_get_mirror_char (SysUniChar ch,
                                    SysUniChar *mirrored_ch);

SysUnicodeScript sys_unichar_get_script (SysUniChar ch) SYS_GNUC_CONST;

/* Validate a Unicode character */
SysBool sys_unichar_validate (SysUniChar ch) SYS_GNUC_CONST;

/* Pairwise canonical compose/decompose */
SysBool sys_unichar_compose (SysUniChar  a,
                            SysUniChar  b,
                            SysUniChar *ch);
SysBool sys_unichar_decompose (SysUniChar  ch,
                              SysUniChar *a,
                              SysUniChar *b);

SysSSize sys_unichar_fully_decompose (SysUniChar  ch,
                                 SysBool  compat,
                                 SysUniChar *result,
                                 SysSSize     result_len);

/**
 * SYS_UNICHAR_MAX_DECOMPOSITION_LENGTH:
 *
 * The maximum length (in codepoints) of a compatibility or canonical
 * decomposition of a single Unicode character.
 *
 * This is as defined by Unicode 6.1.
 *
 * Since: 2.32
 */
#define SYS_UNICHAR_MAX_DECOMPOSITION_LENGTH 18 /* codepoints */

/* Compute canonical ordering of a string in-place.  This rearranges
   decomposed characters in the string according to their combining
   classes.  See the Unicode manual for more information.  */
void sys_unicode_canonical_ordering (SysUniChar *string,
                                   SysSSize     len);

LocaleType sys_unicode_get_locale_type (void);

SysSSize sys_unicode_real_toupper (const SysChar *str,
              SysSSize       max_len,
              SysChar       *out_buffer,
              LocaleType   locale_type);

SysSSize sys_unicode_real_tolower (const SysChar *str,
              SysSSize       max_len,
              SysChar       *out_buffer,
              LocaleType   locale_type);

SYS_END_DECLS

#endif
