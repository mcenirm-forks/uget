/*
 *
 *   Copyright (C) 2005-2013 by C.H. Huang
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

#ifdef _WIN32
#define  _CRT_SECURE_NO_DEPRECATE    // avoid some warning (MS VC 2005)
#include <windows.h>
#endif

#include <errno.h>
#include <UgStdio.h>

#ifdef _WIN32
int  ug_fd_open (const gchar* filename_utf8, int flags, int mode)
{
	int retval;
	int save_errno;

	if (G_WIN32_HAVE_WIDECHAR_API ()) {
		wchar_t *wfilename = g_utf8_to_utf16 (filename_utf8, -1, NULL, NULL, NULL);

		if (wfilename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = _wopen (wfilename, flags, mode);
		save_errno = errno;
		g_free (wfilename);
	}
	else {
		gchar *cp_filename = g_locale_from_utf8 (filename_utf8, -1, NULL, NULL, NULL);

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = _open (cp_filename, flags, mode);
		save_errno = errno;
		g_free (cp_filename);
	}

	errno = save_errno;
	return retval;
}

int  ug_fd_truncate (int fd, gint64 length)
{
	HANDLE h_file;

	if (ug_fd_seek (fd, length, SEEK_SET) == 0) {
		h_file = (HANDLE)_get_osfhandle(fd);
		if (SetEndOfFile (h_file))
			return 0;
	}
	return -1;
}

#if !defined(__MINGW32__)
FILE* ug_fopen (const gchar *filename, const gchar *mode)
{
	FILE *retval;
	int save_errno;

	if (G_WIN32_HAVE_WIDECHAR_API ()) {
		wchar_t *wfilename = g_utf8_to_utf16 (filename, -1, NULL, NULL, NULL);
		wchar_t *wmode;

		if (wfilename == NULL) {
			errno = EINVAL;
			return NULL;
		}

		wmode = g_utf8_to_utf16 (mode, -1, NULL, NULL, NULL);

		if (wmode == NULL) {
			g_free (wfilename);
			errno = EINVAL;
			return NULL;
		}

		retval = _wfopen (wfilename, wmode);
		save_errno = errno;
		g_free (wfilename);
		g_free (wmode);
	}
	else {
		gchar *cp_filename = g_locale_from_utf8 (filename, -1, NULL, NULL, NULL);

		if (cp_filename == NULL) {
			errno = EINVAL;
			return NULL;
		}

		retval = fopen (cp_filename, mode);
		save_errno = errno;
		g_free (cp_filename);
	}
	errno = save_errno;
	return retval;
}
#endif    // End of !defined(__MINGW32__)

#else     // UNIX
int  ug_fd_open (const gchar* filename_utf8, int flags, int mode)
{
	if (g_get_filename_charsets (NULL))
		return g_open (filename_utf8, flags, mode);
	else {
		gchar *cp_filename = g_filename_from_utf8 (filename_utf8, -1, NULL, NULL, NULL);
		int retval;
		int save_errno;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = g_open (cp_filename, flags, mode);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}

FILE* ug_fopen (const gchar *filename, const gchar *mode)
{
	if (g_get_filename_charsets (NULL))
		return g_fopen (filename, mode);
	else {
		gchar *cp_filename = g_filename_from_utf8 (filename, -1, NULL, NULL, NULL);
		FILE *retval;
		int save_errno;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return NULL;
		}

		retval = g_fopen (cp_filename, mode);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}

#endif    // End of _WIN32

int  ug_fs_truncate (FILE* file, gint64 size)
{
	int    fd;

	ug_fs_flush (file);
	ug_fs_seek (file, size, SEEK_SET);
	fd = ug_fs_get_fd (file);
	return ug_fd_truncate (fd, size);
}

