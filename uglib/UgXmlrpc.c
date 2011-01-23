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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_PLUGIN_ARIA2

#ifndef _WIN32
#include <netdb.h>		// gethostbyname ()
#include <arpa/inet.h>	// inet_addr ()
#endif

#include <stdlib.h>
#include <string.h>
#include <UgUri.h>
#include <UgXmlrpc.h>


// ----------------------------------------------------------------------------
// UgXmlrpc: XML-RPC
//
static void	ug_xmlrpc_add_value  (UgXmlrpc* xmlrpc, UgXmlrpcValue* value);
static void	ug_xmlrpc_add_array  (UgXmlrpc* xmlrpc, UgXmlrpcValue* value);
static void	ug_xmlrpc_add_struct (UgXmlrpc* xmlrpc, UgXmlrpcValue* value);
static void	ug_str_unescape_text (gchar* text);

void	ug_xmlrpc_init (UgXmlrpc* xmlrpc)
{
	xmlrpc->host = NULL;
	xmlrpc->port = 80;

	xmlrpc->fd = INVALID_SOCKET;
	xmlrpc->header = g_string_sized_new (128);
	xmlrpc->buffer = g_string_sized_new (1024);

	ug_xmltag_init (&xmlrpc->tag);
}

void	ug_xmlrpc_finalize (UgXmlrpc* xmlrpc)
{
	g_free (xmlrpc->host);
	g_string_free (xmlrpc->header, TRUE);
	g_string_free (xmlrpc->buffer, TRUE);

	if (xmlrpc->fd != INVALID_SOCKET)
		closesocket (xmlrpc->fd);

	ug_xmltag_finalize (&xmlrpc->tag);
}

static gboolean	ug_xmlrpc_open_socket (UgXmlrpc* xmlrpc)
{
	SOCKET				fd;
	struct sockaddr_in	saddr;
	struct hostent*		hostentry;

	fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == INVALID_SOCKET)
		return FALSE;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons (xmlrpc->port);
//	saddr.sin_addr.s_addr = inet_addr ("127.0.0.1");

	if (xmlrpc->host) {
		hostentry = gethostbyname (xmlrpc->host);
		if (hostentry)
			saddr.sin_addr = *(struct in_addr*) hostentry->h_addr_list[0];
//			saddr.sin_addr = *(struct in_addr*) hostentry->h_addr;
	}

	if (connect (fd, (struct sockaddr *) &saddr, sizeof(saddr)) == SOCKET_ERROR) {
		closesocket (fd);
		return FALSE;
	}

	if (xmlrpc->fd != INVALID_SOCKET)
		closesocket (xmlrpc->fd);
	xmlrpc->fd = fd;

	return TRUE;
}

void	ug_xmlrpc_use_client (UgXmlrpc* xmlrpc, const gchar* url, const gchar* user_agent)
{
	UgUriFull	urifull;
	gchar*		string;
	guint		len;

	if (ug_uri_full_init (&urifull, url) == 0)
		return;

	if ((len = ug_uri_full_host (&urifull, NULL))) {
		g_free (xmlrpc->host);
		xmlrpc->host = g_strndup (urifull.host, len);
	}
	if (urifull.port)
		xmlrpc->port = atoi (urifull.port);

	g_string_printf (xmlrpc->header,
			"POST %s HTTP/1.0\r\n"
			"Host: %s:%d\r\n"
			"User-Agent: %s\r\n"
			"Content-Type: text/xml\r\n",
			urifull.path,
			xmlrpc->host,
			xmlrpc->port ? xmlrpc->port : 80,
			user_agent ? user_agent : "uGet/1.7");

	// http header "Authorization: "
	// base64 encoding - "user:password"
	if (urifull.authority != urifull.host) {
		string = g_base64_encode ((const guchar*)urifull.authority,
				urifull.host - urifull.authority);
		g_string_append_printf (xmlrpc->header,
				"Authorization: %s\r\n",
				string);
		g_free (string);
	}

	g_string_append (xmlrpc->header,
			"Content-length: ");
	xmlrpc->header_pos = xmlrpc->header->len;
}

