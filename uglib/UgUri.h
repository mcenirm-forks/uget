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

#ifndef UG_URI_H
#define UG_URI_H

#include <stdint.h>
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

typedef struct UgUri              UgUri;

#define	UG_URI_MEMBERS  \
	const char*  uri;       \
	int16_t  schemeLength;  \
	int16_t  authority;     \
	int16_t  host;          \
	int16_t  port;          \
	int16_t  path;          \
	int16_t  file;          \
	int16_t  query;         \
	int16_t  fragment

struct UgUri
{
	UG_URI_MEMBERS;
//	const char*  uri;
//	int16_t  schemeLength;
//	int16_t  authority;
//	int16_t  host;
//	int16_t  port;
//	int16_t  path;
//	int16_t  file;
//	int16_t  query;
//	int16_t  fragment;
};

int  ug_uri_init (UgUri* uuri, const char* uri);
int  ug_uri_part_scheme   (UgUri* uuri, const char** scheme);
int  ug_uri_part_file     (UgUri* uuri, const char** file);
int  ug_uri_part_file_ext (UgUri* uuri, const char** ext);
int  ug_uri_part_query    (UgUri* uuri, const char** query);
int  ug_uri_part_fragment (UgUri* uuri, const char** fragment);
int  ug_uri_part_referrer (UgUri* uuri, const char** referrer);
int  ug_uri_part_user     (UgUri* uuri, const char** user);
int  ug_uri_part_password (UgUri* uuri, const char** password);
int  ug_uri_part_host     (UgUri* uuri, const char** host);
int  ug_uri_part_port     (UgUri* uuri, const char** port);

#define  ug_uri_scheme        ug_uri_part_scheme
#define  ug_uri_file          ug_uri_part_file
#define  ug_uri_file_ext      ug_uri_part_file_ext
#define  ug_uri_query         ug_uri_part_query
#define  ug_uri_fragment      ug_uri_part_fragment
#define  ug_uri_referrer      ug_uri_part_referrer
#define  ug_uri_user          ug_uri_part_user
#define  ug_uri_password      ug_uri_part_password
#define  ug_uri_host          ug_uri_part_host
#define  ug_uri_port          ug_uri_part_port

int  ug_uri_get_port (UgUri* uuri);

// --------------------------------------------------------
// Convenience Functions for UgUri
//  If filename is valid UTF-8, return unescaped filename,
//  otherwise return escaped filename.
gchar*	ug_uri_get_file (UgUri* upart);

// --------------------------------------------------------
// Convenience Functions for UgUri
gchar*	ug_uri_get_user (UgUri* ufull);
gchar*	ug_uri_get_password (UgUri* ufull);

// --------------------------------------------------------
// Other URI functions
guint	ug_uri_scheme_len (const gchar* uri);
guint	ug_uri_referrer_len (const gchar* uri);
gchar*	ug_uri_get_filename (const gchar* uri);


#ifdef __cplusplus
}
#endif

#endif	// UG_URI_H

