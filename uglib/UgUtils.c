/*
 *
 *   Copyright (C) 2005-2011 by plushuang
 *   plushuang at users.sourceforge.net
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

#include <errno.h>
#include <string.h>
#include <memory.h>

#ifdef _WIN32
#include <windows.h>
#include <sys/utime.h>	// struct utimbuf
#else
#include <utime.h>		// struct utimbuf
#endif

#include <gio/gio.h>
// uglib
#include <UgUri.h>
#include <UgUtils.h>
#include <UgString.h>
#include <UgStdio.h>


// ------------------------------------------------------------------
// file & directory functions
//
#ifndef _WIN32
int  ug_rename (const gchar *old_filename, const gchar *new_filename)
{
	if (g_get_filename_charsets (NULL))
		return g_rename (old_filename, new_filename);
	else {
		gchar *cp_old_filename = g_filename_from_utf8 (old_filename, -1, NULL, NULL, NULL);
		gchar *cp_new_filename = g_filename_from_utf8 (new_filename, -1, NULL, NULL, NULL);
		int save_errno;
		int retval;

		if (cp_old_filename == NULL || cp_new_filename == NULL) {
			g_free (cp_old_filename);
			g_free (cp_new_filename);
			errno = EINVAL;
			return -1;
		}

		retval = g_rename (cp_old_filename, cp_new_filename);
		save_errno = errno;

		g_free (cp_old_filename);
		g_free (cp_new_filename);

		errno = save_errno;
		return retval;
	}
}

int  ug_unlink (const gchar *filename)
{
	if (g_get_filename_charsets (NULL))
		return g_unlink (filename);
	else {
		gchar *cp_filename = g_filename_from_utf8 (filename, -1, NULL, NULL, NULL);
		int save_errno;
		int retval;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = g_unlink (cp_filename);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}

int  ug_create_dir (const gchar *dir_utf8)
{
	if (g_get_filename_charsets (NULL))
		return g_mkdir (dir_utf8, 0755);
	else {
		gchar *cp_filename = g_filename_from_utf8 (dir_utf8, -1, NULL, NULL, NULL);
		int save_errno;
		int retval;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = g_mkdir (cp_filename, 0755);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}

int  ug_delete_dir (const gchar *dir_utf8)
{
	if (g_get_filename_charsets (NULL))
		return g_rmdir (dir_utf8);
	else {
		gchar *cp_filename = g_filename_from_utf8 (dir_utf8, -1, NULL, NULL, NULL);
		int save_errno;
		int retval;

		if (cp_filename == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = g_rmdir (cp_filename);
		save_errno = errno;

		g_free (cp_filename);

		errno = save_errno;
		return retval;
	}
}
#endif

#ifdef _WIN32
int  ug_copy_file (const gchar *src_file_utf8, const gchar *new_file_utf8)
{
	gboolean	retval;
	gunichar2*	src_file_wcs;
	gunichar2*	new_file_wcs;

	src_file_wcs = g_utf8_to_utf16 (src_file_utf8, -1, NULL, NULL, NULL);
	new_file_wcs = g_utf8_to_utf16 (new_file_utf8, -1, NULL, NULL, NULL);
	retval = CopyFileW (src_file_wcs, new_file_wcs, FALSE);
	g_free (src_file_wcs);
	g_free (new_file_wcs);
	if (retval == 0)
		return -1;
	return 0;
}
#else
int  ug_copy_file (const gchar *src_file_utf8, const gchar *new_file_utf8)
{
	int		src_fd;
	int		new_fd;
	char*	buf;
	int		buf_len;
	int		retval = 0;

//	new_fd = open (new_file_utf8,
//	               O_BINARY | O_WRONLY | O_CREAT,
//	               S_IREAD | S_IWRITE | S_IRGRP | S_IROTH);
	new_fd = ug_fd_open (new_file_utf8,
	                     UG_FD_O_BINARY | UG_FD_O_WRONLY | UG_FD_O_CREAT,
	                     UG_FD_S_IREAD | UG_FD_S_IWRITE | UG_FD_S_IRGRP | UG_FD_S_IROTH);
	if (new_fd == -1)
		return -1;

//	src_fd = open (src_file_utf8, O_BINARY | O_RDONLY, S_IREAD);
	src_fd = ug_fd_open (src_file_utf8, UG_FD_O_BINARY | UG_FD_O_RDONLY, UG_FD_S_IREAD);
	if (src_fd == -1) {
		ug_fd_close (new_fd);
		return -1;
	}
	// read & write
	buf = g_malloc (8192);
	for (;;) {
		buf_len = ug_fd_read (src_fd, buf, 8192);
		if (buf_len <=0)
			break;
		if (ug_fd_write (new_fd, buf, buf_len) != buf_len) {
			retval = -1;
			break;
		}
	}
	// clear
	g_free (buf);
	ug_fd_close (src_fd);
	ug_fd_close (new_fd);
	return retval;
}
#endif	// _WIN32

// This function use complex way to handle directory because some locale encoding doesn't avoid '\\' or '/'.
int  ug_create_dir_all (const gchar* dir, gint len)
{
	const gchar*	dir_end;
	const gchar*	element_end;	// path element
	gchar*			element_os;

	if (len == -1)
		len = strlen (dir);
	dir_end = dir + len;
	element_end = dir;

	for (;;) {
		// skip directory separator "\\\\" or "//"
		for (;  element_end < dir_end;  element_end++) {
			if (*element_end != G_DIR_SEPARATOR)
				break;
		}
		if (element_end == dir_end)
			return 0;
		// get directory name [dir, element_end)
		for (;  element_end < dir_end;  element_end++) {
			if (*element_end == G_DIR_SEPARATOR)
				break;
		}
		// create directory by locale encoding name.
		element_os = g_filename_from_utf8 (dir, element_end - dir, NULL, NULL, NULL);
		if (element_os == NULL)
			break;
		if (g_file_test (element_os, G_FILE_TEST_EXISTS) == FALSE) {
			if (g_mkdir (element_os, 0755) == -1) {
				g_free (element_os);
				break;
			}
		}
		else if (g_file_test (element_os, G_FILE_TEST_IS_DIR) == FALSE) {
			g_free (element_os);
			break;
		}
		g_free (element_os);
	}
	return -1;
}

int  ug_delete_dir_recursive (const gchar *utf8_dir)
{
	GDir*		dir;
	gchar*		name;
	gchar*		path;
	gchar*		locale_dir;
	gboolean	is_dir;

	if (g_get_filename_charsets (NULL)) {
		locale_dir = NULL;
		dir = g_dir_open (utf8_dir, 0, NULL);
	}
	else {
		locale_dir = g_filename_from_utf8 (utf8_dir, -1, NULL, NULL, NULL);
		dir = g_dir_open (locale_dir, 0, NULL);
	}

	if (dir == NULL) {
		g_free (locale_dir);
		return -1;
	}

	for (;;) {
		name = (gchar*) g_dir_read_name (dir);
		if (name == NULL)
			break;
		if (locale_dir == NULL) {
			path = g_build_filename (utf8_dir, name, NULL);
			is_dir = g_file_test (path, G_FILE_TEST_IS_DIR);
		}
		else {
			name = g_build_filename (locale_dir, name, NULL);
			path = g_filename_to_utf8 (name, -1, NULL, NULL, NULL);
			is_dir = g_file_test (name, G_FILE_TEST_IS_DIR);
			g_free (name);
		}
		// delete file or directory
		if (is_dir)
			ug_delete_dir_recursive (path);
		else
			ug_delete_file (path);
		g_free (path);
	}
	g_free (locale_dir);
	g_dir_close (dir);
	ug_delete_dir (utf8_dir);
	return 0;
}

// Change the modified time of file
int	ug_modify_file_time (const gchar *file_utf8, time_t mod_time)
{
	struct utimbuf	utb;
	gchar*			file;
	int				retval;

	utb.actime = time (NULL);
	utb.modtime = mod_time;
	file = g_filename_from_utf8 (file_utf8, -1, NULL, NULL, NULL);
	retval = g_utime (file, &utb);
	g_free (file);

	return retval;
}


// ------------------------------------------------------------------
// URI list functions
// get URIs from text file
GList*	ug_text_file_get_uris (const gchar* file_utf8, GError** error)
{
	GIOChannel*		channel;
	GList*			list;
	gchar*			string;
	gchar*			escaped;
	gsize			line_len;

	string = g_filename_from_utf8 (file_utf8, -1, NULL, NULL, NULL);
	channel = g_io_channel_new_file (string, "r", error);
	g_free (string);
	if (channel == NULL)
		return NULL;
	ug_io_channel_decide_encoding (channel);

	list = NULL;
	while (g_io_channel_read_line (channel, &string, NULL, &line_len, NULL) == G_IO_STATUS_NORMAL) {
		if (string == NULL)
			continue;
		string[line_len] = 0;		// clear '\n' in tail
		// check URI scheme
		if (ug_uri_scheme_len (string) == 0)
			g_free (string);
		else {
			// if URI is not valid UTF-8 string, escape it.
			if (g_utf8_validate (string, -1, NULL) == FALSE) {
				escaped = g_uri_escape_string (string,
						G_URI_RESERVED_CHARS_ALLOWED_IN_PATH, FALSE);
				g_free (string);
				string = escaped;
			}
			list = g_list_prepend (list, string);
		}
	}
	g_io_channel_unref (channel);
	return g_list_reverse (list);
}

// get URIs from text
GList*	ug_text_get_uris (const gchar* text, gint text_len)
{
	GList*		list;
	gchar*		escaped;
	gchar*		line;
	gint		line_len;
	gint		offset;

	if (text_len == -1)
		text_len = strlen (text);

	list = NULL;
	for (offset = 0;  offset < text_len;  offset += line_len +1) {
		line_len = ug_str_line_len (text, text_len, offset);
		line = g_strndup (text + offset, line_len);
		// check URI scheme
		if (ug_uri_scheme_len (line) == 0)
			g_free (line);
		else {
			// if URI is not valid UTF-8 string, escape it.
			if (g_utf8_validate (line, -1, NULL) == FALSE) {
				escaped = g_uri_escape_string (line,
						G_URI_RESERVED_CHARS_ALLOWED_IN_PATH, FALSE);
				g_free (line);
				line = escaped;
			}
			list = g_list_prepend (list, line);
		}
	}

	return g_list_reverse (list);
}

GList*	ug_uri_list_remove_scheme (GList* list, const gchar* scheme)
{
	GList*	link;
	gchar*	text;

	for (link = list;  link;  link = link->next) {
		text = g_uri_parse_scheme (link->data);
		if (text && strcmp (text, scheme) == 0) {
			g_free (link->data);
			link->data = NULL;
		}
		g_free (text);
	}
	return g_list_remove_all (list, NULL);
}


// ------------------------------------------------------------------
// Used by ug_io_channel_decide_encoding()
// BOM = Byte Order Mark
#define UG_BOM_UTF32BE			"\x00\x00\xFE\xFF"
#define UG_BOM_UTF32BE_LEN		4
#define UG_BOM_UTF32LE			"\xFF\xFE\x00\x00"
#define UG_BOM_UTF32LE_LEN		4
#define UG_BOM_UTF8				"\xEF\xBB\xBF"
#define	UG_BOM_UTF8_LEN			3
#define UG_BOM_UTF16BE			"\xFE\xFF"
#define	UG_BOM_UTF16BE_LEN		2
#define UG_BOM_UTF16LE			"\xFF\xFE"
#define	UG_BOM_UTF16LE_LEN		2

const char*	ug_io_channel_decide_encoding (GIOChannel* channel)
{
	gchar*		encoding;
	gchar		bom[4];
	guint		bom_len;

	// The internal encoding is always UTF-8.
	// set encoding NULL is safe to use with binary data.
	g_io_channel_set_encoding (channel, NULL, NULL);
	// read 4 bytes BOM (Byte Order Mark)
	if (g_io_channel_read_chars (channel, bom, 4, NULL, NULL) != G_IO_STATUS_NORMAL)
		return NULL;

	if (memcmp (bom, UG_BOM_UTF32BE, UG_BOM_UTF32BE_LEN) == 0) {
		bom_len = UG_BOM_UTF32BE_LEN;
		encoding = "UTF-32BE";
	}
	else if (memcmp (bom, UG_BOM_UTF32LE, UG_BOM_UTF32LE_LEN) == 0) {
		bom_len = UG_BOM_UTF32LE_LEN;
		encoding = "UTF-32LE";
	}
	else if (memcmp (bom, UG_BOM_UTF8, UG_BOM_UTF8_LEN) == 0) {
		bom_len = UG_BOM_UTF8_LEN;
		encoding = "UTF-8";
	}
	else if (memcmp (bom, UG_BOM_UTF16BE, UG_BOM_UTF16BE_LEN) == 0) {
		bom_len = UG_BOM_UTF16BE_LEN;
		encoding = "UTF-16BE";
	}
	else if (memcmp (bom, UG_BOM_UTF16LE, UG_BOM_UTF16LE_LEN) == 0) {
		bom_len = UG_BOM_UTF16LE_LEN;
		encoding = "UTF-16LE";
	}
	else {
		bom_len = 0;
		encoding = NULL;
//		encoding = "UTF-8";
	}
	// repositioned before set encoding. This flushes all the internal buffers.
	g_io_channel_seek_position (channel, bom_len, G_SEEK_SET, NULL);
	// The encoding can be set now.
	g_io_channel_set_encoding (channel, encoding, NULL);
	return encoding;
}


// ------------------------------------------------------------------
// others
//
#ifdef _WIN32
gboolean	ug_launch_default_app (const gchar* folder, const gchar* file)
{
	gchar*		path;
	gunichar2*	path_wcs;

	if (folder == NULL)
		path = g_build_filename (file, NULL);
	else
		path = g_build_filename (folder, file, NULL);
	if (g_file_test (path, G_FILE_TEST_EXISTS) == FALSE) {
		g_free (path);
		return FALSE;
	}

	// UNICODE
	path_wcs = g_utf8_to_utf16 (path, -1, NULL, NULL, NULL);
	g_free (path);
	ShellExecuteW (NULL, L"open", path_wcs, NULL, NULL, SW_SHOW);
	g_free (path_wcs);

	return TRUE;
}

void	ug_shutdown (void)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if (OpenProcessToken (GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
	{
		LookupPrivilegeValue (NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges (hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
		ExitWindowsEx (EWX_SHUTDOWN | EWX_POWEROFF, 0);
	}

	ExitWindowsEx (EWX_SHUTDOWN | EWX_POWEROFF, 0);
}
#else
gboolean	ug_launch_default_app (const gchar* folder, const gchar* file)
{
	GError* error = NULL;
	GFile*	gfile;
	gchar*	uri;
	gchar*	path;
	gchar*	path_wcs;

	path = g_build_filename (folder, file, NULL);
	path_wcs = g_filename_from_utf8 (path, -1, NULL, NULL, NULL);
	g_free (path);
	if (g_file_test (path_wcs, G_FILE_TEST_EXISTS) == FALSE) {
		g_free (path_wcs);
		return FALSE;
	}

	gfile = g_file_new_for_path (path_wcs);
	g_free (path_wcs);
	uri = g_file_get_uri (gfile);
	g_object_unref (gfile);
	g_app_info_launch_default_for_uri (uri, NULL, &error);
	g_free (uri);

	if (error) {
#ifndef NDEBUG
		g_print ("%s", error->message);
#endif
		g_error_free (error);
	}

	return TRUE;
}

void	ug_shutdown (void)
{
	g_spawn_command_line_async ("poweroff", NULL);
//	g_spawn_command_line_async ("shutdown -h -P now", NULL);
	// change to runlevel 0
}
#endif

