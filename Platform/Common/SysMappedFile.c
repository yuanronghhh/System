#include <System/Platform/Common/SysMappedFile.h>
#include <System/Platform/Common/SysFile.h>
#include <System/DataTypes/SysBytes.h>

#ifndef _O_BINARY
#define _O_BINARY 0
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

#define sys_filename_display_name(filename) (SysChar *)filename
/**
 * SysMappedFile:
 *
 * The #SysMappedFile represents a file mapping created with
 * sys_mapped_file_new(). It has only private members and should
 * not be accessed directly.
 */

struct _SysMappedFile
{
  SysChar *contents;
  SysSize  length;
  SysPointer free_func;
  int    ref_count;
#ifdef SYS_OS_WIN32
  HANDLE mapping;
#endif
};

static void
sys_mapped_file_destroy (SysMappedFile *file)
{
  if (file->length)
    {
#ifdef HAVE_MMAP
      munmap (file->contents, file->length);
#endif
#ifdef SYS_OS_WIN32
      UnmapViewOfFile (file->contents);
      CloseHandle (file->mapping);
#endif
    }

  sys_slice_free (SysMappedFile, file);
}

static SysMappedFile*
mapped_file_new_from_fd (int           fd,
    SysBool      writable,
    const SysChar  *filename,
    SysError      **error)
{
  SysMappedFile *file;
  struct stat st;

  file = sys_slice_new0 (SysMappedFile);
  file->ref_count = 1;
  file->free_func = sys_mapped_file_destroy;

  if (fstat (fd, &st) == -1)
    {
      int save_errno = errno;
      SysChar *display_filename = filename ? sys_filename_display_name (filename) : NULL;

      sys_error_set_N (error,
          SYS_("Failed to get attributes of file “%s%s%s%s”: fstat() failed: %s"),
          display_filename ? display_filename : "fd",
          display_filename ? "' " : "",
          display_filename ? display_filename : "",
          display_filename ? "'" : "",
          sys_strerror (save_errno));
      sys_free (display_filename);
      goto out;
    }

  /* mmap() on size 0 will fail with EINVAL, so we avoid calling mmap()
   * in that case -- but only if we have a regular file; we still want
   * attempts to mmap a character device to fail, for example.
   */
  if (st.st_size == 0 && S_ISREG (st.st_mode))
  {
    file->length = 0;
    file->contents = NULL;
    return file;
  }

  file->contents = MAP_FAILED;

#ifdef HAVE_MMAP
  if (sizeof (st.st_size) > sizeof (SysSize) && st.st_size > (off_t) SYS_MAXSIZE)
  {
    errno = EINVAL;
  }
  else
  {
    file->length = (SysSize) st.st_size;
    file->contents = (SysChar *) mmap (NULL,  file->length,
        writable ? PROT_READ|PROT_WRITE : PROT_READ,
        MAP_PRIVATE, fd, 0);
  }
#endif
#ifdef SYS_OS_WIN32
  file->length = st.st_size;
  file->mapping = CreateFileMapping ((HANDLE) _get_osfhandle (fd), NULL,
      writable ? PAGE_WRITECOPY : PAGE_READONLY,
      0, 0,
      NULL);
  if (file->mapping != NULL)
  {
    file->contents = MapViewOfFile (file->mapping,
        writable ? FILE_MAP_COPY : FILE_MAP_READ,
        0, 0,
        0);
    if (file->contents == NULL)
    {
      file->contents = MAP_FAILED;
      CloseHandle (file->mapping);
      file->mapping = NULL;
    }
  }
#endif


  if (file->contents == MAP_FAILED)
  {
    int save_errno = errno;
    SysChar *display_filename = filename ? sys_filename_display_name (filename) : NULL;

    sys_error_set_N (error,
        SYS_("Failed to map %s%s%s%s: mmap() failed: %s"),
        display_filename ? display_filename : "fd",
        display_filename ? "' " : "",
        display_filename ? display_filename : "",
        display_filename ? "'" : "",
        sys_strerror (save_errno));
    sys_free (display_filename);
    goto out;
  }

  return file;

out:
  sys_slice_free (SysMappedFile, file);

  return NULL;
}

/**
 * sys_mapped_file_new:
 * @filename: (type filename): The path of the file to load, in the GLib
 *     filename encoding
 * @writable: whether the mapping should be writable
 * @error: return location for a #SysError, or %NULL
 *
 * Maps a file into memory. On UNIX, this is using the mmap() function.
 *
 * If @writable is %true, the mapped buffer may be modified, otherwise
 * it is an error to modify the mapped buffer. Modifications to the buffer
 * are not visible to other processes mapping the same file, and are not
 * written back to the file.
 *
 * Note that modifications of the underlying file might affect the contents
 * of the #SysMappedFile. Therefore, mapping should only be used if the file
 * will not be modified, or if all modifications of the file are done
 * atomically (e.g. using sys_file_set_contents()).
 *
 * If @filename is the name of an empty, regular file, the function
 * will successfully return an empty #SysMappedFile. In other cases of
 * size 0 (e.g. device files such as /dev/null), @error will be set
 * to the #GFileError value %G_FILE_ERROR_INVAL.
 *
 * Returns: a newly allocated #SysMappedFile which must be unref'd
 *    with sys_mapped_file_unref(), or %NULL if the mapping failed.
 *
 * Since: 2.8
 */
  SysMappedFile *
                 sys_mapped_file_new (const SysChar  *filename,
                     SysBool      writable,
                     SysError      **error)
{
  SysMappedFile *file;
  int fd;

  sys_return_val_if_fail (filename != NULL, NULL);
  sys_return_val_if_fail (!error || *error == NULL, NULL);

  fd = sys_open (filename, (writable ? O_RDWR : O_RDONLY) | _O_BINARY | O_CLOEXEC, 0);
  if (fd == -1)
  {
    int save_errno = errno;
    SysChar *display_filename = sys_filename_display_name (filename);

    sys_error_set_N (error,
        SYS_("Failed to open file “%s”: open() failed: %s"),
        display_filename,
        sys_strerror (save_errno));
    sys_free (display_filename);
    return NULL;
  }

  file = mapped_file_new_from_fd (fd, writable, filename, error);

  close (fd);

  return file;
}


