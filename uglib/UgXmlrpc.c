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
void	ug_xmlrpc_init (UgXmlrpc* xmlrpc)
{
	xmlrpc->host = NULL;
	xmlrpc->port = 80;

	xmlrpc->fd = INVALID_SOCKET;
	xmlrpc->packet = g_string_sized_new (1024);
	xmlrpc->buffer = g_string_sized_new (1024);

	ug_xmltag_init (&xmlrpc->tag);
}

void	ug_xmlrpc_finalize (UgXmlrpc* xmlrpc)
{
	g_free (xmlrpc->host);
	g_string_free (xmlrpc->packet, TRUE);
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

	fd = socket (AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET)
		return FALSE;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons (xmlrpc->port);
//	saddr.sin_addr.s_addr = inet_addr ("127.0.0.1");

	if (xmlrpc->host) {
		hostentry = gethostbyname (xmlrpc->host);
		if (hostentry)
			saddr.sin_addr = *((struct in_addr*) hostentry->h_addr_list[0]);
//			saddr.sin_addr = *((struct in_addr*) hostentry->h_addr);
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

	g_string_truncate (xmlrpc->packet, 0);
	g_string_append_printf (xmlrpc->packet,
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
		g_string_append_printf (xmlrpc->packet,
				"Authorization: %s\r\n",
				string);
		g_free (string);
	}

	g_string_append (xmlrpc->packet,
			"Content-length: ");
	xmlrpc->packet_pos = xmlrpc->packet->len;
}

// ----------------------------------------------------------------------------
// functions used to write UgXmlrpc.buffer
//
gboolean	ug_xmlrpc_call (UgXmlrpc* xmlrpc, const gchar* methodName, ...)
{
	GString*	buffer;
	va_list		args;
	int			n;

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
			g_string_append_printf (buffer,
					"<string>%s</string>", va_arg (args, char*));
			break;

		case UG_XMLRPC_DOUBLE:
			g_string_append_printf (buffer,
					"<double>%f</double>", va_arg (args, double));
			break;

		case UG_XMLRPC_DATETIME:
			g_string_append_printf (buffer,
					"<dateTime.iso8601>%s</dateTime.iso8601>", va_arg (args, char*));
			break;

		case UG_XMLRPC_BASE64:
			g_string_append_printf (buffer,
					"<base64>%s</base64>", va_arg (args, char*));
			break;

		case UG_XMLRPC_ARRAY:
			ug_xmlrpc_add_array (xmlrpc, va_arg (args, void*));
			break;

		case UG_XMLRPC_STRUCT:
			ug_xmlrpc_add_struct (xmlrpc, va_arg (args, void*));
			break;

		case UG_XMLRPC_NIL:
			g_string_append (buffer, "<nil/>");
			break;

		default:
		case UG_XMLRPC_NONE:
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

	g_string_truncate (xmlrpc->packet, xmlrpc->packet_pos);
	g_string_append_printf (xmlrpc->packet,
							"%u" "\r\n"
							"\r\n", (guint) buffer->len);
	g_string_append_len (xmlrpc->packet, buffer->str, buffer->len);

	// send packet
	n = send (xmlrpc->fd, xmlrpc->packet->str, xmlrpc->packet->len, 0);
	if (n == SOCKET_ERROR || n != xmlrpc->packet->len)
		return FALSE;
	// recv to buffer
	n = recv (xmlrpc->fd, buffer->str, buffer->allocated_len, 0);
	buffer->len = 0;
	for (;;) {
		if (n == SOCKET_ERROR)
			return FALSE;
		buffer->len += n;
		if (n == 0) {
			// buffer is null-terminated
			g_string_set_size (buffer, buffer->len);
			break;
		}
		if (buffer->len == buffer->allocated_len) {
			g_string_set_size (buffer, buffer->allocated_len + 1024);
			buffer->len = buffer->allocated_len - 1024;
		}
		// check packet size
		if (buffer->allocated_len > 4096)
			return FALSE;
		// get remained data
		n = recv (xmlrpc->fd, buffer->str + buffer->len, 1024, 0);
	}

	closesocket (xmlrpc->fd);
	xmlrpc->fd = INVALID_SOCKET;

	return TRUE;
}

void	ug_xmlrpc_add_value (UgXmlrpc* xmlrpc, UgXmlrpcValue* value)
{
	gboolean		has_args;
	GString*		buffer;

	has_args = TRUE;
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
		g_string_append_printf (buffer,
				"<string>%s</string>", value->c.string);
		break;

	case UG_XMLRPC_DOUBLE:
		g_string_append_printf (buffer,
				"<double>%f</double>", value->c.double_);
		break;

	case UG_XMLRPC_DATETIME:
		g_string_append_printf (buffer,
				"<dateTime.iso8601>%s</dateTime.iso8601>", value->c.string);
		break;

	case UG_XMLRPC_BASE64:
		g_string_append_printf (buffer,
				"<base64>%s</base64>", value->c.base64);
		break;

	case UG_XMLRPC_STRUCT:
		ug_xmlrpc_add_struct (xmlrpc, value->c.struct_);
		break;

	case UG_XMLRPC_ARRAY:
		ug_xmlrpc_add_array (xmlrpc, value->c.array);
		break;

	case UG_XMLRPC_NIL:
		g_string_append (buffer, "<nil/>");
		break;

	default:
		has_args = FALSE;
		break;
	}
	g_string_append (buffer, "</value>");
}

