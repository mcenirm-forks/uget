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

#ifndef UG_STRING_H
#define UG_STRING_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

// ------------------------------------------------------------------
// string functions

/*  This function works like strcspn ()
 *  If string_len == -1, string is null-terminated.
 *  It return the index of the first character in string that is in charset.
 *  If none of the characters in string is in charset, then the return value is the length of string.
 */
unsigned int ug_str_find_charset (const char* string, int string_len, unsigned int offset, const char* charset);

/*
 *  This function try to skip specified character from offset in string.
 *  If found other than specified character, return current offset.
 *  Otherwise return original offset.
 */
unsigned int ug_str_skip_charset (const char* string, int string_len, unsigned int offset, const char* charset);

/*
 *  This function try to clear specified character from tail of string. (set character to '\0')
 *  return new string_len.
 */
unsigned int ug_str_clear_tail_charset (char* string, int string_len, const char* charset);

/*
 *  count line length until character '\r' , '\n' ,or end of string from offset in string.
 *  return : length of line exclude line terminator.
 */
unsigned int ug_str_line_len (const char* string, int string_len, unsigned int offset);

/*
 *  return : offset of next line in string.
 *  return string_len if no next line.
 */
unsigned int ug_str_line_next (const char* string, int string_len, unsigned int offset);

/*
 * It free original string that point by string_pointer and copy a new src string to string_pointer.
 * If src_len is -1, src string is null-terminated.
 * If src is NULL, *string_pointer will set NULL.
 *
 *  char* temp_str = strdup ("123");
 *
 *  ug_str_set (&temp_str, "45678", -1);		// temp_str will set "45678"
 */
void	ug_str_set (char** string_pointer, const char* src, int src_len);

/*
 * convert double to string
 * If value large than 1024, it will append unit string like "KiB", "MiB", "GiB", "TiB", or "PiB"  to string.
 */
gchar*	ug_str_dtoa_unit (gdouble value, gint precision, const gchar* tail);

/*
 * convert seconds to string (hh:mm:ss)
 */
gchar*	ug_str_from_seconds (guint seconds, gboolean limit_99_99_99);

/*
 * convert time_t to string
 */
gchar*	ug_str_from_time (time_t ptt, gboolean date_only);


#ifdef __cplusplus
}
#endif

#endif  // UG_STRING_H