// ----------------------------------------------------------------------------
// functions used to write UgXmlrpc.buffer
//
gboolean	ug_xmlrpc_call (UgXmlrpc* xmlrpc, const gchar* methodName, ...)
{
	GString*	buffer;
	va_list		args;
	gboolean	result = FALSE;
	union {
		int				len;
		gchar*			str;
		UgXmlrpcValue*	value;
	} temp;

	if (ug_xmlrpc_open_socket (xmlrpc) == FALSE)
		return FALSE;

	buffer = xmlrpc->buffer;
	g_string_assign (buffer,
					"<?xml version='1.0'?>"
					"<methodCall>"
					"<methodName>");
	g_string_append (buffer,
					methodName);
	g_string_append (buffer,
					"</methodName>"
					"<params>");

	va_start (args, methodName);
	for (;;) {
		g_string_append (buffer,
						"<param>"
						"<value>");
		switch (va_arg (args, int)) {
		case UG_XMLRPC_INT:
			g_string_append_printf (buffer,
					"<int>%d</int>", va_arg (args, int));
			break;

		case UG_XMLRPC_INT64:
			g_string_append_printf (buffer,
					"<i8>%lld</i8>", va_arg (args, gint64));
			break;

		case UG_XMLRPC_BOOLEAN:
			g_string_append_printf (buffer,
					"<boolean>%d</boolean>", va_arg (args, int));
			break;

		case UG_XMLRPC_STRING:
			temp.str = g_markup_escape_text (va_arg (args, char*), -1);
			g_string_append_printf (buffer,
					"<string>%s</string>", temp.str);
			g_free (temp.str);
			break;

		case UG_XMLRPC_DOUBLE:
			g_string_append_printf (buffer,
					"<double>%f</double>", va_arg (args, double));
			break;

		case UG_XMLRPC_DATETIME:
			g_string_append_printf (buffer,
					"<dateTime.iso8601>%s</dateTime.iso8601>", va_arg (args, char*));
			break;

		case UG_XMLRPC_BINARY:
			temp.value = va_arg (args, UgXmlrpcValue*);
			temp.str = g_base64_encode (temp.value->c.binary, temp.value->len);
			g_string_append_printf (buffer,
					"<base64>%s</base64>", temp.str);
			g_free (temp.str);
			break;

		case UG_XMLRPC_NIL:
			(void) va_arg (args, void*);
			g_string_append (buffer, "<nil/>");
			break;

		case UG_XMLRPC_ARRAY:
			ug_xmlrpc_add_array (xmlrpc, va_arg (args, UgXmlrpcValue*));
			break;

		case UG_XMLRPC_STRUCT:
			ug_xmlrpc_add_struct (xmlrpc, va_arg (args, UgXmlrpcValue*));
			break;

		default:
			// buffer->len - strlen ("<param><value>")
			g_string_truncate (buffer, buffer->len -14);
			goto break_for_loop;
		}
		g_string_append (buffer,
						"</value>"
						"</param>");
	}
break_for_loop:
	va_end (args);

	g_string_append (buffer,
					"</params>"
					"</methodCall>");

	// complete HTTP header "Content-length: "
	xmlrpc->header->len = xmlrpc->header_pos;
	g_string_append_printf (xmlrpc->header,
							"%u" "\r\n"
							"\r\n", (guint) buffer->len);
	// send header + content
	if (send (xmlrpc->fd, xmlrpc->header->str, xmlrpc->header->len, 0) == xmlrpc->header->len &&
		send (xmlrpc->fd, buffer->str, buffer->len, 0) == buffer->len)
	{
		// recv to buffer
		buffer->len = 0;
		for (;;) {
			temp.len = buffer->allocated_len - buffer->len;
			temp.len = recv (xmlrpc->fd, buffer->str + buffer->len, temp.len, 0);
			if (temp.len < 1) {
				if (temp.len == 0)
					result = TRUE;
				// buffer is null-terminated
				g_string_set_size (buffer, buffer->len);
				break;
			}
			buffer->len += temp.len;
			if (buffer->len == buffer->allocated_len) {
				temp.len = buffer->len;
				g_string_set_size (buffer, buffer->allocated_len * 2);
				buffer->len = temp.len;
			}
			// check buffer size
			if (buffer->len > 1024 * 32) {
				g_string_set_size (buffer, buffer->len);
				break;
			}
		}
	}

	closesocket (xmlrpc->fd);
	xmlrpc->fd = INVALID_SOCKET;

	return result;
}

