/*
 *
 *   Copyright (C) 2005-2011 by Raymond Huang
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

/*
 *	XML-RPC for C Language
 *
 *	UgXmlrpc		xmlrpc;
 *	UgXmlrpcValue*	value;
 *
 *	ug_xmlrpc_init (&xmlrpc);
 *	value = ug_xmlrpc_value_new ();
 *
 *	ug_xmlrpc_use_client (&xmlrpc, "http://localhost:8080/RPC", "Agent/1.0");
 *
 *	ug_xmlrpc_call (&xmlrpc,  "methodName"
 *			UG_XMLRPC_INT,    5678,
 *			UG_XMLRPC_STRING, "sample",
 *			UG_XMLRPC_NONE);
 *
 *	if (ug_xmlrpc_response (&xmlrpc) != UG_XMLRPC_ERROR)
 *		ug_xmlrpc_get_value (&xmlrpc, value);
 *
 *	ug_xmlrpc_finalize (&xmlrpc);
 *
 */

#ifndef UG_XMLRPC_H
#define UG_XMLRPC_H

#include <stdarg.h>
#include <glib.h>

#ifdef _WIN32
#include <winsock.h>
#else
// UNIX
#include <netinet/in.h>		// struct sockaddr_in
#include <sys/socket.h>		// socket api
#include <unistd.h>			// uid_t and others
#include <errno.h>
// define Winsock types and functions for UNIX
#define SOCKET			int
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)
#define closesocket		close
#endif


