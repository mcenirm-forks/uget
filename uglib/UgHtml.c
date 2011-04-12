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

#include <string.h>
#include <UgStdio.h>
#include <UgHtml.h>


#define	TAG_LEN_MAX		4095

typedef	struct	UgHtmlContextPrivate		UgHtmlContextPrivate;
typedef	enum	UgHtmlContextState			UgHtmlContextState;
typedef	enum	UgHtmlTagState				UgHtmlTagState;

/* Called for open tags <foo bar="baz"> */
static void	cb_start_element (UgHtmlContextPrivate* context,
	                          const char*    element_name,
	                          const char**   attribute_names,
	                          const char**   attribute_values,
	                          gpointer       user_data);

/* Called for close tags </foo> */
static void	cb_end_element   (UgHtmlContext* context,
	                          const char*    element_name,
	                          gpointer       user_data);

/* Called for character data */
/* text is not nul-terminated */
static void	cb_text          (UgHtmlContext* context,
	                          const  char*   text,
	                          int            text_len,
	                          gpointer       user_data);

enum UgHtmlContextState
{
	UG_HTML_CONTEXT_NULL,
	UG_HTML_CONTEXT_TEXT,
	UG_HTML_CONTEXT_TAG,
//	UG_HTML_CONTEXT_TAG_PASS,	// <!--pass-->
};

enum UgHtmlTagState
{
	UG_HTML_TAG_ERROR,	// (0 << 0)
	UG_HTML_TAG_START,	// (1 << 0)
	UG_HTML_TAG_END,	// (1 << 1)
};

// ----------------------------------------------------------------------------
// UgHtmlContextPrivate : simple and uncompleted parser.
struct	UgHtmlContextPrivate
{
	char*	base_href;
	char*	charset;

	const char*	element_name;

	// index 0, 2, 4, 6... : UgHtmlParser*	parser
	// index 1, 3, 5, 7... : gpointer		user_data
	GPtrArray*			parser;

	// attribute name & value
	// used by ug_html_context_parse_tag()
	GPtrArray*			attr_names;
	GPtrArray*			attr_values;

	UgHtmlContextState	state;
	UgHtmlTagState		tag_state;

//	GHashTable*	hash_table;
//	key = tag_name, value = list of UgHtmlFilter
	GList*	filter;

	int		tag_len;
	char	tag[TAG_LEN_MAX + 1];
};

static UgHtmlParser	default_parser =
{
	(gpointer) cb_start_element,
	(gpointer) cb_end_element,
	(gpointer) cb_text,
};

UgHtmlContext*	ug_html_context_new (void)
{
	UgHtmlContextPrivate*	context;

	context = g_malloc (sizeof (UgHtmlContextPrivate));
	context->base_href	= NULL;
	context->charset	= NULL;
	context->element_name = context->tag;

	context->parser		= g_ptr_array_sized_new (16);
	context->attr_names	= g_ptr_array_sized_new (16);
	context->attr_values= g_ptr_array_sized_new (16);
	context->state		= UG_HTML_CONTEXT_NULL;
	context->filter		= NULL;
	context->tag[0]		= 0;
	context->tag_len	= 0;

	ug_html_context_push_parser ((UgHtmlContext*)context, &default_parser, NULL);
	return (UgHtmlContext*) context;
}

void	ug_html_context_free (UgHtmlContext* context_pub)
{
	UgHtmlContextPrivate*	context = (UgHtmlContextPrivate*) context_pub;

	g_free (context->base_href);
	g_free (context->charset);
	g_ptr_array_free (context->parser, TRUE);
	g_ptr_array_free (context->attr_names,  TRUE);
	g_ptr_array_free (context->attr_values, TRUE);
	g_list_foreach (context->filter, (GFunc) ug_html_filter_unref, NULL);
	g_list_free (context->filter);
	g_free (context);
}

void	ug_html_context_add_filter (UgHtmlContext* context_pub, UgHtmlFilter* filter)
{
	UgHtmlContextPrivate*	context = (UgHtmlContextPrivate*) context_pub;

	context->filter = g_list_prepend (context->filter, filter);
	ug_html_filter_ref (filter);
	filter->context = context_pub;
}

void	ug_html_context_push_parser (UgHtmlContext* context_pub, UgHtmlParser* parser, gpointer user_data)
{
	UgHtmlContextPrivate*	context = (UgHtmlContextPrivate*) context_pub;

	g_ptr_array_add (context->parser, parser);
	g_ptr_array_add (context->parser, user_data);
}

void	ug_html_context_pop_parser (UgHtmlContext* context_pub)
{
	UgHtmlContextPrivate*	context = (UgHtmlContextPrivate*) context_pub;
	GPtrArray*				parser_array;

	parser_array = context->parser;
	if (parser_array->len >= 2)
		g_ptr_array_set_size (parser_array, parser_array->len -2);
}

