/*
 *
 *   Copyright (C) 2005-2014 by C.H. Huang
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

#ifndef UG_SELECTOR_H
#define UG_SELECTOR_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct	UgSelector			UgSelector;
typedef struct	UgSelectorPage		UgSelectorPage;
typedef	void	(*UgSelectorNotify) (gpointer user_data, gboolean completed);

// ----------------------------------------------------------------------------
// UgSelector
//
struct UgSelector
{
	GtkWidget*		self;		// GtkVBox
	GtkWindow*		parent;		// parent window of UgSelector.self

	GtkNotebook*	notebook;
	// <base href>
	GtkWidget*		href_label;
	GtkEntry*		href_entry;		// entry for hypertext reference
	GtkWidget*		href_separator;
	// select button
	GtkWidget*		select_all;
	GtkWidget*		select_none;
	GtkWidget*		select_filter;	// select by filter

	// UgSelectorPage is placed in array
	GArray*			pages;

	// UgSelectorFilter use UgSelectorFilterData in UgSelectorPage
	struct UgSelectorFilter
	{
		GtkDialog*		dialog;
		GtkTreeView*	host_view;
		GtkTreeView*	ext_view;
	} filter;

	// callback
	struct
	{
		UgSelectorNotify	func;
		gpointer			data;
	} notify;
};

void	ug_selector_init (UgSelector* selector, GtkWindow* parent);
void	ug_selector_finalize (UgSelector* selector);

void	ug_selector_hide_href (UgSelector* selector);

// (UgDataset*) list->data.
// To free the returned value, use:
//	g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
//	g_list_free (list);
GList*	ug_selector_get_marked_downloads (UgSelector* selector);

// count marked item and notify
gint	ug_selector_count_marked (UgSelector* selector);

UgSelectorPage*	ug_selector_add_page (UgSelector* selector, const gchar* title);
UgSelectorPage*	ug_selector_get_page (UgSelector* selector, gint nth_page);

// ----------------------------------------------------------------------------
// UgSelectorPage
//
struct UgSelectorPage
{
	GtkWidget*		self;	// GtkScrolledWindow

	GtkTreeView*	view;
	GtkListStore*	store;

	// total marked count
	gint			n_marked;

	// used by UgSelectorFilter
	struct UgSelectorFilterData
	{
		GHashTable*		hash;
		GtkListStore*	host;
		GtkListStore*	ext;
	} filter;
};

void	ug_selector_page_init (UgSelectorPage* page);
void	ug_selector_page_finalize (UgSelectorPage* page);

void	ug_selector_page_add_uris (UgSelectorPage* page, GList* uris);
void	ug_selector_page_add_downloads (UgSelectorPage* page, GList* downloads);
void	ug_selector_page_make_filter (UgSelectorPage* page);
void	ug_selector_page_mark_by_filter_all (UgSelectorPage* page);


#ifdef __cplusplus
}
#endif

#endif  // End of UG_SELECTOR_H

