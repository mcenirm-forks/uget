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

#include <string.h>
#include <UgString.h>

// ----------------------------------------------------------------------------
// string functions

/*  This function works like strcspn ()
 *  If string_len == -1, string is null-terminated.
 *  It return the index of the first character in string that is in charset.
 *  If none of the characters in string is in charset, then the return value is the length of string.
 */
unsigned int ug_str_find_charset (const char* string, int string_len, unsigned int offset, const char* charset)
{
	const char* string_end;
	const char* string_cur;
	const char* ch;

	if (string_len == -1)
		string_len = (int) strlen (string);
	string_end = string + string_len;

	for (string_cur = string + offset;  string_cur < string_end;  string_cur++) {
		for (ch = charset;  *ch;  ch++) {
			if (*string_cur == *ch)
				return (unsigned int) (string_cur - string);
		}
	}

	return string_len;
}

/*
 *  This function try to skip specified character from offset in string.
 *  If found other than specified character, return current offset.
 *  Otherwise return original offset.
 */
unsigned int ug_str_skip_charset (const char* string, int string_len, unsigned int offset, const char* charset)
{
	const char* string_cur;
	const char* string_end;
	const char* ch;

	if (string_len == -1)
		string_len = (int) strlen (string);
	string_end = string + string_len;

	for (string_cur = string + offset;  string_cur < string_end;  string_cur++) {
		for (ch = charset;  ;  ch++) {
			if (*ch == 0)
				goto exit;
			if (*string_cur == *ch)
				break;
		}
	}

exit:
	return (unsigned int) (string_cur - string);
}

/*
 *  This function try to clear specified character from tail of string. (set character to '\0')
 *  return new string_len.
 */
unsigned int ug_str_clear_tail_charset (char* string, int string_len, const char* charset)
{
	const char*	ch;
	char*		string_cur;

	if (string_len == -1)
		string_len = (int) strlen (string);

	for (string_cur = string +string_len -1;  string_cur >= string;  string_cur--) {
		for (ch = charset;  ;  ch++) {
			if (*ch == 0)
				goto exit;
			if (*string_cur == *ch) {
				*string_cur = 0;
				break;
			}
		}
	}

exit:
	return (unsigned int) (string_cur - string + 1);
}

/*
 *  count line length until character '\r' , '\n' ,or end of string from offset in string.
 *  return : length of line exclude line terminator.
 */
unsigned int ug_str_line_len (const char* string, int string_len, unsigned int offset)
{
	unsigned int retval;

	retval = ug_str_find_charset(string, string_len, offset, "\r\n");
	if (retval > offset)
		return retval - offset;
	return 0;
}

/*
 *  return : offset of next line in string.
 *  return string_len if no next line.
 */
unsigned int ug_str_line_next (const char* string, int string_len, unsigned int offset)
{
	const char* string_end;
	const char* string_cur;

	if (string_len == -1)
		string_len = (int) strlen (string);
	string_end = string + string_len;

	for (string_cur = string + offset;  string_cur < string_end;  string_cur++) {
		if (*string_cur == '\n')
			return (unsigned int) (string_cur - string) + 1;
	}

	return string_len;
}

/*
 * It free original string that point by string_pointer and copy a new src string to string_pointer.
 * If src_len is -1, src string is null-terminated.
 * If src is NULL, *string_pointer will set NULL.
 *
 *  char* temp_str = strdup ("123");
 *
 *  ug_str_set (&temp_str, "45678", -1);		// temp_str will set "45678"
 */
void	ug_str_set (char** string_pointer, const char* src, int src_len)
{
	g_free (*string_pointer);

	if (src == NULL || *src == 0) {
		*string_pointer = NULL;
		return;
	}

	if (src_len == -1)
		*string_pointer = g_strdup (src);
	else
		*string_pointer = g_strndup (src, src_len);
}

/*
 * convert double to string
 * If value large than 1024, it will append unit string like "KiB", "MiB", "GiB", "TiB", or "PiB"  to string.
 */
gchar*	ug_str_dtoa_unit (gdouble value, gint precision, const gchar* tail)
{
	static const gchar*	unit_array[] = {"", " KiB", " MiB", " GiB", " TiB", " PiB"};
	static const guint	unit_array_len = sizeof (unit_array) / sizeof (gchar*);
	guint	index;

	for (index=0;  index < unit_array_len -1;  index++) {
		if (value < 1024.0)
			break;
		value /= 1024.0;
	}
	if (index == 0)
		precision = 0;

	return g_strdup_printf ("%.*f%s%s",
			precision, value, unit_array[index], (tail) ? tail : "");
}

/*
 * convert seconds to string (hh:mm:ss)
 */
gchar*	ug_str_from_seconds (guint seconds, gboolean limit_99_99_99)
{
	guint		hour, minute;
	guint		day, year;

	minute  = seconds / 60;
	hour    = minute / 60;
	minute  = minute % 60;
	seconds = seconds % 60;

	if (hour < 100)
		return g_strdup_printf ("%.2u:%.2u:%.2u", hour, minute, seconds);
	else if (limit_99_99_99)
		return g_strdup ("99:99:99");
	else {
		day    = hour / 24;
		year   = day / 365;
		if (year)
			return g_strdup ("> 1 year");
		else
			return g_strdup_printf ("%u days", day);
	}
}

/*
 * convert time_t to string
 */
gchar*	ug_str_from_time (time_t ptt, gboolean date_only)
{
	GString*	gstr;
	struct tm*	timem;

	timem = localtime (&ptt);
	if (date_only)
		gstr = g_string_sized_new (11);
	else
		gstr = g_string_sized_new (11 + 9);
	g_string_append_printf (gstr, "%.4d-%.2d-%.2d",
			timem->tm_year + 1900,
			timem->tm_mon  + 1,
			timem->tm_mday);
	if (date_only == FALSE) {
		g_string_append_printf (gstr, " %.2d:%.2d:%.2d",
				timem->tm_hour,
				timem->tm_min,
				timem->tm_sec);
	}

	return g_string_free (gstr, FALSE);
}

