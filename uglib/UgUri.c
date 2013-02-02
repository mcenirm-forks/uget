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

#include <string.h>
#include <stdlib.h>
#include <UgUri.h>

#include <string.h>
#include <stdlib.h>
#include <UgUri.h>

// ----------------------------------------------------------------------------
// UgUri

int  ug_uri_init (UgUri* upart, const char* uri)
{
	const char* cur;
	const char* tmp;

	upart->uri = uri;
	// scheme
	cur = strpbrk (uri, ":/?#");    // make sure ':' before '/', '?', and '#'
	if (cur && cur[0] == ':') {
		upart->schemeLength = cur - uri;
		cur++;
	}
	else {
		upart->schemeLength = 0;
		cur = uri;
	}

	// authority & path
	if (upart->schemeLength && cur[0] == '/' && cur[1] == '/') {
		cur += 2;
		upart->authority = cur - uri;
		cur += strcspn (cur, "/");
	}
	else
		upart->authority = -1;
	upart->path = cur - uri;

	// file
	upart->file = -1;
	if (cur[0]) {
		for (; ; ) {
			tmp = strpbrk (cur, "/?#");
			if (tmp == NULL || tmp[0] != '/') {
				upart->file = cur - uri;
				break;
			}
			cur = tmp + 1;
		}
	}

	// query
	if ((tmp = strchr (cur, '?')) == NULL)
		upart->query = -1;
	else {
		cur = tmp + 1;
		upart->query = cur - uri;
	}

	// fragment
	if ((tmp = strrchr (cur, '#')) == NULL)
		upart->fragment = -1;
	else {
		cur = tmp + 1;
		upart->fragment = cur - uri;
	}

	// host & port
	upart->port = -1;
	if (upart->authority == -1)
		upart->host = -1;
	else {
		upart->host = upart->authority;
		tmp = uri + upart->authority;
		for (cur = uri + upart->path -1;  cur >= tmp;  cur--) {
			if (cur[0] == '@') {
				upart->host = cur - uri + 1;
				break;
			}
			if (cur[0] == ':')
				upart->port = cur - uri + 1;
		}
	}

	return upart->schemeLength;
}

int  ug_uri_part_scheme (UgUri* uuri, const char** scheme)
{
	if (scheme && uuri->schemeLength)
		*scheme = uuri->uri;
	return uuri->schemeLength;
}

int  ug_uri_part_file (UgUri* uuri, const char** file)
{
	if (uuri->file != -1) {
		if (file)
			*file = uuri->uri + uuri->file;
		if (uuri->query != -1)
			return uuri->query - uuri->file - 1;   // - '?'
		if (uuri->fragment != -1)
			return uuri->fragment - uuri->file - 1;  // - '#'
		return strlen (uuri->uri + uuri->file);
	}
	return 0;
}

int  ug_uri_part_file_ext (UgUri* uuri, const char** ext)
{
	const char* beg;
	const char* end;
	int  len;

	if (uuri->file != -1) {
		len = ug_uri_part_file (uuri, &beg);
		end = uuri->uri + uuri->file + len -1;
		for (;  end >= beg;  end--) {
			if (end[0] == '.') {
				end += 1;	// + '.'
				if (ext)
					*ext = end;
				return len - (end - beg);
			}
		}
	}
	return 0;
}

int  ug_uri_part_query (UgUri* uuri, const char** query)
{
	if (uuri->query != -1) {
		if (query)
			*query = uuri->uri + uuri->query;
		if (uuri->fragment != -1)
			return uuri->fragment - uuri->query -1;  // - '#'
		return strlen (uuri->uri + uuri->query);
	}
	return 0;
}

int  ug_uri_part_fragment (UgUri* uuri, const char** fragment)
{
	if (uuri->fragment != -1) {
		if (fragment)
			*fragment = uuri->uri + uuri->fragment;
		return strlen (uuri->uri + uuri->fragment);
	}
	return 0;
}

int  ug_uri_part_referrer (UgUri* uuri, const char** referrer)
{
	if (referrer)
		*referrer = uuri->uri;
	if (uuri->file == -1)
		return uuri->path;
	return uuri->file;
}

