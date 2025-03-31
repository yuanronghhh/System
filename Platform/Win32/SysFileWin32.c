#include <System/Platform/Common/SysFilePrivate.h>
#include <System/Utils/SysPath.h>
#include <System/Utils/SysUtf8.h>

static int _sys_win32_readlink_utf16_handle (const SysUniChar2  *filename,
                                HANDLE            file_handle,
                                DWORD            *reparse_tag,
                                SysUniChar2        *buf,
                                SysSize             buf_size,
                                SysUniChar2       **alloc_buf,
                                SysBool          terminate);

static SysChar exepath[MAX_PATH];

FILE *sys_fopen(const SysChar *filename, const SysChar * mode) {
  FILE *fp;

  errno_t eno = fopen_s(&fp, filename, mode);
  if (eno == 0) {
    return fp;
  }

  sys_error_N("%s: %s", sys_strerror(errno), filename);
  return NULL;
}

SysSize sys_fread(
    void*  buffer,
    SysSize bufsize,
    SysSize elem_size,
    SysSize elem_count,
    FILE*  fp) {
  return fread_s(buffer, bufsize, elem_size, elem_count, fp);
}

SysInt sys_open(const SysChar *filename, SysInt flags, SysInt mode) {
  SysInt fp;
  SysInt shareflag = SH_DENYNO;

  errno_t eno = _sopen_s(&fp, filename, flags, shareflag, mode);
  if (eno == 0) {
    return fp;
  }

  sys_error_N("%s: %s", sys_strerror(errno), filename);
  return -1;
}

SysInt sys_close(SysInt fd) {
  return _close(fd);
}

SysSSize sys_read(SysInt fd, SysPointer buf, SysSize mcount) {
  return _read(fd, buf, (SysUInt)mcount);
}

const SysChar *sys_exe_path(void) {
  SysULong nlen;
  SysChar *sp;

  sp = &exepath[0];

  nlen = GetModuleFileName(0, sp, MAX_PATH);
  sys_path_escape(sp);
  UNUSED(nlen);

  return sp;
}

void sys_fcloseall(void) {
  _fcloseall();
}

