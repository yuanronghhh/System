#include <System/DataTypes/SysQuark.h>
#include <System/Utils/SysStr.h>
#include <System/Platform/Common/SysThread.h>

#define QUARK_BLOCK_SIZE         2048
#define QUARK_STRING_BLOCK_SIZE (4096 - sizeof (SysSize))

static inline SysQuark  quark_new (SysChar *string);
SYS_LOCK_DEFINE_STATIC (quark_global);
static SysHashTable    *quark_ht = NULL;
static SysChar        **quarks = NULL;
static SysInt           quark_seq_id = 0;
static SysChar         *quark_block = NULL;
static SysInt           quark_block_offset = 0;

/**
 * SECTION:quarks
 * @title: Quarks
 * @short_description: a 2-way association between a string and a
 *     unique integer identifier
 *
 * Quarks are associations between strings and integer identifiers.
 * Given either the string or the #SysQuark identifier it is possible to
 * retrieve the other.
 *
 * Quarks are used for both [datasets][glib-Datasets] and
 * [keyed data lists][glib-Keyed-Data-Lists].
 *
 * To create a new quark from a string, use sys_quark_from_string() or
 * sys_quark_from_static_string().
 *
 * To find the string corresponding to a given #SysQuark, use
 * sys_quark_to_string().
 *
 * To find the #SysQuark corresponding to a given string, use
 * sys_quark_try_string().
 *
 * Another use for the string pool maintained for the quark functions
 * is string interning, using sys_intern_string() or
 * sys_intern_static_string(). An interned string is a canonical
 * representation for a string. One important advantage of interned
 * strings is that they can be compared for equality by a simple
 * pointer comparison, rather than using strcmp().
 */

/**
 * SysQuark:
 *
 * A SysQuark is a non-zero integer which uniquely identifies a
 * particular string. A SysQuark value of zero is associated to %NULL.
 */

/**
 * SYS_DEFINE_QUARK:
 * @QN: the name to return a #SysQuark for
 * @q_n: prefix for the function name
 *
 * A convenience macro which defines a function returning the
 * #SysQuark for the name @QN. The function will be named
 * @q_n_quark().
 *
 * Note that the quark name will be stringified automatically
 * in the macro, so you shouldn't use double quotes.
 *
 * Since: 2.34
 */

/**
 * sys_quark_try_string:
 * @string: (nullable): a string
 *
 * Gets the #SysQuark associated with the given string, or 0 if string is
 * %NULL or it has no associated #SysQuark.
 *
 * If you want the SysQuark to be created if it doesn't already exist,
 * use sys_quark_from_string() or sys_quark_from_static_string().
 *
 * This function must not be used before library constructors have finished
 * running.
 *
 * Returns: the #SysQuark associated with the string, or 0 if @string is
 *     %NULL or there is no #SysQuark associated with it
 */
SysQuark
sys_quark_try_string (const SysChar *string)
{
  SysQuark quark = 0;

  if (string == NULL)
    return 0;

  SYS_LOCK (quark_global);
  quark = POINTER_TO_UINT (sys_hash_table_lookup (quark_ht, (SysPointer)string));
  SYS_UNLOCK (quark_global);

  return quark;
}

/* HOLDS: quark_global_lock */
static char *
quark_strdup (const SysChar *string)
{
  SysChar *copy;
  SysSize len;

  len = strlen (string) + 1;

  /* For strings longer than half the block size, fall back
     to strdup so that we fill our blocks at least 50%. */
  if (len > QUARK_STRING_BLOCK_SIZE / 2)
    return sys_strdup (string);

  if (quark_block == NULL ||
      QUARK_STRING_BLOCK_SIZE - quark_block_offset < len)
    {
      quark_block = sys_malloc (QUARK_STRING_BLOCK_SIZE);
      quark_block_offset = 0;
    }

  copy = quark_block + quark_block_offset;
  memcpy (copy, string, len);
  quark_block_offset += len;

  return copy;
}

/* HOLDS: quark_global_lock */
static inline SysQuark
quark_from_string (const SysChar *string,
                   SysBool     duplicate)
{
  SysQuark quark = 0;

  quark = POINTER_TO_UINT (sys_hash_table_lookup (quark_ht, (SysPointer)string));

  if (!quark)
    {
      quark = quark_new (duplicate ? quark_strdup (string) : (SysChar *)string);
    }

  return quark;
}

static inline SysQuark
quark_from_string_locked (const SysChar   *string,
                          SysBool       duplicate)
{
  SysQuark quark = 0;

  if (!string)
    return 0;

  SYS_LOCK (quark_global);
  quark = quark_from_string (string, duplicate);
  SYS_UNLOCK (quark_global);

  return quark;
}

/**
 * sys_quark_from_string:
 * @string: (nullable): a string
 *
 * Gets the #SysQuark identifying the given string. If the string does
 * not currently have an associated #SysQuark, a new #SysQuark is created,
 * using a copy of the string.
 *
 * This function must not be used before library constructors have finished
 * running. In particular, this means it cannot be used to initialize global
 * variables in C++.
 *
 * Returns: the #SysQuark identifying the string, or 0 if @string is %NULL
 */
