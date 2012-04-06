/*
 *
 *   Copyright (C) 2005-2012 by C.H. Huang
 *   plushuang.tw@gmail.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  ---
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of portions of this program with the
 *  OpenSSL library under certain conditions as described in each
 *  individual source file, and distribute linked combinations
 *  including the two.
 *  You must obey the GNU Lesser General Public License in all respects
 *  for all of the code used other than OpenSSL.  If you modify
 *  file(s) with this exception, you may extend this exception to your
 *  version of the file(s), but you are not obligated to do so.  If you
 *  do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source
 *  files in the program, then also delete it here.
 *
 */

//	Enhanced version of gstdio.h
//	Support LFS(Large File Support) and UTF-8 filename
//	It can compile with MS Visual C++, Mingw, Unix...etc.

// To enable LFS (Large File Support) in UNIX platform
// add `getconf LFS_CFLAGS`  to CFLAGS
// add `getconf LFS_LDFLAGS` to LDFLAGS

#ifndef UG_STDIO_H
#define UG_STDIO_H

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <fcntl.h>       // for O_* flags
//#include <sys/types.h>
#include <sys/stat.h>    // for S_* mode
//#include <stdio.h>
#include <glib.h>
#include <glib/gstdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// To enable LFS (Large File Support) in UNIX platform
// add `getconf LFS_CFLAGS`  to CFLAGS
// add `getconf LFS_LDFLAGS` to LDFLAGS

// ------------------------------------------------------------------
// low level file I/O
// wrapper functions/definitions for file descriptor.

// redefine flags: O_APPEND, O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_EXCL
// redefine mode: S_IREAD, S_IWRITE
#if defined(_WIN32)
   // flags
#  define UG_FD_O_APPEND	_O_APPEND
#  define UG_FD_O_BINARY	_O_BINARY	// Text is the default in MS platform
#  define UG_FD_O_TEXT		_O_TEXT		// Text is the default in MS platform
#  define UG_FD_O_RDONLY	_O_RDONLY
#  define UG_FD_O_WRONLY	_O_WRONLY
#  define UG_FD_O_RDWR		_O_RDWR
#  define UG_FD_O_CREAT		_O_CREAT
#  define UG_FD_O_EXCL		_O_EXCL		// Use with _O_CREAT, return error if file exist.
   // mnemonic flags
#  define UG_FD_O_READONLY	_O_RDONLY
#  define UG_FD_O_WRITEONLY	_O_WRONLY
#  define UG_FD_O_READWRITE	_O_RDWR
#  define UG_FD_O_CREATE	_O_CREAT
   // mode
#  define UG_FD_S_IREAD		_S_IREAD	// Use with _O_CREAT
#  define UG_FD_S_IWRITE	_S_IWRITE	// Use with _O_CREAT
#  define UG_FD_S_IRGRP		0			// Use with _O_CREAT
#  define UG_FD_S_IWGRP		0			// Use with _O_CREAT
#  define UG_FD_S_IROTH		0			// Use with _O_CREAT
#  define UG_FD_S_IWOTH		0			// Use with _O_CREAT
#else
   // flags
#  define UG_FD_O_APPEND	O_APPEND
#  ifdef O_BINARY
#    define UG_FD_O_BINARY	O_BINARY	// Text is the default in MS platform
#    define UG_FD_O_TEXT	O_TEXT		// Text is the default in MS platform
#  else
#    define UG_FD_O_BINARY	0			// Text is the default in MS platform
#    define UG_FD_O_TEXT	0			// Text is the default in MS platform
#  endif
#  define UG_FD_O_RDONLY	O_RDONLY
#  define UG_FD_O_WRONLY	O_WRONLY
#  define UG_FD_O_RDWR		O_RDWR
#  define UG_FD_O_CREAT		O_CREAT
#  define UG_FD_O_EXCL		O_EXCL		// Use with O_CREAT, return error if file exist.
   // mnemonic flags
#  define UG_FD_O_READONLY	O_RDONLY
#  define UG_FD_O_WRITEONLY	O_WRONLY
#  define UG_FD_O_READWRITE	O_RDWR
#  define UG_FD_O_CREATE	O_CREAT
   // mode
#  define UG_FD_S_IREAD		S_IREAD		// Use with O_CREAT
#  define UG_FD_S_IWRITE	S_IWRITE	// Use with O_CREAT
#  define UG_FD_S_IRGRP		S_IRGRP		// Use with O_CREAT	// GROUP READ
#  define UG_FD_S_IWGRP		S_IWGRP		// Use with O_CREAT	// GROUP WRITE
#  define UG_FD_S_IROTH		S_IROTH		// Use with O_CREAT	// OTHERS READ
#  define UG_FD_S_IWOTH		S_IWOTH		// Use with O_CREAT	// OTHERS WRITE
#endif

// ug_fd_open ()
// Returns :  a new file descriptor, or -1 if an error occurred.
#if defined(__MINGW32__)
#define ug_fd_open         g_open
#else
int		ug_fd_open (const gchar* filename_utf8, int flags, int mode);
#endif

#if defined(_WIN32)
int		ug_fd_truncate (int fd, gint64 length);
#endif

#ifdef _WIN32
#  define  ug_fd_close		_close
#  define  ug_fd_read		_read
#  define  ug_fd_write		_write
#  define  ug_fd_seek		_lseeki64	// for MS VC
#  define  ug_fd_tell		_telli64	// for MS VC
#else
#  define  ug_fd_close		close
#  define  ug_fd_read		read
#  define  ug_fd_write		write
#  define  ug_fd_seek		lseek
#  define  ug_fd_tell(fd)	lseek(fd, 0L, SEEK_CUR)
#  define  ug_fd_truncate	ftruncate
#endif

// ------------------------------------------------------------------
// streaming file I/O
// wrapper functions/definitions for file stream. (struct FILE)

#if defined(_WIN32)
#  if defined(__MINGW32__)
#    define ug_fopen				g_fopen
#    define ug_fseek				fseeko64
#    define ug_ftell				ftello64
#  else     //  __MSVC__
#    define ug_fseek				_fseeki64
#    define ug_ftell				_ftelli64
#  endif
#  define ug_fileno					_fileno
#else
#  define ug_fseek					fseek
#  define ug_ftell					ftell
#  define ug_fileno					fileno
#endif

#if !defined(__MINGW32__)
FILE*	ug_fopen (const gchar *filename_utf8, const gchar *mode);
#endif
int		ug_fs_truncate (FILE* file, gint64 size);

#define ug_fs_open					ug_fopen
#define ug_fs_close					fclose
#define ug_fs_read(file,data,len)	fread  (data, len, 1, file)
#define ug_fs_write(file,data,len)	fwrite (data, len, 1, file)
#define ug_fs_printf				fprintf
#define ug_fs_puts(file,string)		fputs (string, file)
#define ug_fs_putc(file,character)	fputc (character, file)
#define	ug_fs_gets(file,buf,len)	fgets (buf, len, file)
#define	ug_fs_getc(file)			fgetc (file)
#define ug_fs_flush					fflush
#define ug_fs_seek					ug_fseek
#define ug_fs_tell					ug_ftell
#define ug_fs_get_fd				ug_fileno

#ifdef __cplusplus
}
#endif

#endif  // UG_STDIO_H
