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

#include <stdlib.h>
#include <string.h>

#include <UgUri.h>
#include <UgXmlrpc.h>
#include <curl/curl.h>


// ----------------------------------------------------------------------------
// UgXmlrpc: XML-RPC
//
static void		ug_xmlrpc_add_value  (UgXmlrpc* xmlrpc, UgXmlrpcValue* value);
static void		ug_xmlrpc_add_array  (UgXmlrpc* xmlrpc, UgXmlrpcValue* value);
static void		ug_xmlrpc_add_struct (UgXmlrpc* xmlrpc, UgXmlrpcValue* value);
static size_t	ug_xmlrpc_curl_write (char* buffer, size_t size, size_t nmemb, UgXmlrpc* xmlrpc);
static void		ug_str_unescape_text (gchar* text);
// parser
static void		ug_xmltag_parse_unknown (UgXmltag* xmltag, gpointer data);
static void		ug_xmltag_parse_response (UgXmltag* xmltag, UgXmlrpc* xmlrpc);
static void		ug_xmltag_parse_param_top (UgXmltag* xmltag, UgXmlrpcValue* value);
static void		ug_xmltag_parse_value (UgXmltag* xmltag, UgXmlrpcValue* value);
static void		ug_xmltag_parse_value_top (UgXmltag* xmltag, UgXmlrpcValue* value);
static void		ug_xmltag_parse_array (UgXmltag* xmltag, UgXmlrpcValue* value);
static void		ug_xmltag_parse_array_data (UgXmltag* xmltag, UgXmlrpcValue* value);
static void		ug_xmltag_parse_struct (UgXmltag* xmltag, UgXmlrpcValue* value);
static void		ug_xmltag_parse_struct_member (UgXmltag* xmltag, UgXmlrpcValue* value);


void	ug_xmlrpc_init (UgXmlrpc* xmlrpc)
{
	xmlrpc->curl = curl_easy_init ();
	xmlrpc->uri        = NULL;
	xmlrpc->user_agent = NULL;

	xmlrpc->buffer = g_string_sized_new (1024);
	xmlrpc->chunk  = g_string_chunk_new (4096);

	ug_xmltag_init (&xmlrpc->xmltag);
	ug_xmlrpc_value_init (&xmlrpc->value);
}

void	ug_xmlrpc_finalize (UgXmlrpc* xmlrpc)
{
	curl_easy_cleanup (xmlrpc->curl);
	g_free (xmlrpc->uri);
	g_free (xmlrpc->user_agent);

	g_string_free (xmlrpc->buffer, TRUE);
	g_string_chunk_free (xmlrpc->chunk);

	ug_xmltag_finalize (&xmlrpc->xmltag);
	ug_xmlrpc_value_clear (&xmlrpc->value);
}

void	ug_xmlrpc_use_client (UgXmlrpc* xmlrpc, const gchar* uri, const gchar* user_agent)
{
	xmlrpc->uri = g_strdup (uri);
	xmlrpc->user_agent = g_strdup (user_agent ? user_agent : "uGet/1.7");

	curl_easy_setopt (xmlrpc->curl, CURLOPT_URL, xmlrpc->uri);
	curl_easy_setopt (xmlrpc->curl, CURLOPT_USERAGENT, xmlrpc->user_agent);
}

static size_t	ug_xmlrpc_curl_write (char* buffer, size_t size, size_t nmemb, UgXmlrpc* xmlrpc)
{
	size = size * nmemb;
	ug_xmltag_parse (&xmlrpc->xmltag, buffer, size);
	return size;
}