static void	ug_xmlrpc_add_value (UgXmlrpc* xmlrpc, UgXmlrpcValue* value)
{
	GString*	buffer;
	gchar*		temp;

	buffer   = xmlrpc->buffer;
	g_string_append (buffer, "<value>");
	switch (value->type) {
	case UG_XMLRPC_INT:
		g_string_append_printf (buffer,
				"<int>%d</int>", value->c.int_);
		break;

	case UG_XMLRPC_INT64:
		g_string_append_printf (buffer,
				"<i8>%lld</i8>", value->c.int64);
		break;

	case UG_XMLRPC_BOOLEAN:
		g_string_append_printf (buffer,
				"<boolean>%d</boolean>", value->c.boolean);
		break;

	case UG_XMLRPC_STRING:
		temp = g_markup_escape_text (value->c.string, -1);
		g_string_append_printf (buffer,
				"<string>%s</string>", temp);
		g_free (temp);
		break;

	case UG_XMLRPC_DOUBLE:
		g_string_append_printf (buffer,
				"<double>%f</double>", value->c.double_);
		break;

	case UG_XMLRPC_DATETIME:
		g_string_append_printf (buffer,
				"<dateTime.iso8601>%s</dateTime.iso8601>", value->c.string);
		break;

	case UG_XMLRPC_BINARY:
		temp = g_base64_encode (value->c.binary, value->len);
		g_string_append_printf (buffer,
				"<base64>%s</base64>", temp);
		g_free (temp);
		break;

	case UG_XMLRPC_NIL:
		g_string_append (buffer, "<nil/>");
		break;

	case UG_XMLRPC_ARRAY:
		ug_xmlrpc_add_array (xmlrpc, value);
		break;

	case UG_XMLRPC_STRUCT:
		ug_xmlrpc_add_struct (xmlrpc, value);
		break;

	default:
		break;
	}
	g_string_append (buffer, "</value>");
}

static void	ug_xmlrpc_add_array (UgXmlrpc* xmlrpc, UgXmlrpcValue* value)
{
	UgXmlrpcValue*	cur;
	UgXmlrpcValue*	end;

	cur = value->data;
	end = cur + value->len;

	g_string_append (xmlrpc->buffer,
					"<array>"
					"<data>");

	for (;  cur < end;  cur++)
		ug_xmlrpc_add_value (xmlrpc, cur);

	g_string_append (xmlrpc->buffer,
					"</data>"
					"</array>");
}

static void	ug_xmlrpc_add_struct (UgXmlrpc* xmlrpc, UgXmlrpcValue*  value)
{
	GString*		buffer;
	UgXmlrpcValue*	cur;
	UgXmlrpcValue*	end;

	buffer = xmlrpc->buffer;
	cur    = value->data;
	end    = cur + value->len;

	g_string_append (buffer, "<struct>");
	for (;  cur < end;  cur++) {
		g_string_append (buffer,
						"<member>"
						"<name>");
		g_string_append (buffer, cur->name);
		g_string_append (buffer,
						"</name>");
		ug_xmlrpc_add_value (xmlrpc, cur);
		g_string_append (buffer,
						"</member>");
	}
	g_string_append (buffer, "</struct>");
}


// ----------------------------------------------------------------------------
// functions used to parse UgXmlrpc.buffer
//
static void	ug_xmltag_parse_unknow (UgXmltag* xmltag, gpointer data);
static void	ug_xmltag_parse_param_top (UgXmltag* xmltag, UgXmlrpcValue* value);
static void	ug_xmltag_parse_value (UgXmltag* xmltag, UgXmlrpcValue* value);
static void	ug_xmltag_parse_value_top (UgXmltag* xmltag, UgXmlrpcValue* value);

static void	ug_xmltag_parse_array (UgXmltag* xmltag, UgXmlrpcValue* value);
static void	ug_xmltag_parse_array_data (UgXmltag* xmltag, UgXmlrpcValue* value);
static void	ug_xmltag_parse_struct (UgXmltag* xmltag, UgXmlrpcValue* value);
static void	ug_xmltag_parse_struct_member (UgXmltag* xmltag, UgXmlrpcValue* value);