void	ug_xmlrpc_add_array (UgXmlrpc* xmlrpc, UgXmlrpcData*  xrdata)
{
	UgXmlrpcValue*	value;
	UgXmlrpcValue*	end;

	value = xrdata->data;
	end   = value + xrdata->len;

	g_string_append (xmlrpc->buffer,
					"<array>"
					"<data>");

	for (;  value < end;  value++)
		ug_xmlrpc_add_value (xmlrpc, value);

	g_string_append (xmlrpc->buffer,
					"</data>"
					"</array>");
}

void	ug_xmlrpc_add_struct (UgXmlrpc* xmlrpc, UgXmlrpcData*  xrdata)
{
	GString*		buffer;
	UgXmlrpcValue*	value;
	UgXmlrpcValue*	end;

	buffer = xmlrpc->buffer;
	value  = xrdata->data;
	end    = value + xrdata->len;

	g_string_append (buffer, "<struct>");
	for (;  value < end;  value++) {
		g_string_append (buffer,
						"<member>"
						"<name>");
		g_string_append (buffer, value->name);
		g_string_append (buffer,
						"</name>");
		ug_xmlrpc_add_value (xmlrpc, value);
		g_string_append (buffer,
						"</member>");
	}
	g_string_append (buffer, "</struct>");
}


// ----------------------------------------------------------------------------
// functions used to parse UgXmlrpc.buffer
//
static void	ug_xmltag_parse_unknow (UgXmltag* xmltag, gpointer data);
static void	ug_xmltag_parse_param (UgXmltag* xmltag, UgXmlrpcValue* value);
static void	ug_xmltag_parse_value (UgXmltag* xmltag, UgXmlrpcValue* value);
static void	ug_xmltag_parse_value_inplace (UgXmltag* xmltag, UgXmlrpcValue* value);

static void	ug_xmltag_parse_struct (UgXmltag* xmltag, UgXmlrpcData* xrdata);
static void	ug_xmltag_parse_struct_member (UgXmltag* xmltag, UgXmlrpcValue* value);

static void	ug_xmltag_parse_array (UgXmltag* xmltag, UgXmlrpcData* xrdata);