// ----------------------------------------------------------------------------
// functions used to write UgXmlrpc.buffer
//
UgXmlrpcResponse	ug_xmlrpc_call (UgXmlrpc* xmlrpc, const gchar* methodName, ...)
{
	GString*	buffer;
	va_list		args;
	union {
		long			response;
		gchar*			str;
		UgXmlrpcValue*	value;
	} temp;

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

	temp.response = 0;
	xmlrpc->index = 0;
	xmlrpc->response = UG_XMLRPC_OK;
	xmlrpc->xmltag.user.storage = xmlrpc->chunk;
	g_string_chunk_clear (xmlrpc->chunk);
	ug_xmlrpc_value_clear (&xmlrpc->value);
	ug_xmltag_push (&xmlrpc->xmltag, (UgXmltagFunc) ug_xmltag_parse_response, xmlrpc);

	curl_easy_setopt  (xmlrpc->curl, CURLOPT_POST, TRUE);
	curl_easy_setopt  (xmlrpc->curl, CURLOPT_POSTFIELDS, buffer->str);
	curl_easy_setopt  (xmlrpc->curl, CURLOPT_POSTFIELDSIZE, buffer->len);
	curl_easy_setopt  (xmlrpc->curl, CURLOPT_WRITEFUNCTION,
			(curl_write_callback) ug_xmlrpc_curl_write);
	curl_easy_setopt  (xmlrpc->curl, CURLOPT_WRITEDATA, xmlrpc);
	curl_easy_perform (xmlrpc->curl);

	curl_easy_getinfo (xmlrpc->curl, CURLINFO_RESPONSE_CODE, &temp.response);
	if (temp.response != 200)
		return UG_XMLRPC_ERROR;
	if (ug_xmltag_clear (&xmlrpc->xmltag) == FALSE)
		return UG_XMLRPC_ERROR;

	return xmlrpc->response;
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
UgXmlrpcValue*	ug_xmlrpc_get_value (UgXmlrpc* xmlrpc)
{
	switch (xmlrpc->response) {
	case UG_XMLRPC_OK:
		if (xmlrpc->index < xmlrpc->value.len)
			return xmlrpc->value.data + xmlrpc->index++;
		else
			return NULL;

	case UG_XMLRPC_FAULT:
		return &xmlrpc->value;

	default:
//	case UG_XMLRPC_ERROR:
		break;
	}

	return NULL;
}

// parser ---------
static void	ug_xmltag_parse_unknown (UgXmltag* xmltag, gpointer data)
{
	ug_xmltag_push (xmltag, ug_xmltag_parse_unknown, NULL);
}

static void	ug_xmltag_parse_response (UgXmltag* xmltag, UgXmlrpc* xmlrpc)
{
	switch (xmltag->beg[0]) {
	// methodResponse
	case 'm':
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_response, xmlrpc);
		break;

	// params
	case 'p':
		xmlrpc->response = UG_XMLRPC_OK;
		xmlrpc->value.type = UG_XMLRPC_PARAMS;
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_param_top,
				ug_xmlrpc_value_alloc (&xmlrpc->value));
		break;

	// fault
	case 'f':
		xmlrpc->response = UG_XMLRPC_FAULT;
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_value_top, &xmlrpc->value);
		break;

	default:
		ug_xmltag_push (xmltag, ug_xmltag_parse_unknown, NULL);
	}
}

static void	ug_xmltag_parse_param_top (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (memcmp (xmltag->beg, "param", xmltag->end - xmltag->beg) == 0)
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_value_top, value);
	else
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_unknown, value);
}

static void	ug_xmltag_parse_value (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	const char*	tag  = xmltag->beg;
	const char*	text = xmltag->end + 1;
	gsize		temp;

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
		if (memcmp (tag, "struct", xmltag->end - xmltag->beg) == 0) {
			value->type = UG_XMLRPC_STRUCT;
			ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_struct, value);
			return;
		}
		else {
			value->type = UG_XMLRPC_STRING;
			value->c.string = g_string_chunk_insert_len (xmltag->user.storage, text, xmltag->next - text);
			ug_str_unescape_text (value->c.string);
		}
		break;

	case 'b':	// boolean or base64
		if (tag[1] == 'o') {
			value->type = UG_XMLRPC_BOOLEAN;
			value->c.boolean = atoi (text);
		}
		else {
			value->type = UG_XMLRPC_BINARY;
			value->c.string = g_string_chunk_insert_len (xmltag->user.storage, text, xmltag->next - text);
			value->c.binary = g_base64_decode_inplace (value->c.string, &temp);
			value->len = temp;
		}
		break;

	case 'd':	// double  or dateTime.iso8601
		if (tag[1] == 'o')
			value->c.double_ = atof (text);
		else
			value->c.datetime = g_string_chunk_insert_len (xmltag->user.storage, text, xmltag->next - text);
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

	ug_xmltag_push (xmltag, ug_xmltag_parse_unknown, NULL);
}