UgXmlrpcResponse	ug_xmlrpc_response (UgXmlrpc* xmlrpc)
{
	gchar*		string;
	gchar*		tag;

	string = xmlrpc->buffer->str;

	// ----------------------------------------------------
	// HTTP header
	//
	if ((string = strstr (string, "200 OK")) == NULL)
		return UG_XMLRPC_ERROR;
	string += 6;	// strlen ("200 OK");

//	if ((string = strstr (string, "Content-Type: ")) == NULL)
//		return UG_XMLRPC_ERROR;
//	string += 14;	// strlen ("Content-Type: ");

	//  strncmp (string, "text/xml", strlen ("text/xml")
//	if (strncmp (string, "text/xml", 8) != 0)
//		return UG_XMLRPC_ERROR;
//	string += 8;	// strlen ("text/xml"), "\r\n\r\n");

	if ((string = strstr (string, "\r\n\r\n")) == NULL)
		return UG_XMLRPC_ERROR;
	string += 4;	// strlen ("\r\n\r\n");

	// ----------------------------------------------------
	// XML content
	//
//	if ((string = strstr (string, "<?xml")) == NULL)
//		return FALSE;
//	string += 5;	// strlen ("<?xml");

	if ((string = strstr (string, "<methodResponse>")) == NULL)
		return UG_XMLRPC_ERROR;

	// get next tag after <methodResponse>
	//            strchr (string + strlen ("<methodResponse>"), '<')
	if ((string = strchr (string + 16, '<')) == NULL)
		return UG_XMLRPC_ERROR;

	// get next 2 tag after <methodResponse>
	if ((tag = strchr (string + 1, '<')) != NULL)
		xmlrpc->tag.beg = tag;

	//  strncmp (string, "<fault>", strlen ("<fault>"))
	if (strncmp (string, "<fault>", 7) == 0)
		return UG_XMLRPC_FAULT;

	return UG_XMLRPC_OK;
}

gboolean	ug_xmlrpc_get_value  (UgXmlrpc* xmlrpc, UgXmlrpcValue* value)
{
	// param or value
	if (xmlrpc->tag.beg[1] == 'p')
		ug_xmltag_push (&xmlrpc->tag, (UgXmltagFunc) ug_xmltag_parse_param_top, value);
	else
		ug_xmltag_push (&xmlrpc->tag, (UgXmltagFunc) ug_xmltag_parse_value_top, value);

	if (ug_xmltag_parse (&xmlrpc->tag, xmlrpc->tag.beg) == FALSE) {
		ug_xmlrpc_value_clear (value);
		return FALSE;
	}
	return TRUE;
}

// parser ---------
static void	ug_xmltag_parse_unknow (UgXmltag* xmltag, gpointer data)
{
	ug_xmltag_push (xmltag, ug_xmltag_parse_unknow, NULL);
}

static void	ug_xmltag_parse_param_top (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (strncmp (xmltag->beg, "param", xmltag->end - xmltag->beg) == 0)
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_value_top, value);
	else
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_unknow, value);
}

static void	ug_xmltag_parse_value (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	gchar*	tag  = xmltag->beg;
	gchar*	text = xmltag->end + 1;

	if (xmltag->next == NULL)
		return;

	switch (tag[0]) {
	case 'i':	// int, i4, i8
		if (tag[1] == '8') {
			value->type = UG_XMLRPC_INT64;
#ifdef _WIN32
			value->c.int64 = _atoi64 (text);
#else
			value->c.int64 = atoll (text);
#endif
		}
		else {
			value->type = UG_XMLRPC_INT;
			value->c.int_ =  atoi (text);
		}
		break;

	case 's':	// string or struct
		if (strncmp (tag, "struct", xmltag->end - xmltag->beg) == 0) {
			value->type = UG_XMLRPC_STRUCT;
			ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_struct, value);
			return;
		}
		else {
			value->type = UG_XMLRPC_STRING;
			value->c.string = text;
			xmltag->next[0] = 0;	// null-terminated
			ug_str_unescape_text (text);
		}
		break;

	case 'b':	// boolean or base64
		if (tag[1] == 'o') {
			value->type = UG_XMLRPC_BOOLEAN;
			value->c.boolean = atoi (text);
		}
		else {
			value->type = UG_XMLRPC_BINARY;
			xmltag->next[0] = 0;	// null-terminated
			value->c.binary = g_base64_decode_inplace (text, &value->len);
		}
		break;

	case 'd':	// double  or dateTime.iso8601
		if (tag[1] == 'o')
			value->c.double_ = atof (text);
		else {
			value->c.datetime = text;
			xmltag->next[0] = 0;	// null-terminated
		}
		break;

	case 'a':	// array
		value->type = UG_XMLRPC_ARRAY;
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_array, value);
		return;

	case 'n':	// nil
		value->type = UG_XMLRPC_NIL;