/**
 * sys_mapped_file_new_from_fd:
 * @fd: The file descriptor of the file to load
 * @writable: whether the mapping should be writable
 * @error: return location for a #SysError, or %NULL
 *
 * Maps a file into memory. On UNIX, this is using the mmap() function.
 *
 * If @writable is %true, the mapped buffer may be modified, otherwise
 * it is an error to modify the mapped buffer. Modifications to the buffer
 * are not visible to other processes mapping the same file, and are not
 * written back to the file.
 *
 * Note that modifications of the underlying file might affect the contents
 * of the #SysMappedFile. Therefore, mapping should only be used if the file
 * will not be modified, or if all modifications of the file are done
 * atomically (e.g. using sys_file_set_contents()).
 *
 * Returns: a newly allocated #SysMappedFile which must be unref'd
 *    with sys_mapped_file_unref(), or %NULL if the mapping failed.
 *
 * Since: 2.32
 */
  SysMappedFile *
sys_mapped_file_new_from_fd (SysInt          fd,
    SysBool      writable,
    SysError      **error)
{
  return mapped_file_new_from_fd (fd, writable, NULL, error);
}

/**
 * sys_mapped_file_get_length:
 * @file: a #SysMappedFile
 *
 * Returns the length of the contents of a #SysMappedFile.
 *
 * Returns: the length of the contents of @file.
 *
 * Since: 2.8
 */
  SysSize
sys_mapped_file_get_length (SysMappedFile *file)
{
  sys_return_val_if_fail (file != NULL, 0);

  return file->length;
}

/**
 * sys_mapped_file_get_contents:
 * @file: a #SysMappedFile
 *
 * Returns the contents of a #SysMappedFile. 
 *
 * Note that the contents may not be zero-terminated,
 * even if the #SysMappedFile is backed by a text file.
 *
 * If the file is empty then %NULL is returned.
 *
 * Returns: the contents of @file, or %NULL.
 *
 * Since: 2.8
 */
  SysChar *
sys_mapped_file_get_contents (SysMappedFile *file)
{
  sys_return_val_if_fail (file != NULL, NULL);

  return file->contents;
}

/**
 * sys_mapped_file_free:
 * @file: a #SysMappedFile
 *
 * This call existed before #SysMappedFile had refcounting and is currently
 * exactly the same as sys_mapped_file_unref().
 *
 * Since: 2.8
 * Deprecated:2.22: Use sys_mapped_file_unref() instead.
 */
  void
sys_mapped_file_free (SysMappedFile *file)
{
  sys_mapped_file_unref (file);
}

/**
 * sys_mapped_file_ref:
 * @file: a #SysMappedFile
 *
 * Increments the reference count of @file by one.  It is safe to call
 * this function from any thread.
 *
 * Returns: the passed in #SysMappedFile.
 *
 * Since: 2.22
 **/
  SysMappedFile *
sys_mapped_file_ref (SysMappedFile *file)
{
  sys_return_val_if_fail (file != NULL, NULL);

  sys_atomic_int_inc (&file->ref_count);

  return file;
}

/**
 * sys_mapped_file_unref:
 * @file: a #SysMappedFile
 *
 * Decrements the reference count of @file by one.  If the reference count
 * drops to 0, unmaps the buffer of @file and frees it.
 *
 * It is safe to call this function from any thread.
 *
 * Since 2.22
 **/
  void
sys_mapped_file_unref (SysMappedFile *file)
{
  sys_return_if_fail (file != NULL);

  if (sys_atomic_int_dec_and_test (&file->ref_count))
    sys_mapped_file_destroy (file);
}

/**
 * sys_mapped_file_get_bytes:
 * @file: a #SysMappedFile
 *
 * Creates a new #SysBytes which references the data mapped from @file.
 * The mapped contents of the file must not be modified after creating this
 * bytes object, because a #SysBytes should be immutable.
 *
 * Returns: (transfer full): A newly allocated #SysBytes referencing data
 *     from @file
 *
 * Since: 2.34
 **/
  SysBytes *
            sys_mapped_file_get_bytes (SysMappedFile *file)
{
  sys_return_val_if_fail (file != NULL, NULL);

  return sys_bytes_new_with_free_func (file->contents,
      file->length,
      (SysDestroyFunc) sys_mapped_file_unref,
      sys_mapped_file_ref (file));
}
