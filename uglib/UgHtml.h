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

#ifndef UG_HTML_H
#define UG_HTML_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef	struct	UgHtmlContext			UgHtmlContext;
typedef	struct	UgHtmlParser			UgHtmlParser;
typedef	struct	UgHtmlFilter			UgHtmlFilter;

typedef void	(*UgHtmlFilterFunc)	(UgHtmlFilter* filter, const char* value, gpointer user_data);

// ----------------------------------------------------------------------------
// UgHtmlContext : simple and uncompleted parser.
struct	UgHtmlContext
{
	char*	base_href;
	char*	charset;

	const char*	element_name;
};

UgHtmlContext*	ug_html_context_new		(void);
void			ug_html_context_free	(UgHtmlContext* context);

void		ug_html_context_add_filter	(UgHtmlContext* context, UgHtmlFilter* filter);
void		ug_html_context_push_parser	(UgHtmlContext* context, UgHtmlParser* parser, gpointer user_data);
void		ug_html_context_pop_parser	(UgHtmlContext* context);
//gboolean	ug_html_context_end_parse	(UgHtmlContext* context);
gboolean	ug_html_context_parse		(UgHtmlContext* context, const char* buffer, int buffer_len);
gboolean	ug_html_context_parse_file	(UgHtmlContext* context, const char* file);


// ----------------------------------------------------------------------------
// UgHtmlParser

// This one is similar to GMarkupParser
struct UgHtmlParser
{
	/* Called for open tags <foo bar="baz"> */
	void	(*start_element) (UgHtmlContext* context,
	                          const char*    element_name,
	                          const char**   attribute_names,
	                          const char**   attribute_values,
	                          gpointer       user_data);

	/* Called for close tags </foo> */
	void	(*end_element)   (UgHtmlContext* context,
	                          const char*    element_name,
	                          gpointer       user_data);

	/* Called for character data */
	/* text is not nul-terminated */
	void	(*text)          (UgHtmlContext* context,
	                          const  char*   text,
	                          int            text_len,
	                          gpointer       user_data);
};


// ----------------------------------------------------------------------------
// UgHtmlFilter

struct UgHtmlFilter
{
	int		ref_count;

	char*	tag_name;		// element name
	char*	attr_name;		// attribute name
	GList*	attr_values;	// attribute values, string list

	UgHtmlContext*		context;

	// callback
	UgHtmlFilterFunc	callback;
	gpointer			callback_data;
};

UgHtmlFilter*	ug_html_filter_new (char* element_name, char* attr_name);

void	ug_html_filter_ref   (UgHtmlFilter* filter);
void	ug_html_filter_unref (UgHtmlFilter* filter);


#ifdef __cplusplus
}
#endif

#endif  // UG_HTML_H