//		value->c.tag = NULL;
		break;

	default:
		break;
	}

	ug_xmltag_push (xmltag, ug_xmltag_parse_unknow, NULL);
}

static void	ug_xmltag_parse_value_top (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (xmltag->next && xmltag->next[1] == '/') {
		value->type = UG_XMLRPC_STRING;
		value->c.string = xmltag->end + 1;
		xmltag->next[0] = 0;	// null-terminated
	}
	ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_value, value);
}

static void	ug_xmltag_parse_array (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (strncmp (xmltag->beg, "data", xmltag->end - xmltag->beg) == 0)
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_array_data, value);
	else
		ug_xmltag_push (xmltag, ug_xmltag_parse_unknow, NULL);
}

static void	ug_xmltag_parse_array_data (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	ug_xmltag_parse_value_top (xmltag, ug_xmlrpc_value_alloc (value));
}

static void	ug_xmltag_parse_struct (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (strncmp (xmltag->beg, "member", xmltag->end - xmltag->beg) == 0) {
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_struct_member,
				ug_xmlrpc_value_alloc (value));
	}
	else
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_unknow, NULL);
}

static void	ug_xmltag_parse_struct_member (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	switch (xmltag->beg[0]) {
	// name
	case 'n':
		if (xmltag->next && xmltag->next[1] == '/') {
			value->name = xmltag->end + 1;
			xmltag->next[0] = 0;	// null-terminated
		}
		ug_xmltag_push (xmltag, ug_xmltag_parse_unknow, NULL);
		break;

	// value
	case 'v':
		ug_xmltag_parse_value_top (xmltag, value);
		break;
	}
}


// ----------------------------------------------------------------------------
// UgXmltag: simple parser for XML-RPC
//
void	ug_xmltag_init (UgXmltag* xmltag)
{
	xmltag->parser.data = NULL;
	xmltag->parser.allocated = 0;
	xmltag->parser.len = 0;
}

void	ug_xmltag_finalize (UgXmltag* xmltag)
{
	g_free (xmltag->parser.data);
}

gboolean	ug_xmltag_parse  (UgXmltag* xmltag, gchar* string)
{
	gpointer*		parser;
	UgXmltagFunc	func;
	gpointer		data;

	if (string [0] != '<')
		return FALSE;
	for (xmltag->beg = string + 1;  ;  ) {
		if (xmltag->parser.len == 0)
			return TRUE;
		if ((xmltag->end = strchr (xmltag->beg, '>')) == NULL)
			break;
		xmltag->next = strchr (xmltag->end + 1, '<');

		if (xmltag->beg[0] == '/')
			ug_xmltag_pop (xmltag);
		else {
			parser = xmltag->parser.data + xmltag->parser.len;
			func = *(parser-2);
			data = *(parser-1);
			func (xmltag, data);
			if (xmltag->end[-1] == '/')
				ug_xmltag_pop (xmltag);
		}

		if (xmltag->next == NULL)
			break;
		xmltag->beg = xmltag->next + 1;
	}
	xmltag->parser.len = 0;
	return FALSE;
}

void	ug_xmltag_push (UgXmltag* xmltag, UgXmltagFunc func, gpointer data)
{
	if (xmltag->parser.allocated == xmltag->parser.len) {
		if (xmltag->parser.allocated == 0)
			xmltag->parser.allocated = 32 * 2;
		else
			xmltag->parser.allocated *= 2;
		xmltag->parser.data = g_realloc (xmltag->parser.data,
				sizeof (gpointer) * xmltag->parser.allocated);
	}
	xmltag->parser.data [xmltag->parser.len++] = func;
	xmltag->parser.data [xmltag->parser.len++] = data;
}

