/*
 *
 *   Copyright (C) 2005-2011 by C.H. Huang
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

#ifndef UG_URI_H
#define UG_URI_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Request for Comments: 3986  (RFC 3986)
 * Uniform Resource Identifier (URI):
 *
 *         uri://example.org:8080/road/where?name=ferret#nose
 *         \_/   \______________/\_________/ \_________/ \__/
 *          |           |            |            |        |
 *       scheme     authority       path        query   fragment
 *          |   _____________________|__
 *         / \ /                        \
 *         urn:example:animal:ferret:nose
 *
 *         uri://user:password@host:port/path/file.ext?query#fragment
 *         urn:path
 */

// UgUriPart
// |
// +- UgUriFull
//
typedef struct UgUriPart		UgUriPart;
typedef struct UgUriFull		UgUriFull;

#define	UG_URI_PART_MEMBERS		\
	const char*		authority;	\
	const char*		path;		\
	const char*		query;		\
	const char*		fragment

// ----------------------------------------------------------------------------
// UgUriPart: UgUriPart use smaller stack size than UgUriFull.
//            It can NOT parse user, password, host, and port in authority.
//
struct UgUriPart
{
	UG_URI_PART_MEMBERS;
//	const char*		authority;
//	const char*		path;
//	const char*		query;
//	const char*		fragment;
};

// ug_uri_part_init() return length of scheme. upart can be NULL.
int		ug_uri_part_init     (UgUriPart* upart, const char*  uri);
int		ug_uri_part_file     (UgUriPart* upart, const char** file);
int		ug_uri_part_file_ext (UgUriPart* upart, const char** ext);
int		ug_uri_part_query    (UgUriPart* upart, const char** query);
int		ug_uri_part_fragment (UgUriPart* upart, const char** fragment);
int		ug_uri_part_referrer (UgUriPart* upart, const char*  uri);

// ----------------------------------------------------------------------------
// UgUriFull: UgUriFull is based on UgUriPart. It can parse authority.
//
struct UgUriFull
{
	UG_URI_PART_MEMBERS;
//	const char*		authority;
//	const char*		path;
//	const char*		query;
//	const char*		fragment;

	const char*		host;
	const char*		port;
};

// ug_uri_full_init() return length of scheme
int		ug_uri_full_init     (UgUriFull* ufull, const char*  uri);
int		ug_uri_full_user     (UgUriFull* ufull, const char** user);
int		ug_uri_full_password (UgUriFull* ufull, const char** password);
int		ug_uri_full_host     (UgUriFull* ufull, const char** host);
int		ug_uri_full_port     (UgUriFull* ufull, const char** port);
int		ug_uri_full_port_n   (UgUriFull* ufull);

#define	ug_uri_full_file(urifull, file)			ug_uri_part_file ((UgUriPart*) urifull, file)
#define	ug_uri_full_file_ext(urifull, ext)		ug_uri_part_file_ext ((UgUriPart*) urifull, ext)
#define	ug_uri_full_query(urifull, query)		ug_uri_part_query ((UgUriPart*) urifull, query)
#define	ug_uri_full_fragment(urifull, fragment)	ug_uri_part_fragment ((UgUriPart*) urifull, fragment)
#define	ug_uri_full_referrer(urifull, uri)		ug_uri_part_referrer ((UgUriPart*) urifull, uri)

// --------------------------------------------------------
// Convenience Functions for UgUriPart
//  If filename is valid UTF-8, return unescaped filename,
//  otherwise return escaped filename.
gchar*	ug_uri_part_get_file (UgUriPart* upart);
#define	ug_uri_full_get_file(urifull)			ug_uri_part_get_file ((UgUriPart*) urifull)

// --------------------------------------------------------
// Convenience Functions for UgUriFull
gchar*	ug_uri_full_get_user (UgUriFull* ufull);
gchar*	ug_uri_full_get_password (UgUriFull* ufull);

// --------------------------------------------------------
// Other URI functions
guint	ug_uri_scheme_len (const gchar* uri);
guint	ug_uri_referrer_len (const gchar* uri);
gchar*	ug_uri_get_filename (const gchar* uri);


#ifdef __cplusplus
}
#endif

#endif	// UG_URI_H