// parse context->tag, separate attribute names and values.
// output: context->element_name, context->attr_names, context->attr_values, context->tag_state
static void	ug_html_context_parse_tag (UgHtmlContextPrivate* context)
{
	gchar*   attr_name		= NULL;
	gchar*   attr_value		= NULL;
	gchar*   tag_cur;
	gchar    inside_chr		= 0;
	gint     inside_level;

	g_ptr_array_set_size (context->attr_names,  0);
	g_ptr_array_set_size (context->attr_values, 0);

	// tag name
	for (tag_cur=context->tag;  tag_cur[0];  tag_cur++) {
		if (tag_cur[0] == ' ') {
			tag_cur[0] = 0;
			tag_cur++;
			break;
		}
	}
	if (context->tag[0] == 0) {
		context->tag_state = UG_HTML_TAG_ERROR;
		return;
	}
	if (context->tag[0] == '/') {
		context->element_name = context->tag + 1;
		context->tag_state = UG_HTML_TAG_END;
		return;
	}

	// attribute names & values
	while (tag_cur[0]) {
		// skip space
		while (tag_cur[0] == ' ')
			tag_cur++;

		// attribute name
		attr_name  = tag_cur;
		for(; tag_cur[0]; tag_cur++) {
			if (tag_cur[0] == '=') {
				tag_cur[0] = 0;				// null-terminated
				tag_cur++;
				break;
			}
			else if (tag_cur[0] == ' ') {	// wrong attribute
				attr_name = NULL;
				break;
			}
		}
		if (attr_name==NULL)
			continue;

		// attribute value
		attr_value = tag_cur;
		inside_level = 0;
		inside_chr   = 0;
		for (; ; tag_cur++) {
			switch (tag_cur[0]) {
			case 0:
				goto break_forLoop;

			case '"':
			case '\'':
				// handle <tag attr='"'"value"'"'>
				if (inside_chr == tag_cur[0]) {
					inside_chr = (inside_chr=='"') ? '\'' : '"';
					inside_level--;
				}
				else {
					if (inside_level==0)
						attr_value = tag_cur +1;	// ignore first character
					inside_chr = tag_cur[0];
					inside_level++;
				}

				if (inside_level==0) {
					tag_cur[0] = 0;	// null-terminated
					tag_cur++;
					goto break_forLoop;
				}
				break;

			case '/':
				if (inside_level == 0) {
					inside_chr = '/';
					tag_cur[0] = 0;
//					tag_cur++;
					goto break_forLoop;
				}
				break;

			case ' ':
				if (inside_level == 0) {
					tag_cur[0] = 0;
					tag_cur++;
					goto break_forLoop;
				}
			default:
				break;
			}
		}
break_forLoop:
		// add attribute name & value
		g_ptr_array_add (context->attr_names,  attr_name);
		g_ptr_array_add (context->attr_values, attr_value);
	}

	// null-terminated
	g_ptr_array_add (context->attr_names,  NULL);
	g_ptr_array_add (context->attr_values, NULL);

	context->element_name = context->tag;
	if (inside_chr == '/')
		context->tag_state = UG_HTML_TAG_START | UG_HTML_TAG_END;
	else
		context->tag_state = UG_HTML_TAG_START;
}

gboolean	ug_html_context_parse (UgHtmlContext* context_pub, const char* buffer, int buffer_len)
{
	UgHtmlContextPrivate*	context = (UgHtmlContextPrivate*) context_pub;
	UgHtmlParser*	parser;
	gpointer		user_data;
	const char*		buffer_cur;
	const char*		buffer_end;

	if (buffer_len == -1)
		buffer_len = strlen (buffer);
	buffer_end = buffer + buffer_len;

	for (buffer_cur = buffer;  buffer_cur < buffer_end;  buffer_cur++) {
		switch (*buffer_cur) {
		case '<':
			if (context->state == UG_HTML_CONTEXT_TEXT && buffer < buffer_cur) {
				parser    = g_ptr_array_index (context->parser, context->parser->len -2);
				user_data = g_ptr_array_index (context->parser, context->parser->len -1);
				if (parser->text)
					parser->text (context_pub, buffer, buffer_cur -buffer, user_data);
			}
			context->state = UG_HTML_CONTEXT_TAG;
			context->tag_len = 0;
			break;

		case '>':
			if (context->state == UG_HTML_CONTEXT_TAG) {
				context->tag[context->tag_len++] = 0;	// null-terminated
				ug_html_context_parse_tag (context);
				if (context->tag_state & UG_HTML_TAG_START) {
					parser    = g_ptr_array_index (context->parser, context->parser->len -2);
					user_data = g_ptr_array_index (context->parser, context->parser->len -1);
					if (parser->start_element) {
						parser->start_element ((UgHtmlContext*) context, context->tag,
		                       (const char**) context->attr_names->pdata,
		                       (const char**) context->attr_values->pdata,
		                       user_data);
					}
				}
				if (context->tag_state & UG_HTML_TAG_END) {
					parser    = g_ptr_array_index (context->parser, context->parser->len -2);
					user_data = g_ptr_array_index (context->parser, context->parser->len -1);
					if (parser->end_element)
						parser->end_element ((UgHtmlContext*) context, context->element_name, user_data);
				}
			}
			context->state = UG_HTML_CONTEXT_TEXT;
			context->tag_len = 0;
			buffer = buffer_cur + 1;
			break;

		case '\r':
		case '\n':
			// skip
			break;

		default:
			if (context->state == UG_HTML_CONTEXT_TAG) {
				if (context->tag_len == TAG_LEN_MAX) {
					context->state = UG_HTML_CONTEXT_NULL;
					context->tag_len = 0;
				}
				else
					context->tag[context->tag_len++] = *buffer_cur;
			}
		}
	}

	// text
	if (context->state == UG_HTML_CONTEXT_TEXT && buffer < buffer_cur) {
		parser    = g_ptr_array_index (context->parser, context->parser->len -2);
		user_data = g_ptr_array_index (context->parser, context->parser->len -1);
		if (parser->text)
			parser->text (context_pub, buffer, buffer_cur -buffer, user_data);
	}

	return TRUE;
}