#ifdef __cplusplus
extern "C" {
#endif


typedef struct	UgXmlrpc			UgXmlrpc;
typedef struct	UgXmlrpcValue		UgXmlrpcValue;
typedef union	UgXmlrpcValueC		UgXmlrpcValueC;
typedef enum	UgXmlrpcType		UgXmlrpcType;
typedef enum	UgXmlrpcResponse	UgXmlrpcResponse;

// XML-RPC types
// If no type is indicated, the type is string.
enum UgXmlrpcType
{
	UG_XMLRPC_NONE,

	UG_XMLRPC_INT,			// <i4> or <int>		// int
	UG_XMLRPC_INT64,		// <i8>					// long long
	UG_XMLRPC_BOOLEAN,		// <boolean>			// int
	UG_XMLRPC_STRING,		// <string>				// char*
	UG_XMLRPC_DOUBLE,		// <double>				// double
//	UG_XMLRPC_DATETIME,		// <dateTime.iso8601>	// time_t
	UG_XMLRPC_DATETIME,		// <dateTime.iso8601>	// char*
	UG_XMLRPC_BASE64,		// <base64>				// char*

	// XML-RPC extension, such as
	// Python's None object, Perl's undef value, or a NULL pointer in C.
	UG_XMLRPC_NIL,			// <nil>				// NULL

	UG_XMLRPC_ARRAY,		// <array>				// UgXmlrpcValue*
	UG_XMLRPC_STRUCT,		// <struct>				// UgXmlrpcValue*
};

enum UgXmlrpcResponse
{
	UG_XMLRPC_OK,
	UG_XMLRPC_FAULT,
	UG_XMLRPC_ERROR,	// HTTP header or data has error
};


// ----------------------------------------------------------------------------
// UgXmltag: simple parser for XML-RPC
//
typedef struct	UgXmltag		UgXmltag;
typedef void	(*UgXmltagFunc)	(UgXmltag* xmltag, gpointer user_data);

struct UgXmltag
{
	// beg        end  next
	//  V          V    V
	// <elementName>text<nextElement>
	gchar*			beg;
	gchar*			end;
	gchar*			next;

	struct
	{
		gpointer*		data;
		guint			len;
		guint			allocated;
	} parser;
};

void	ug_xmltag_init     (UgXmltag* xmltag);
void	ug_xmltag_finalize (UgXmltag* xmltag);

#define		ug_xmltag_len(xmltag)		((xmltag)->end - (xmltag)->beg)

gboolean	ug_xmltag_parse (UgXmltag* xmltag, gchar* string);
void		ug_xmltag_push  (UgXmltag* xmltag, UgXmltagFunc func, gpointer data);
void		ug_xmltag_pop   (UgXmltag* xmltag);


// ----------------------------------------------------------------------------
// UgXmlrpc: XML-RPC
//
struct UgXmlrpc
{
	gchar*			host;
	guint			port;

	SOCKET			fd;
	GString*		packet;
	GString*		buffer;

	UgXmltag		tag;

	// position after "Content-length: " in packet
	guint			packet_pos;
};

void	ug_xmlrpc_init     (UgXmlrpc* xmlrpc);
void	ug_xmlrpc_finalize (UgXmlrpc* xmlrpc);

void	ug_xmlrpc_use_client (UgXmlrpc* xmlrpc, const gchar* url, const gchar* user_agent);

// ----------------------------------------------------------------------------
// functions used to write UgXmlrpc.buffer
//
//
//	UgXmlrpcValue*	xrarray  = ug_xmlrpc_value_new_array (0);
//	UgXmlrpcValue*	xrstruct = ug_xmlrpc_value_new_struct (0);
//
//	ug_xmlrpc_call (xmlrpc,   "methodName",
//			UG_XMLRPC_INT,    9876,
//			UG_XMLRPC_DOUBLE, 0.65,
//			UG_XMLRPC_STRING, "sample",
//			UG_XMLRPC_ARRAY,  xrarray,
//			UG_XMLRPC_STRUCT, xrstruct,
//			UG_XMLRPC_NIL,    NULL,
//			UG_XMLRPC_NONE);
//
gboolean	ug_xmlrpc_call (UgXmlrpc* xmlrpc, const gchar* methodName, ...);

// ----------------------------------------------------------------------------
// functions used to parse UgXmlrpc.buffer
//
UgXmlrpcResponse	ug_xmlrpc_response (UgXmlrpc* xmlrpc);

gboolean	ug_xmlrpc_get_value (UgXmlrpc* xmlrpc, UgXmlrpcValue* value);


// ----------------------------------------------------------------------------
// UgXmlrpcValueC: XML-RPC value for C Language
//
union UgXmlrpcValueC
{
	int				int_;		// UG_XMLRPC_INT
	gint64			int64;		// UG_XMLRPC_INT64
	int				boolean;	// UG_XMLRPC_BOOLEAN
	char*			string;		// UG_XMLRPC_STRING
	double			double_;	// UG_XMLRPC_DOUBLE
//	time_t			datetime;	// UG_XMLRPC_DATETIME
	char*			datetime;	// UG_XMLRPC_DATETIME
	char*			base64;		// UG_XMLRPC_BASE64

	// used for UG_XMLRPC_STRUCT only
	GTree*			tree;
};


// ----------------------------------------------------------------------------
// UgXmlrpcValue: XML-RPC value
//
struct UgXmlrpcValue
{
	UgXmlrpcType	type;
	UgXmlrpcValueC	c;

	// used for member of UG_XMLRPC_STRUCT
	gchar*			name;

	// used for UG_XMLRPC_ARRAY and UG_XMLRPC_STRUCT
	UgXmlrpcValue*	data;
	guint			len;
	guint			allocated;
};

UgXmlrpcValue*	ug_xmlrpc_value_new   (void);
void			ug_xmlrpc_value_free  (UgXmlrpcValue* value);
void			ug_xmlrpc_value_clear (UgXmlrpcValue* value);
UgXmlrpcValue*	ug_xmlrpc_value_alloc (UgXmlrpcValue* value);
UgXmlrpcValue*	ug_xmlrpc_value_find  (UgXmlrpcValue* value, const gchar* name);

UgXmlrpcValue*	ug_xmlrpc_value_new_data (UgXmlrpcType type, guint preallocated_size);
#define			ug_xmlrpc_value_new_array(size)		ug_xmlrpc_value_new_data (UG_XMLRPC_ARRAY,  size)
#define			ug_xmlrpc_value_new_struct(size)	ug_xmlrpc_value_new_data (UG_XMLRPC_STRUCT, size)


#ifdef __cplusplus
}
#endif

#endif  // UG_XMLRPC_H