static void	ug_xmltag_parse_value_top (UgXmltag* xmltag, UgXmlrpcValue* value)
{
//	if (xmltag->next && xmltag->next[1] == '/') {
//		value->type = UG_XMLRPC_STRING;
//		value->c.string = g_string_chunk_insert_len (xmltag->user.storage,
//				xmltag->end+1, xmltag->next - (xmltag->end+1));
//	}
	ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_value, value);
}

static void	ug_xmltag_parse_array (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (memcmp (xmltag->beg, "data", xmltag->end - xmltag->beg) == 0)
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_array_data, value);
	else
		ug_xmltag_push (xmltag, ug_xmltag_parse_unknown, NULL);
}

static void	ug_xmltag_parse_array_data (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	ug_xmltag_parse_value_top (xmltag, ug_xmlrpc_value_alloc (value));
}

static void	ug_xmltag_parse_struct (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	if (memcmp (xmltag->beg, "member", xmltag->end - xmltag->beg) == 0) {
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_struct_member,
				ug_xmlrpc_value_alloc (value));
	}
	else
		ug_xmltag_push (xmltag, (UgXmltagFunc) ug_xmltag_parse_unknown, NULL);
}

static void	ug_xmltag_parse_struct_member (UgXmltag* xmltag, UgXmlrpcValue* value)
{
	switch (xmltag->beg[0]) {
	// name
	case 'n':
		if (xmltag->next) {
			value->name = g_string_chunk_insert_len (xmltag->user.storage,
					xmltag->end+1, xmltag->next - (xmltag->end+1));
		}
		ug_xmltag_push (xmltag, ug_xmltag_parse_unknown, NULL);
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

	xmltag->buffer = g_string_sized_new (2048);
}

void	ug_xmltag_finalize (UgXmltag* xmltag)
{
	g_string_free (xmltag->buffer, TRUE);
	g_free (xmltag->parser.data);
}

static void	ug_xmltag_parse_in (UgXmltag* xmltag, const char* string, int len, gboolean terminate)
{
	gpointer*		parser;
	UgXmltagFunc	func;
	gpointer		data;

	xmltag->beg = string;	// the next character of '<'
	string = string + len;	// point to end

	for (;;) {
		if (xmltag->parser.len == 0)
			return;
		if ((xmltag->end = memchr (xmltag->beg, '>', string - xmltag->beg)) == NULL)
			break;
		if ((xmltag->next = memchr (xmltag->end+1, '<', string - xmltag->end -1)) == NULL && terminate == FALSE)
			break;

		switch (xmltag->beg[0]) {
		case '/':
			ug_xmltag_pop (xmltag);
			break;

		case '?':
			break;

		default:
			parser = xmltag->parser.data + xmltag->parser.len;
			func = *(parser-2);
			data = *(parser-1);
			func (xmltag, data);
			if (xmltag->end[-1] == '/')
				ug_xmltag_pop (xmltag);
			break;
		}

		if (xmltag->next == NULL)
			break;
		xmltag->beg = xmltag->next + 1;
	}
}

gboolean	ug_xmltag_parse (UgXmltag* xmltag, const gchar* string, int len)
{
	GString*		buffer;
	const gchar*	temp;

	buffer = xmltag->buffer;
	temp = memchr (string, '<', len);

	if (buffer->len) {
		if (temp == NULL) {
			g_string_append_len (buffer, string, len);
			return TRUE;
		}
		else {
			g_string_append_len (buffer, string, temp +1 -string);
			ug_xmltag_parse_in (xmltag, buffer->str, buffer->len, FALSE);
			buffer->len = 0;
		}
	}

	if (temp) {
		temp++;					// the next character of '<'
		string = string + len;	// point to end
		ug_xmltag_parse_in (xmltag, temp, string -temp, FALSE);
		g_string_overwrite_len (buffer, 0, xmltag->beg, string - xmltag->beg);
	}

	return TRUE;
}

gboolean	ug_xmltag_clear	(UgXmltag* xmltag)
{
	gboolean	result;

	ug_xmltag_parse_in (xmltag, xmltag->buffer->str, xmltag->buffer->len, TRUE);
	xmltag->buffer->len = 0;

	// Because user push parser and data before running ug_xmltag_parse(),
	// xmltag->parser.len must be 2
	if (xmltag->parser.len == 2)
		result = TRUE;
	else
		result = FALSE;

	xmltag->parser.len = 0;
	return result;
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