gboolean	ug_html_context_parse_file (UgHtmlContext* context, const char* file_utf8)
{
	char*	buf;
	gint	buf_len;
	gint	fd;

//	fd = open (file_utf8, O_RDONLY | O_TEXT, S_IREAD);
	fd = ug_fd_open (file_utf8, UG_FD_O_READONLY | UG_FD_O_TEXT, UG_FD_S_IREAD);
	if (fd == -1)
		return FALSE;

	buf = g_malloc (4096);
	for (;;) {
//		buf_len = read (fd, buf, 4096);
		buf_len = ug_fd_read (fd, buf, 4096);
		if (buf_len <= 0)
			break;
		ug_html_context_parse (context, buf, buf_len);
	}
	g_free (buf);

//	close (fd);
	ug_fd_close (fd);

	return TRUE;
}


// ----------------------------------------------------------------------------
// UgHtmlFilter

UgHtmlFilter*	ug_html_filter_new (char* element_name, char* attr_name)
{
	UgHtmlFilter*	filter;

	filter = g_malloc0 (sizeof (UgHtmlFilter));
	filter->tag_name  = g_strdup (element_name);
	filter->attr_name = g_strdup (attr_name);
//	filter->attr_values = NULL;
	filter->ref_count = 1;
	return filter;
}

void	ug_html_filter_ref   (UgHtmlFilter* filter)
{
	filter->ref_count++;
}

void	ug_html_filter_unref (UgHtmlFilter* filter)
{
	filter->ref_count--;
	if (filter->ref_count == 0) {
		g_free (filter->tag_name);
		g_free (filter->attr_name);
		g_list_foreach (filter->attr_values, (GFunc) g_free, NULL);
		g_free (filter);
	}
}


// ----------------------------------------------------------------------------
// parser

/* Called for open tags <foo bar="baz"> */
static void	cb_start_element (UgHtmlContextPrivate* context,
	                          const char*    element_name,
	                          const char**   attribute_names,
	                          const char**   attribute_values,
	                          gpointer       user_data)
{
	UgHtmlFilter*	filter;
	GList*			link;
	gchar*			string;

	// get charset
	if (context->charset == NULL && g_ascii_strcasecmp (element_name, "meta") == 0) {
		for (; *attribute_names; attribute_names++, attribute_values++) {
			if (g_ascii_strcasecmp (*attribute_names, "content") != 0)
				continue;
			string = strstr (*attribute_values, "charset=");
			if (string == NULL)
				continue;
			g_free (context->charset);
			context->charset = g_strdup (string + 8);
			break;
		}
		return;
	}

	// base href
	if (g_ascii_strcasecmp (element_name, "base") == 0) {
		for (; *attribute_names; attribute_names++, attribute_values++) {
			if (g_ascii_strcasecmp (*attribute_names, "href") != 0)
				continue;
			g_free (context->base_href);
			context->base_href = g_strdup (*attribute_values);
			break;
		}
		return;
	}

	// filter
	for (link = context->filter;  link;  link = link->next) {
		filter = link->data;
		if (g_ascii_strcasecmp (element_name, filter->tag_name) != 0)
			continue;
		for (; *attribute_names; attribute_names++, attribute_values++) {
			if (g_ascii_strcasecmp (*attribute_names, filter->attr_name) != 0)
				continue;
			if (filter->callback)
				filter->callback (filter, *attribute_values, filter->callback_data);
			else
				filter->attr_values = g_list_prepend (filter->attr_values, g_strdup (*attribute_values));
		}
	}
}

/* Called for close tags </foo> */
static void	cb_end_element   (UgHtmlContext* context,
	                          const char*    element_name,
	                          gpointer       user_data)
{
}

/* Called for character data */
/* text is not nul-terminated */
static void	cb_text          (UgHtmlContext* context,
	                          const  char*   text,
	                          int            text_len,
	                          gpointer       user_data)
{
}