static int sys_win32_readlink_utf8 (const SysChar  *filename,
                       SysChar        *buf,
                       SysSize         buf_size,
                       SysChar       **alloc_buf,
                       SysBool      terminate) {
  wchar_t *wfilename;
  int result;
  wchar_t *buf_utf16;
  SysLong tmp_len;
  SysChar *tmp;

  sys_return_val_if_fail ((buf != NULL || alloc_buf != NULL) &&
                        (buf == NULL || alloc_buf == NULL),
                        -1);

  wfilename = sys_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);

  if (wfilename == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  result = _sys_win32_readlink_utf16_handle (wfilename, NULL, NULL,
                                           NULL, 0, &buf_utf16, terminate);

  sys_free (wfilename);

  if (result <= 0)
    return result;

  tmp = sys_utf16_to_utf8 (buf_utf16,
                         result / sizeof (SysUniChar2),
                         NULL,
                         &tmp_len,
                         NULL);

  sys_free (buf_utf16);

  if (tmp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  if (alloc_buf)
    {
      *alloc_buf = tmp;
      return tmp_len;
    }

  if ((SysSize) tmp_len > buf_size)
    tmp_len = (SysLong)buf_size;

  memcpy (buf, tmp, tmp_len);
  sys_free (tmp);

  return tmp_len;
}


SysChar *sys_file_read_link(const SysChar  *filename, SysError **error) {
  SysChar *buffer;
  SysInt read_size;

  sys_return_val_if_fail(filename == NULL, NULL);

  sys_return_val_if_fail (filename != NULL, NULL);
  sys_return_val_if_fail (error == NULL || *error == NULL, NULL);

  read_size = sys_win32_readlink_utf8 (filename, NULL, 0, &buffer, true);
  if (read_size < 0) {

    int saved_errno = errno;
    if (error)
      sys_error_set_N (error,
          SYS_("Failed to read the symbolic link “%s”: %s"),
          filename,
          saved_errno);
    return NULL;

  } else if (read_size == 0) {

    return sys_strdup ("");

  } else {

    return buffer;
  }
}

SysBool sys_file_state_get_by_filename(const SysChar *filename, SysFileState* state) {
  SysInt fno;
  struct stat _fstate;

  sys_return_val_if_fail(state != NULL, false);
  sys_return_val_if_fail(filename != NULL, false);

  fno = stat(filename, &_fstate);
  if (fno == -1) { return false; }

  state->st_size = _fstate.st_size;
  state->is_dir = S_ISDIR (_fstate.st_mode);

  return true;
}

static int w32_error_to_errno (DWORD error_code) {
  switch (error_code)
    {
    case ERROR_ACCESS_DENIED:
      return EACCES;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
      return EEXIST;
    case ERROR_FILE_NOT_FOUND:
      return ENOENT;
    case ERROR_INVALID_FUNCTION:
      return EFAULT;
    case ERROR_INVALID_HANDLE:
      return EBADF;
    case ERROR_INVALID_PARAMETER:
      return EINVAL;
    case ERROR_LOCK_VIOLATION:
    case ERROR_SHARING_VIOLATION:
      return EACCES;
    case ERROR_NOT_ENOUGH_MEMORY:
    case ERROR_OUTOFMEMORY:
      return ENOMEM;
    case ERROR_NOT_SAME_DEVICE:
      return EXDEV;
    case ERROR_PATH_NOT_FOUND:
      return ENOENT; /* or ELOOP, or ENAMETOOLONG */
    default:
      return EIO;
    }
}

static SysInt64
_sys_win32_filetime_to_unix_time (const FILETIME *ft,
                                SysInt32         *nsec)
{
  SysInt64 result;
  /* 1 unit of FILETIME is 100ns */
  const SysInt64 hundreds_of_usec_per_sec = 10000000;
  /* The difference between January 1, 1601 UTC (FILETIME epoch) and UNIX epoch
   * in hundreds of nanoseconds.
   */
  const SysInt64 filetime_unix_epoch_offset = 116444736000000000;

  result = ((SysInt64) ft->dwLowDateTime) | (((SysInt64) ft->dwHighDateTime) << 32);
  result -= filetime_unix_epoch_offset;

  if (nsec)
    *nsec = (SysInt32)(result % hundreds_of_usec_per_sec) * 100;

  return result / hundreds_of_usec_per_sec;
}

static int
_sys_win32_copy_and_maybe_terminate (const SysUChar *data,
                                   SysSize         in_to_copy,
                                   SysUniChar2    *buf,
                                   SysSize         buf_size,
                                   SysUniChar2   **alloc_buf,
                                   SysBool      terminate)
{
  SysSize to_copy = in_to_copy;
  /* Number of bytes we can use to add extra zeroes for NUL-termination.
   * 0 means that we can destroy up to 2 bytes of data,
   * 1 means that we can destroy up to 1 byte of data,
   * 2 means that we do not perform destructive NUL-termination
   */
  SysSize extra_bytes = terminate ? 2 : 0;
  char *buf_in_chars;

  if (to_copy == 0)
    return 0;

  /* 2 bytes is sizeof (wchar_t), for an extra NUL-terminator. */
  if (buf)
    {
      if (to_copy >= buf_size)
        {
          extra_bytes = 0;
          to_copy = buf_size;
        }
      else if (to_copy > buf_size - 2)
        {
          extra_bytes = 1;
        }

      memcpy (buf, data, to_copy);
    }
  else
    {
      /* Note that SubstituteNameLength is USHORT, so to_copy + 2, being
       * SysSize, never overflows.
       */
      *alloc_buf = sys_malloc (to_copy + extra_bytes);
      memcpy (*alloc_buf, data, to_copy);
    }

  if (!terminate)
    return (SysInt)to_copy;

  if (buf)
    buf_in_chars = (char *) buf;
  else
    buf_in_chars = (char *) *alloc_buf;

  if (to_copy >= 2 && buf_in_chars[to_copy - 2] == 0 &&
      buf_in_chars[to_copy - 1] == 0)
    {
      /* Fully NUL-terminated, do nothing */
    }
  else if ((to_copy == 1 || buf_in_chars[to_copy - 2] != 0) &&
           buf_in_chars[to_copy - 1] == 0)
    {
      /* Have one zero, try to add another one */
      if (extra_bytes > 0)
        {
          /* Append trailing zero */
          buf_in_chars[to_copy] = 0;
          /* Be precise about the number of bytes we return */
          to_copy += 1;
        }
      else if (to_copy >= 2)
        {
          /* No space for appending, destroy one byte */
          buf_in_chars[to_copy - 2] = 0;
        }
      /* else there's no space at all (to_copy == 1), do nothing */
    }
  else if (extra_bytes > 0 || to_copy >= 2)
    {
      buf_in_chars[to_copy - 2 + extra_bytes] = 0;
      buf_in_chars[to_copy - 1 + extra_bytes] = 0;
      to_copy += extra_bytes;
    }
  else /* extra_bytes == 0 && to_copy == 1 */
    {
      buf_in_chars[0] = 0;
    }

  return (SysInt)to_copy;
}


static int
_sys_win32_readlink_handle_raw (HANDLE      h,
                              DWORD      *reparse_tag,
                              SysUniChar2  *buf,
                              SysSize       buf_size,
                              SysUniChar2 **alloc_buf,
                              SysBool    terminate) {
  DWORD error_code;
  DWORD returned_bytes = 0;
  BYTE *data = NULL;
  SysSize to_copy;
  /* This is 16k. It's impossible to make DeviceIoControl() tell us
   * the required size. NtFsControlFile() does have such a feature,
   * but for some reason it doesn't work with CreateFile()-returned handles.
   * The only alternative is to repeatedly call DeviceIoControl()
   * with bigger and bigger buffers, until it succeeds.
   * We choose to sacrifice stack space for speed.
   */
  BYTE max_buffer[sizeof (REPARSE_DATA_BUFFER) + MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = {0,};
  DWORD max_buffer_size = sizeof (REPARSE_DATA_BUFFER) + MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
  REPARSE_DATA_BUFFER *rep_buf;

  sys_return_val_if_fail ((buf != NULL || alloc_buf != NULL || reparse_tag != NULL) &&
                        (buf == NULL || alloc_buf == NULL),
                        -1);

  if (!DeviceIoControl (h, FSCTL_GET_REPARSE_POINT, NULL, 0,
                        max_buffer,
                        max_buffer_size,
                        &returned_bytes, NULL))
    {
      error_code = GetLastError ();
      errno = w32_error_to_errno (error_code);
      return -1;
    }

  rep_buf = (REPARSE_DATA_BUFFER *) max_buffer;

  if (reparse_tag != NULL)
    *reparse_tag = rep_buf->ReparseTag;

  if (buf == NULL && alloc_buf == NULL)
    return 0;

  if (rep_buf->ReparseTag == IO_REPARSE_TAG_SYMLINK)
    {
      data = &((BYTE *) rep_buf->SymbolicLinkReparseBuffer.PathBuffer)[rep_buf->SymbolicLinkReparseBuffer.SubstituteNameOffset];

      to_copy = rep_buf->SymbolicLinkReparseBuffer.SubstituteNameLength;
    }
  else if (rep_buf->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
    {
      data = &((BYTE *) rep_buf->MountPointReparseBuffer.PathBuffer)[rep_buf->MountPointReparseBuffer.SubstituteNameOffset];

      to_copy = rep_buf->MountPointReparseBuffer.SubstituteNameLength;
    }
  else
    to_copy = 0;

  return _sys_win32_copy_and_maybe_terminate (data, to_copy, buf, buf_size, alloc_buf, terminate);
}

static int _sys_win32_readlink_utf16_raw (const SysUniChar2  *filename,
                             DWORD            *reparse_tag,
                             SysUniChar2        *buf,
                             SysSize             buf_size,
                             SysUniChar2       **alloc_buf,
                             SysBool          terminate) {
  HANDLE h;
  DWORD attributes;
  DWORD to_copy;
  DWORD error_code;

  if ((attributes = GetFileAttributesW (filename)) == 0)
    {
      error_code = GetLastError ();
      errno = w32_error_to_errno (error_code);
      return -1;
    }

  if ((attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0)
    {
      errno = EINVAL;
      return -1;
    }

  /* To read symlink target we need to open the file as a reparse
   * point and use DeviceIoControl() on it.
   */
  h = CreateFileW (filename,
                   FILE_READ_EA,
                   FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                   NULL, OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL
                   | FILE_FLAG_OPEN_REPARSE_POINT
                   | (attributes & FILE_ATTRIBUTE_DIRECTORY ? FILE_FLAG_BACKUP_SEMANTICS : 0),
                   NULL);

  if (h == INVALID_HANDLE_VALUE)
    {
      error_code = GetLastError ();
      errno = w32_error_to_errno (error_code);
      return -1;
    }

  to_copy = _sys_win32_readlink_handle_raw (h, reparse_tag, buf, buf_size, alloc_buf, terminate);

  CloseHandle (h);

  return to_copy;
}

static SysBool _sys_win32_strip_extended_ntobjm_prefix (SysUniChar2 *str,
                                       SysSize     *str_size)
{
  const wchar_t *extended_prefix = L"\\\\?\\";
  const SysSize    extended_prefix_len = wcslen (extended_prefix);
  const SysSize    extended_prefix_len_bytes = sizeof (SysUniChar2) * extended_prefix_len;
  const SysSize    extended_prefix_with_drive_len_bytes = sizeof (SysUniChar2) * (extended_prefix_len + 2);
  const wchar_t *ntobjm_prefix = L"\\??\\";
  const SysSize    ntobjm_prefix_len = wcslen (ntobjm_prefix);
  const SysSize    ntobjm_prefix_len_bytes = sizeof (SysUniChar2) * ntobjm_prefix_len;
  const SysSize    ntobjm_prefix_with_drive_len_bytes = sizeof (SysUniChar2) * (ntobjm_prefix_len + 2);
  SysBool do_move = false;
  SysSize move_shift = 0;

  if ((*str_size) * sizeof (SysUniChar2) > extended_prefix_with_drive_len_bytes &&
      memcmp (str,
              extended_prefix,
              extended_prefix_len_bytes) == 0 &&
      iswascii (str[extended_prefix_len]) &&
      iswalpha (str[extended_prefix_len]) &&
      str[extended_prefix_len + 1] == L':')
   {
     do_move = true;
     move_shift = extended_prefix_len;
   }
  else if ((*str_size) * sizeof (SysUniChar2) > ntobjm_prefix_with_drive_len_bytes &&
           memcmp (str,
                   ntobjm_prefix,
                   ntobjm_prefix_len_bytes) == 0 &&
           iswascii (str[ntobjm_prefix_len]) &&
           iswalpha (str[ntobjm_prefix_len]) &&
           str[ntobjm_prefix_len + 1] == L':')
    {
      do_move = true;
      move_shift = ntobjm_prefix_len;
    }

  if (do_move)
    {
      *str_size -= move_shift;
      memmove (str,
               str + move_shift,
               (*str_size) * sizeof (SysUniChar2));
    }

  return do_move;
}


static int _sys_win32_readlink_utf16_handle (const SysUniChar2  *filename,
                                HANDLE            file_handle,
                                DWORD            *reparse_tag,
                                SysUniChar2        *buf,
                                SysSize             buf_size,
                                SysUniChar2       **alloc_buf,
                                SysBool          terminate) {
  int   result;
  SysSize string_size;

  sys_return_val_if_fail ((buf != NULL || alloc_buf != NULL || reparse_tag != NULL) &&
                        (filename != NULL || file_handle != NULL) &&
                        (buf == NULL || alloc_buf == NULL) &&
                        (filename == NULL || file_handle == NULL),
                        -1);

  if (filename)
    result = _sys_win32_readlink_utf16_raw (filename, reparse_tag, buf, buf_size, alloc_buf, terminate);
  else
    result = _sys_win32_readlink_handle_raw (file_handle, reparse_tag, buf, buf_size, alloc_buf, terminate);

  if (result <= 0)
    return result;

  /* Ensure that output is a multiple of sizeof (SysUniChar2),
   * cutting any trailing partial SysUniChar2, if present.
   */
  result -= result % sizeof (SysUniChar2);

  if (result <= 0)
    return result;

  /* DeviceIoControl () tends to return filenames as NT Object Manager
   * names , i.e. "\\??\\C:\\foo\\bar".
   * Remove the leading 4-byte "\\??\\" prefix, as glib (as well as many W32 API
   * functions) is unprepared to deal with it. Unless it has no 'x:' drive
   * letter part after the prefix, in which case we leave everything
   * as-is, because the path could be "\\??\\Volume{GUID}" - stripping
   * the prefix will allow it to be confused with relative links
   * targeting "Volume{GUID}".
   */
  string_size = result / sizeof (SysUniChar2);
  _sys_win32_strip_extended_ntobjm_prefix (buf ? buf : *alloc_buf, &string_size);

  return (SysInt)(string_size * sizeof (SysUniChar2));
}

static int
_sys_win32_fill_statbuf_from_handle_info (const wchar_t                    *filename,
                                        const wchar_t                    *filename_target,
                                        const BY_HANDLE_FILE_INFORMATION *handle_info,
                                        struct __stat64                  *statbuf)
{
  wchar_t drive_letter_w = 0;
  size_t drive_letter_size = MB_CUR_MAX;
  char *drive_letter = _alloca (drive_letter_size);

  /* If filename (target or link) is absolute,
   * then use the drive letter from it as-is.
   */
  if (filename_target != NULL &&
      filename_target[0] != L'\0' &&
      filename_target[1] == L':')
    drive_letter_w = filename_target[0];
  else if (filename[0] != L'\0' &&
           filename[1] == L':')
    drive_letter_w = filename[0];

  if (drive_letter_w > 0 &&
      iswalpha (drive_letter_w) &&
      iswascii (drive_letter_w) &&
      wctomb (drive_letter, drive_letter_w) == 1)
    statbuf->st_dev = toupper (drive_letter[0]) - 'A'; /* 0 means A: drive */
  else
    /* Otherwise use the PWD drive.
     * Return value of 0 gives us 0 - 1 = -1,
     * which is the "no idea" value for st_dev.
     */
    statbuf->st_dev = _getdrive () - 1;

  statbuf->st_rdev = statbuf->st_dev;
  /* Theoretically, it's possible to set it for ext-FS. No idea how.
   * Meaningless for all filesystems that Windows normally uses.
   */
  statbuf->st_ino = 0;
  statbuf->st_mode = 0;

  if ((handle_info->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    statbuf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
  else
    statbuf->st_mode |= S_IFREG;
  /* No idea what S_IFCHR means here. */
  /* S_IFIFO is not even mentioned in MSDN */
  /* S_IFBLK is also not mentioned */

  /* The aim here is to reproduce MS stat() behaviour,
   * even if it's braindead.
   */
  statbuf->st_mode |= S_IRUSR | S_IRGRP | S_IROTH;
  if ((handle_info->dwFileAttributes & FILE_ATTRIBUTE_READONLY) != FILE_ATTRIBUTE_READONLY)
    statbuf->st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;

  if (!S_ISDIR (statbuf->st_mode))
    {
      const wchar_t *name;
      const wchar_t *dot = NULL;

      if (filename_target != NULL)
        name = filename_target;
      else
        name = filename;

      do
        {
          wchar_t *last_dot = wcschr (name, L'.');
          if (last_dot == NULL)
            break;
          dot = last_dot;
          name = &last_dot[1];
        }
      while (true);

      if ((dot != NULL &&
          (wcsicmp (dot, L".exe") == 0 ||
           wcsicmp (dot, L".com") == 0 ||
           wcsicmp (dot, L".bat") == 0 ||
           wcsicmp (dot, L".cmd") == 0)))
        statbuf->st_mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    }

  statbuf->st_nlink = (SysShort)handle_info->nNumberOfLinks;
  statbuf->st_uid = statbuf->st_gid = 0;
  statbuf->st_size = (((SysUInt64) handle_info->nFileSizeHigh) << 32) | handle_info->nFileSizeLow;
  statbuf->st_ctime = _sys_win32_filetime_to_unix_time (&handle_info->ftCreationTime, NULL);
  statbuf->st_mtime = _sys_win32_filetime_to_unix_time (&handle_info->ftLastWriteTime, NULL);
  statbuf->st_atime = _sys_win32_filetime_to_unix_time (&handle_info->ftLastAccessTime, NULL);

  return 0;
}

static void _sys_win32_fill_privatestat (const struct __stat64            *statbuf,
                           const BY_HANDLE_FILE_INFORMATION *handle_info,
                           const FILE_STANDARD_INFO         *std_info,
                           DWORD                             reparse_tag,
                           SysWin32PrivateStat *buf)
{
  SysInt32 nsec;

  buf->st_dev = statbuf->st_dev;
  buf->st_ino = statbuf->st_ino;
  buf->st_mode = statbuf->st_mode;
  buf->volume_serial = handle_info->dwVolumeSerialNumber;
  buf->file_index = (((SysUInt64) handle_info->nFileIndexHigh) << 32) | handle_info->nFileIndexLow;
  buf->attributes = handle_info->dwFileAttributes;
  buf->st_nlink = handle_info->nNumberOfLinks;
  buf->st_size = (((SysUInt64) handle_info->nFileSizeHigh) << 32) | handle_info->nFileSizeLow;
  buf->allocated_size = std_info->AllocationSize.QuadPart;

  buf->reparse_tag = reparse_tag;

  buf->st_ctim.tv_sec = _sys_win32_filetime_to_unix_time (&handle_info->ftCreationTime, &nsec);
  buf->st_ctim.tv_nsec = nsec;
  buf->st_mtim.tv_sec = _sys_win32_filetime_to_unix_time (&handle_info->ftLastWriteTime, &nsec);
  buf->st_mtim.tv_nsec = nsec;
  buf->st_atim.tv_sec = _sys_win32_filetime_to_unix_time (&handle_info->ftLastAccessTime, &nsec);
  buf->st_atim.tv_nsec = nsec;
}

static int _sys_win32_stat_utf16_no_trailing_slashes (const SysUniChar2    *filename,
    SysWin32PrivateStat  *buf,
    SysBool            for_symlink)
{
  struct __stat64 statbuf;
  BY_HANDLE_FILE_INFORMATION handle_info;
  FILE_STANDARD_INFO std_info;
  SysBool is_symlink = false;
  wchar_t *filename_target = NULL;
  DWORD immediate_attributes;
  DWORD open_flags;
  SysBool is_directory;
  DWORD reparse_tag = 0;
  DWORD error_code;
  BOOL succeeded_so_far;
  HANDLE file_handle;

  immediate_attributes = GetFileAttributesW (filename);

  if (immediate_attributes == INVALID_FILE_ATTRIBUTES)
    {
      error_code = GetLastError ();
      errno = w32_error_to_errno (error_code);

      return -1;
    }

  is_symlink = (immediate_attributes & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT;
  is_directory = (immediate_attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;

  open_flags = FILE_ATTRIBUTE_NORMAL;

  if (for_symlink && is_symlink)
    open_flags |= FILE_FLAG_OPEN_REPARSE_POINT;

  if (is_directory)
    open_flags |= FILE_FLAG_BACKUP_SEMANTICS;

  file_handle = CreateFileW (filename, FILE_READ_ATTRIBUTES | FILE_READ_EA,
                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             NULL, OPEN_EXISTING,
                             open_flags,
                             NULL);

  if (file_handle == INVALID_HANDLE_VALUE)
    {
      error_code = GetLastError ();
      errno = w32_error_to_errno (error_code);
      return -1;
    }

  succeeded_so_far = GetFileInformationByHandle (file_handle,
                                                 &handle_info);
  error_code = GetLastError ();

  if (succeeded_so_far)
    {
      succeeded_so_far = GetFileInformationByHandleEx (file_handle,
                                                       FileStandardInfo,
                                                       &std_info,
                                                       sizeof (std_info));
      error_code = GetLastError ();
    }

  if (!succeeded_so_far)
    {
      CloseHandle (file_handle);
      errno = w32_error_to_errno (error_code);
      return -1;
    }

  /* It's tempting to use GetFileInformationByHandleEx(FileAttributeTagInfo),
   * but it always reports that the ReparseTag is 0.
   * We already have a handle open for symlink, use that.
   * For the target we have to specify a filename, and the function
   * will open another handle internally.
   */
  if (is_symlink &&
      _sys_win32_readlink_utf16_handle (for_symlink ? NULL : filename,
                                      for_symlink ? file_handle : NULL,
                                      &reparse_tag,
                                      NULL, 0,
                                      for_symlink ? NULL : &filename_target,
                                      true) < 0)
    {
      CloseHandle (file_handle);
      return -1;
    }

  CloseHandle (file_handle);

  _sys_win32_fill_statbuf_from_handle_info (filename,
                                          filename_target,
                                          &handle_info,
                                          &statbuf);
  sys_free (filename_target);
  _sys_win32_fill_privatestat (&statbuf,
                             &handle_info,
                             &std_info,
                             reparse_tag,
                             buf);

  return 0;
}


static int
_sys_win32_stat_utf8 (const SysChar       *filename,
                    SysWin32PrivateStat *buf,
                    SysBool for_symlink)
{
  wchar_t *wfilename;
  int result;
  SysSize len;

  if (filename == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  len = strlen (filename);

  while (len > 0 && SYS_IS_DIR_SEPARATOR (filename[len - 1]))
    len--;

  if (len <= 0 ||
      (sys_path_is_absolute (filename) && len <= (SysSize) (sys_path_skip_root (filename) - filename)))
    len = strlen (filename);

  wfilename = sys_utf8_to_utf16 (filename, (SysULong)len, NULL, NULL, NULL);

  if (wfilename == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  result = _sys_win32_stat_utf16_no_trailing_slashes (wfilename, buf, for_symlink);

  sys_free (wfilename);

  return result;
}

static int sys_win32_lstat_utf8 (const SysChar       *filename,
                    SysWin32PrivateStat *buf)
{
  return _sys_win32_stat_utf8 (filename, buf, true);
}

SysInt sys_lstat(const SysChar *filename, struct stat * buf) {
  SysWin32PrivateStat w32_buf;
  int retval;

  sys_return_val_if_fail(buf != NULL, -1);
  sys_return_val_if_fail(filename != NULL, -1);
  
  retval = sys_win32_lstat_utf8(filename, &w32_buf);
  buf->st_dev = w32_buf.st_dev;
  buf->st_ino = (SysUShort)w32_buf.st_ino;
  buf->st_mode = w32_buf.st_mode;
  buf->st_nlink = (SysShort)w32_buf.st_nlink;
  buf->st_uid = w32_buf.st_uid;
  buf->st_gid = w32_buf.st_gid;
  buf->st_rdev = w32_buf.st_dev;
  buf->st_size = (SysLong)w32_buf.st_size;
  buf->st_atime = w32_buf.st_atim.tv_sec;
  buf->st_mtime = w32_buf.st_mtim.tv_sec;
  buf->st_ctime = w32_buf.st_ctim.tv_sec;

  return retval;
}

