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

#include <string.h>
#include <stdlib.h>
// uglib
#include <UgUri.h>

// ----------------------------------------------------------------------------
// UgUriPart
//
int	ug_uri_part_init (UgUriPart* upart, const char* uri)
{
	const char* cur;
	int			len;	// length of scheme

	// scheme
	cur = strpbrk (uri, ":/?#");
	if (cur && *cur==':') {
		len = cur - uri;
		cur += 1;
	}
	else {
		len = 0;
		cur = uri;
	}

	if (upart) {
		// authority & path
		if (cur[0] == '/' && cur[1] == '/') {
			cur += 2;
			upart->authority = cur;
			cur += strcspn (cur, "/");
			upart->path = cur;
		}
		else {
			upart->authority = NULL;
			upart->path = cur;
		}
		// query & fragment
		if ((upart->query = strchr (cur, '?')) != NULL) {
			upart->query++;
			upart->fragment = strrchr (upart->query, '#');
		}
		else
			upart->fragment = strrchr (cur, '#');
		if (upart->fragment)
			upart->fragment++;
	}
	return len;
}

int	ug_uri_part_file (UgUriPart* upart, const char** file)
{
	const char*	cur;
	const char*	beg;
	const char*	end;

	// range [beg, end)
	beg = upart->path;
	if (upart->query)
		end = upart->query - 1;			// - '?'
	else if (upart->fragment)
		end = upart->fragment - 1;		// - '#'
	else
		end = beg + strlen (beg);
	// get last path element
	for (cur = end -1;  cur >= beg;  cur--) {
		if (*cur == '/') {
			cur += 1;
			if (file)
				*file = cur;
			return end - cur;
		}
	}
	if (file)
		*file = beg;
	return end - beg;
}

int	ug_uri_part_file_ext (UgUriPart* upart, const char** ext)
{
	const char*	beg;
	const char*	end;
	int			len;

	// get filename
	if ((len = ug_uri_part_file (upart, &beg)) != 0) {
		for (end = beg +len -1;  end >= beg;  end--) {
			if (*end == '.') {
				end += 1;	// + '.'
				if (ext)
					*ext = end;
				return len - (end - beg);
			}
		}
	}
	return 0;
}

int	ug_uri_part_query (UgUriPart* upart, const char** query)
{
	const char*	beg;

	if ((beg = upart->query) != NULL) {
		if (query)
			*query = beg;
		if (upart->fragment)
			return upart->fragment - beg - 1;	// - '#'
		return strlen (beg);
	}
	return 0;
}

int	ug_uri_part_fragment (UgUriPart* upart, const char** fragment)
{
	const char*	beg;

	if ((beg = upart->fragment) != NULL) {
		if (fragment)
			*fragment = beg;
		return strlen (beg);
	}
	return 0;
}

int	ug_uri_part_referrer (UgUriPart* upart, const char* uri)
{
	const char*	end;

	if (ug_uri_part_file (upart, &end) == 0)
		end = upart->path;
	return end - uri;
}

// --------------------------------------------------------
// UgUriFull
//
int		ug_uri_full_init (UgUriFull* ufull, const char*  uri)
{
	const char*	beg;
	const char*	cur;
	int			len;

	ufull->host = NULL;
	ufull->port = NULL;
	len = ug_uri_part_init ((UgUriPart*) ufull, uri);
	if ((beg = ufull->authority) != NULL) {
		ufull->host = beg;
		for (cur = ufull->path -1;  cur >= beg;  cur--) {
			if (*cur == '@') {
				ufull->host = cur + 1;
				break;
			}
			if (*cur == ':')
				ufull->port = cur + 1;
		}
	}
	return len;
}

int	ug_uri_full_user (UgUriFull* ufull, const char** user)
{
	const char*	beg;
	const char*	end;

	beg = ufull->authority;
	end = ufull->host;
	if (beg != end) {
		if (user)
			*user = beg;
		for (end -= 1;  beg < end;  beg++) {
			if (*beg == ':')
				return beg - ufull->authority;
		}
	}
	return end - ufull->authority;
}

int	ug_uri_full_password (UgUriFull* ufull, const char** password)
{
	const char*	beg;
	const char*	end;

	beg = ufull->authority;
	end = ufull->host;
	if (beg != end) {
		for (end -= 1;  beg < end;  beg++) {
			if (*beg == ':') {
				beg += 1;	// + ':'
				if (password)
					*password = beg;
				return end - beg;
			}
		}
	}
	return 0;
}

int	ug_uri_full_host (UgUriFull* ufull, const char** host)
{
	const char*	beg;

	if ((beg = ufull->host) != NULL) {
		if (host)
			*host = beg;
		if (ufull->port)
			return ufull->port - beg - 1;	// - ':'
		else
			return ufull->path - beg;
	}
	return 0;
}

int	ug_uri_full_port (UgUriFull* ufull, const char** port)
{
	const char*	beg;

	if ((beg = ufull->port) != NULL) {
		if (port)
			*port = beg;
		return ufull->path - beg;
	}
	return 0;
}

int	ug_uri_full_port_n (UgUriFull* ufull)
{
	if (ufull->port)
		return atoi (ufull->port);
	return -1;
}

// --------------------------------------------------------
// Convenience Functions for UgUriPart
gchar*	ug_uri_part_get_file (UgUriPart* upart)
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
// Convenience Functions for UgUriFull
gchar*	ug_uri_full_get_user (UgUriFull* ufull)
{
	const char*	str = NULL;
	int			len;

	if ((len = ug_uri_full_user (ufull, &str)) != 0)
		return g_strndup (str, len);
	return NULL;
}

gchar*	ug_uri_full_get_password (UgUriFull* ufull)
{
	const char*	str;
	int			len;

	if ((len = ug_uri_full_password (ufull, &str)) != 0)
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
	UgUriPart	upart;

	ug_uri_part_init (&upart, uri);
	return ug_uri_part_referrer ((UgUriPart*) &upart, uri);
}

gchar*	ug_uri_get_filename (const gchar* str)
{
	UgUriPart	upart;

	ug_uri_part_init (&upart, str);
	return ug_uri_part_get_file ((UgUriPart*) &upart);
}