int  ug_uri_part_user (UgUri* uuri, const char** user)
{
	const char* beg;
	const char* end;

	if (uuri->authority == uuri->host)
		return 0;

	beg = uuri->uri + uuri->authority;
	end = uuri->uri + uuri->host - 1;    // - '@'
	if (user)
		*user = beg;
	for (; beg < end;  beg++) {
		if (beg[0] == ':')
			break;
	}
	return beg - uuri->uri - uuri->authority;
}

int  ug_uri_part_password (UgUri* uuri, const char** password)
{
	const char* tmp;
	int  length;

	length = ug_uri_part_user (uuri, &tmp);
	if (length && tmp[length] == ':') {
		tmp += length + 1;  // + ':'
		if (password)
			*password = tmp;
		return uuri->host - (tmp - uuri->uri) - 1;  // - '@'
	}
	return 0;
}

int  ug_uri_part_host (UgUri* uuri, const char** host)
{
	if (uuri->host != -1) {
		if (host)
			*host = uuri->uri + uuri->host;
		if (uuri->port != -1)
			return uuri->port - uuri->host - 1;   // - ':'
		else
			return uuri->path - uuri->host;
	}
	return 0;
}

int  ug_uri_part_port (UgUri* uuri, const char** port)
{
	if (uuri->port != -1) {
		if (port)
			*port = uuri->uri + uuri->port;
		return uuri->path - uuri->port;
	}
	return 0;
}

int  ug_uri_get_port (UgUri* uuri)
{
	if (uuri->port != -1)
		return strtol (uuri->uri + uuri->port, NULL, 10);
	return -1;
}

// --------------------------------------------------------
// Convenience Functions for UgUri
gchar*	ug_uri_get_file (UgUri* upart)
{
	const char*	str;
	char*		name;
	int			len;

	if ((len = ug_uri_part_file (upart, &str)) == 0)
		return NULL;
	name = g_uri_unescape_segment (str, str+len, NULL);
	if (name == NULL)
		name = g_strndup (str, len);
	// check encoding
	if (g_utf8_validate (name, -1, NULL) == FALSE) {
		str = g_locale_to_utf8 (name, -1, NULL, NULL, NULL);
		if (str == NULL) {
			str = g_uri_escape_string (name,
					G_URI_RESERVED_CHARS_ALLOWED_IN_PATH_ELEMENT, FALSE);
		}
		g_free (name);
		return (gchar*) str;
	}

	return name;
}

// --------------------------------------------------------
// Convenience Functions for UgUri
gchar*	ug_uri_get_user (UgUri* upart)
{
	const char*	str = NULL;
	int			len;

	if ((len = ug_uri_part_user (upart, &str)) != 0)
		return g_strndup (str, len);
	return NULL;
}

gchar*	ug_uri_get_password (UgUri* upart)
{
	const char*	str;
	int			len;

	if ((len = ug_uri_part_password (upart, &str)) != 0)
		return g_strndup (str, len);
	return NULL;
}


// ----------------------------------------------------------------------------
// Other URI functions

guint	ug_uri_scheme_len (const gchar* uri)
{
	gchar*		scheme;
	guint		scheme_len;

	scheme = g_uri_parse_scheme (uri);
	if (scheme) {
		scheme_len = strlen (scheme);
		// This check Windows/DOS path like "C:\\"
		// It doesn't need to check length of uri in this case.
		if (scheme_len == 1 && scheme[scheme_len + 2] == '\\')
			scheme_len = 0;
		g_free (scheme);
		return scheme_len;
	}
	return 0;
}

// get length of referrer
guint	ug_uri_referrer_len (const gchar* uri)
{
	UgUri	upart;

	ug_uri_init (&upart, uri);
	return ug_uri_part_referrer ((UgUri*) &upart, NULL);
}

gchar*	ug_uri_get_filename (const gchar* str)
{
	UgUri	upart;

	ug_uri_init (&upart, str);
	return ug_uri_get_file ((UgUri*) &upart);
}