SysQuark
sys_quark_from_string (const SysChar *string)
{
  return quark_from_string_locked (string, true);
}

/**
 * sys_quark_from_static_string:
 * @string: (nullable): a string
 *
 * Gets the #SysQuark identifying the given (static) string. If the
 * string does not currently have an associated #SysQuark, a new #SysQuark
 * is created, linked to the given string.
 *
 * Note that this function is identical to sys_quark_from_string() except
 * that if a new #SysQuark is created the string itself is used rather
 * than a copy. This saves memory, but can only be used if the string
 * will continue to exist until the program terminates. It can be used
 * with statically allocated strings in the main program, but not with
 * statically allocated memory in dynamically loaded modules, if you
 * expect to ever unload the module again (e.g. do not use this
 * function in GTK+ theme engines).
 *
 * This function must not be used before library constructors have finished
 * running. In particular, this means it cannot be used to initialize global
 * variables in C++.
 *
 * Returns: the #SysQuark identifying the string, or 0 if @string is %NULL
 */
SysQuark
sys_quark_from_static_string (const SysChar *string)
{
  return quark_from_string_locked (string, false);
}

/**
 * sys_quark_to_string:
 * @quark: a #SysQuark.
 *
 * Gets the string associated with the given #SysQuark.
 *
 * Returns: the string associated with the #SysQuark
 */
const SysChar *
sys_quark_to_string (SysQuark quark)
{
  SysChar* result = NULL;
  SysChar **strings;
  SysUInt seq_id;

  seq_id = (SysUInt) sys_atomic_int_get (&quark_seq_id);
  strings = sys_atomic_pointer_get (&quarks);

  if (quark < seq_id)
    result = strings[quark];

  return result;
}

/* HOLDS: sys_quark_global_lock */
static inline SysQuark
quark_new (SysChar *string)
{
  SysQuark quark;
  SysChar **quarks_new;

  if (quark_seq_id % QUARK_BLOCK_SIZE == 0)
    {
      quarks_new = sys_new (SysChar*, quark_seq_id + QUARK_BLOCK_SIZE);
      if (quark_seq_id != 0)
        memcpy (quarks_new, quarks, sizeof (char *) * quark_seq_id);
      memset (quarks_new + quark_seq_id, 0, sizeof (char *) * QUARK_BLOCK_SIZE);
      /* This leaks the old quarks array. Its unfortunate, but it allows
       * us to do lockless lookup of the arrays, and there shouldn't be that
       * many quarks in an app
       */
      sys_atomic_pointer_set (&quarks, quarks_new);
    }

  quark = quark_seq_id;
  sys_atomic_pointer_set (&quarks[quark], string);
  sys_hash_table_insert (quark_ht, string, UINT_TO_POINTER (quark));
  sys_atomic_int_inc (&quark_seq_id);

  return quark;
}

static inline const SysChar *
quark_intern_string_locked (const SysChar   *string,
                            SysBool       duplicate)
{
  const SysChar *result;
  SysQuark quark;

  if (!string)
    return NULL;

  SYS_LOCK (quark_global);
  quark = quark_from_string (string, duplicate);
  result = quarks[quark];
  SYS_UNLOCK (quark_global);

  return result;
}

/**
 * sys_intern_string:
 * @string: (nullable): a string
 *
 * Returns a canonical representation for @string. Interned strings
 * can be compared for equality by comparing the pointers, instead of
 * using strcmp().
 *
 * This function must not be used before library constructors have finished
 * running. In particular, this means it cannot be used to initialize global
 * variables in C++.
 *
 * Returns: a canonical representation for the string
 *
 * Since: 2.10
 */
const SysChar * sys_intern_string (const SysChar *string)
{
  return quark_intern_string_locked (string, true);
}

/**
 * sys_intern_static_string:
 * @string: (nullable): a static string
 *
 * Returns a canonical representation for @string. Interned strings
 * can be compared for equality by comparing the pointers, instead of
 * using strcmp(). sys_intern_static_string() does not copy the string,
 * therefore @string must not be freed or modified.
 *
 * This function must not be used before library constructors have finished
 * running. In particular, this means it cannot be used to initialize global
 * variables in C++.
 *
 * Returns: a canonical representation for the string
 *
 * Since: 2.10
 */
const SysChar * sys_intern_static_string (const SysChar *string)
{
  return quark_intern_string_locked (string, false);
}


void sys_quark_setup(void) {
  sys_assert (quark_seq_id == 0);
  quark_ht = sys_hash_table_new (sys_str_hash, (SysEqualFunc)sys_str_equal);
  quarks = sys_new (SysChar*, QUARK_BLOCK_SIZE);
  quarks[0] = NULL;
  quark_seq_id = 1;
}

void sys_quark_teardown(void) {
  sys_free(quarks);
  sys_hash_table_unref(quark_ht);
}