void	ug_xmltag_pop (UgXmltag* xmltag)
{
	if (xmltag->parser.len > 1)
		xmltag->parser.len -= 2;
}


// ----------------------------------------------------------------------------
// UgXmlrpcValue: XML-RPC <value>
//
UgXmlrpcValue*	ug_xmlrpc_value_new (void)
{
	return g_slice_new0 (UgXmlrpcValue);
}

UgXmlrpcValue*	ug_xmlrpc_value_new_data (UgXmlrpcType type, guint preallocated_size)
{
	UgXmlrpcValue*	value;

	value = g_slice_new0 (UgXmlrpcValue);
	value->type = type;
	if (type >= UG_XMLRPC_ARRAY) {
		value->data = g_malloc (sizeof (UgXmlrpcValue) * preallocated_size);
		value->allocated = preallocated_size;
	}
	return value;
}

void	ug_xmlrpc_value_free (UgXmlrpcValue* value)
{
	ug_xmlrpc_value_clear (value);
	g_slice_free (UgXmlrpcValue, value);
}

void	ug_xmlrpc_value_clear (UgXmlrpcValue* value)
{
	UgXmlrpcValue*	cur;
	UgXmlrpcValue*	end;

	if (value->data) {
		cur = value->data;
		end = cur + value->len;
		// free array data or struct members
		for (;  cur < end;  cur++) {
			if (cur->type >= UG_XMLRPC_ARRAY)
				ug_xmlrpc_value_clear (cur);
		}
		g_free (value->data);
		value->data = NULL;
		value->len = 0;
		value->allocated = 0;
		// free Balanced Binary Trees
		if (value->c.tree) {
			g_tree_destroy (value->c.tree);
			value->c.tree = NULL;
		}
	}
}

UgXmlrpcValue*	ug_xmlrpc_value_alloc (UgXmlrpcValue* value)
{
	UgXmlrpcValue*	newone;

	if (value->type >= UG_XMLRPC_ARRAY) {
		if (value->len == value->allocated) {
			if (value->allocated == 0)
				value->allocated  = 16;
			else
				value->allocated *= 2;

			value->data = g_realloc (value->data,
					sizeof (UgXmlrpcValue) * value->allocated);
		}

		newone = value->data + value->len++;
//		memset (newone, 0, sizeof (UgXmlrpcValue));
		ug_xmlrpc_value_init (newone);
		return newone;
	}

	return NULL;
}

UgXmlrpcValue*	ug_xmlrpc_value_find (UgXmlrpcValue* value, const gchar* name)
{
	UgXmlrpcValue*	cur;
	UgXmlrpcValue*	end;

	if (value->type == UG_XMLRPC_STRUCT) {
		if (value->c.tree == NULL) {
			value->c.tree = g_tree_new ((GCompareFunc) g_strcmp0);
			cur = value->data;
			end = cur + value->len;

			for (;  cur < end;  cur++)
				g_tree_insert (value->c.tree, cur->name, cur);
		}
		return g_tree_lookup (value->c.tree, name);
	}
	return NULL;
}


// ----------------------------------------------------------------------------
// utility
//
static void	ug_str_unescape_text (gchar* text)
{
	gchar*	beg;
	gchar*	end;
	guint	len;

	if ((text = strchr (text, '&')) == NULL)
		return;
	end = text;
	// beg  end
	//  v   v
	// &quot;
	for (;;) {
		for (beg = end;  *beg;  beg++) {
			if (*beg == '&')
				break;
		}
		while (end < beg)
			*text++ = *end++;
		if ((end = strchr (beg, ';')) == NULL)
			break;
		len =  end - (beg += 1);

		if (strncmp (beg, "quot", len) == 0)
			*text++ = '\"';
		else if (strncmp (beg, "amp", len) == 0)
			*text++ = '&';
		else if (strncmp (beg, "lt", len) == 0)
			*text++ = '<';
		else if (strncmp (beg, "gt", len) == 0)
			*text++ = '>';
		end++;
	}

	*text = 0;
}

#endif	// HAVE_PLUGIN_ARIA2

