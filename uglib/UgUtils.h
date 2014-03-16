/*
 *
 *   Copyright (C) 2005-2014 by C.H. Huang
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

#ifndef UG_UTILS_H
#define UG_UTILS_H

#include <glib.h>
#include <glib/gstdio.h>

#ifdef __cplusplus
extern "C" {
#endif


// ------------------------------------------------------------------
// file & directory functions
// All files and directories name are UTF-8 encoding.
// all functions returns 0 if the directory was successfully, -1 if an error occurred

#ifdef _WIN32
#  define ug_rename             g_rename
#  define ug_unlink             g_unlink
#  define ug_create_dir(dir)    g_mkdir(dir,0755)
#  define ug_delete_dir         g_rmdir
#endif

#ifndef _WIN32
int  ug_rename (const gchar *old_file_utf8, const gchar *new_file_utf8);
int  ug_unlink (const gchar *file_utf8);
int  ug_create_dir (const gchar *dir_utf8);
int  ug_delete_dir (const gchar *dir_utf8);
#endif

#define ug_rename_file        ug_rename
#define ug_delete_file        ug_unlink

int		ug_copy_file (const gchar *src_file_utf8, const gchar *new_file_utf8);

// Create or check all elements of the specified directories.
// Return  0 if all elements of directory was exist or created successfully.
// Return -1 if error occurred.
// dir : use UTF-8 encoding.
// len : length of dir. if len == -1, dir is null-terminated string.
int		ug_create_dir_all (const gchar *dir_utf8, gint len);
// Remove a File or Directory Recursively
int		ug_delete_dir_recursive (const gchar *dir_utf8);
// Change the modified time of file
int		ug_modify_file_time (const gchar *file_utf8, time_t mod_time);


// ------------------------------------------------------------------
// URI list functions
// To get URIs from text file, error is G_FILE_ERROR.
GList*	ug_text_file_get_uris (const gchar* file_utf8, GError** error);
// get URIs from text
GList*	ug_text_get_uris (const gchar* text, gint text_len);
// remove URIs from list by scheme
GList*	ug_uri_list_remove_scheme (GList* uris, const gchar* scheme);


// ------------------------------------------------------------------
// check BOM in file header and set it's encoding.
// return encoding string.
const char*	ug_io_channel_decide_encoding (GIOChannel* channel);


// ------------------------------------------------------------------
// Power Management

void		ug_reboot (void);
void		ug_shutdown (void);
void		ug_suspend (void);
void		ug_hibernate (void);

// ------------------------------------------------------------------
// others
//
gboolean	ug_launch_uri (const gchar* uri);
gboolean	ug_launch_default_app (const gchar* folder, const gchar* file);
gchar*		ug_sys_release (void);

#ifdef __cplusplus
}
#endif

#endif  // UG_UTILS_H