UgXmlrpcResponse	ug_xmlrpc_response (UgXmlrpc* xmlrpc)
{
	gchar*		string;
	gchar*		tag;

	string = xmlrpc->buffer->str;

	// ----------------------------------------------------
	// HTTP header
	//
	if ((string = strstr (string, "200 OK")) == NULL)
		return UG_XMLRPC_RESPONSE_ERROR;
	string += 6;	// strlen ("200 OK");

//	if ((string = strstr (string, "Content-Type: ")) == NULL)
//		return UG_XMLRPC_RESPONSE_ERROR;
//	string += 14;	// strlen ("Content-Type: ");

	//  strncmp (string, "text/xml", strlen ("text/xml")
//	if (strncmp (string, "text/xml", 8) != 0)
//		return UG_XMLRPC_RESPONSE_ERROR;
//	string += 8;	// strlen ("text/xml"), "\r\n\r\n");

	if ((string = strstr (string, "\r\n\r\n")) == NULL)
		return UG_XMLRPC_RESPONSE_ERROR;
	string += 4;	// strlen ("\r\n\r\n");

	// ----------------------------------------------------
	// XML content
	//
//	if ((string = strstr (string, "<?xml")) == NULL)
//		return FALSE;
//	string += 5;	// strlen ("<?xml");

	if ((string = strstr (string, "<methodResponse>")) == NULL)
		return UG_XMLRPC_RESPONSE_ERROR;

	// get next tag after <methodResponse>
	//            strchr (string + strlen ("<methodResponse>"), '<')
	if ((string = strchr (string + 16, '<')) == NULL)
		return UG_XMLRPC_RESPONSE_ERROR;

	// get next 2 tag after <methodResponse>
	if ((tag = strchr (string + 1, '<')) != NULL)
		xmlrpc->tag.beg = tag;

	//  strncmp (string, "<fault>", strlen ("<fault>"))
	if (strncmp (string, "<fault>", 7) == 0)
		return UG_XMLRPC_RESPONSE_FAULT;

	return UG_XMLRPC_RESPONSE_OK;
}

gboolean	ug_xmlrpc_get_param  (UgXmlrpc* xmlrpc, UgXmlrpcValue*  value)
{
	ug_xmltag_push (&xmlrpc->tag, (UgXmltagFunc) ug_xmltag_parse_param, value);

	return ug_xmltag_parse (&xmlrpc->tag, xmlrpc->tag.beg);
}

gboolean	ug_xmlrpc_get_value  (UgXmlrpc* xmlrpc, UgXmlrpcValue*  value)
{
	ug_xmltag_push (&xmlrpc->tag, (UgXmltagFunc) ug_xmltag_parse_value_inplace, value);

	return ug_xmltag_parse (&xmlrpc->tag, xmlrpc->tag.beg);
}

// parser ---------
static void	ug_xmltag_parse_unknow (UgXmltag* xmltag, gpointer data)
{
	ug_xmltag_push (xmltag, ug_xmltag_parse_unknow, NULL);
}

static void	ug_xmltag_parse_param (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (strncmp (xmltag->beg, "param", xmltag->end - xmltag->beg) == 0)
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_value_inplace, value);
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
			value->c.struct_ = ug_xmlrpc_data_new (16);
			ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_struct, value->c.struct_);
			return;
		}
		else {
			value->type = UG_XMLRPC_STRING;
			value->c.string = text;
			xmltag->next[0] = 0;	// null-terminated
		}
		break;

	case 'b':	// boolean or base64
		if (tag[1] == 'o') {
			value->type = UG_XMLRPC_BOOLEAN;
			value->c.boolean = atoi (text);
		}
		else {
			value->type = UG_XMLRPC_BASE64;
			value->c.string = text;
			xmltag->next[0] = 0;	// null-terminated
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
		value->c.array = ug_xmlrpc_data_new (16);
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_array, value->c.array);
		return;

	case 'n':	// nil
		value->type = UG_XMLRPC_NIL;
//		value->c.array = NULL;
		break;

	default:
		break;
	}

	ug_xmltag_push (xmltag, ug_xmltag_parse_unknow, NULL);
}

static void	ug_xmltag_parse_value_inplace (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (xmltag->next && xmltag->next[1] == '/') {
		value->type = UG_XMLRPC_STRING;
		value->c.string = xmltag->end + 1;
		xmltag->next[0] = 0;	// null-terminated
	}
	ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_value, value);
}

static void	ug_xmltag_parse_struct (UgXmltag* xmltag, UgXmlrpcData* xrdata)
{
	if (strncmp (xmltag->beg, "member", xmltag->end - xmltag->beg) == 0) {
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_struct_member,
				ug_xmlrpc_data_alloc (xrdata));
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
		ug_xmltag_parse_value_inplace (xmltag, value);
		break;
	}
}

static void	ug_xmltag_parse_array (UgXmltag* xmltag, UgXmlrpcData* xrdata)
{
	if (strncmp (xmltag->beg, "data", xmltag->end - xmltag->beg) == 0) {
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_value_inplace,
				ug_xmlrpc_data_alloc (xrdata));
	}
	else
		ug_xmltag_push (xmltag, ug_xmltag_parse_unknow, NULL);
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

	xmltag->beg = string + 1;

	for (;;) {
		if (xmltag->parser.len == 0)
			return TRUE;

		if (xmltag->beg == NULL)
			return FALSE;
		xmltag->end  = strchr (xmltag->beg,    '>');
		xmltag->next = strchr (xmltag->end +1, '<');

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
			return FALSE;
		xmltag->beg = xmltag->next + 1;
	}
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
UgXmlrpcValue*	ug_xmlrpc_value_new  (void)
{
	return g_slice_new (UgXmlrpcValue);
}

void	ug_xmlrpc_value_free (UgXmlrpcValue* value)
{
	if (value->type == UG_XMLRPC_ARRAY || value->type == UG_XMLRPC_STRUCT)
		ug_xmlrpc_data_free (value->c.array);
	g_slice_free (UgXmlrpcValue, value);
}


// ----------------------------------------------------------------------------
// UgXmlrpcData: for XML-RPC array and struct.
//
UgXmlrpcData*	ug_xmlrpc_data_new (guint n)
{
	UgXmlrpcData*	xrdata;

	xrdata = g_slice_new0 (UgXmlrpcData);
	if (n) {
		xrdata->data = g_malloc (sizeof (UgXmlrpcValue) * n);
		xrdata->allocated = n;
	}

	return xrdata;
}

void	ug_xmlrpc_data_free (UgXmlrpcData* xrdata)
{
	UgXmlrpcValue*	value;
	UgXmlrpcValue*	end;

	value = xrdata->data;
	end   = value + xrdata->len;

	for (;  value < end;  value++) {
		if (value->type == UG_XMLRPC_ARRAY || value->type == UG_XMLRPC_STRUCT)
			ug_xmlrpc_data_free (value->c.array);
	}

	if (xrdata->tree)
		g_tree_destroy (xrdata->tree);
	g_slice_free (UgXmlrpcData, xrdata);
}

UgXmlrpcValue*	ug_xmlrpc_data_alloc (UgXmlrpcData* xrdata)
{
	UgXmlrpcValue*	value;

	if (xrdata->len == xrdata->allocated) {
		if (xrdata->allocated == 0)
			xrdata->allocated  = 16;
		else
			xrdata->allocated *= 2;

		xrdata->data = g_realloc (xrdata->data, sizeof (UgXmlrpcValue) * xrdata->allocated);
	}

	value = xrdata->data + xrdata->len++;
	value->name = NULL;
	return value;
}

UgXmlrpcValue*	ug_xmlrpc_data_find (UgXmlrpcData* xrdata, const gchar* name)
{
	UgXmlrpcValue*	value;
	UgXmlrpcValue*	end;


	if (xrdata->tree == NULL) {
		xrdata->tree = g_tree_new ((GCompareFunc) strcmp);
		value = xrdata->data;
		end   = value + xrdata->len;

		for (;  value < end;  value++)
			g_tree_insert (xrdata->tree, value->name, value);
	}

	return g_tree_lookup (xrdata->tree, name);
}

