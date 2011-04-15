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


#ifndef UG_CATEGORY_GTK_H
#define UG_CATEGORY_GTK_H

#include <gtk/gtk.h>
#include <UgCategory.h>
#include <UgDownloadWidget.h>

#ifdef __cplusplus
extern "C" {
#endif


#define	UG_CATEGORY_GTK(category)			((UgCategoryGtk*) (category)->user.category)
#define	UG_RELATION_GTK_CATEGORY(relation)	((UgCategoryGtk*) (relation)->user.category)
#define	UG_RELATION_GTK_LISTSTORE(relation)	((GtkListStore*)  (relation)->user.storage)
#define	UG_RELATION_GTK_TREEITER(relation)	((GtkTreeIter*)   (relation)->user.position)
#define	UG_RELATION_GTK_DLWIDGET(relation)	((UgDownloadWidget*) (relation)->user.data)


// ----------------------------------------------------------------------------
// UgCategoryGtk: additional data for UgCategory and GTK+
//
typedef struct	UgCategoryGtk			UgCategoryGtk;

struct UgCategoryGtk
{
	UgCategoryGtk*		primary;	// if it is NULL, this category is primary.
	GtkListStore*		store;		// Only primary category has this.
	GtkTreeModel*		filter;		// model or filter for primary.store

	UgDownloadWidget	all;		// GtkTreeModelSort
	UgDownloadWidget	active;		// GtkTreeModelFilter
	UgDownloadWidget	queuing;	// GtkTreeModelFilter
	UgDownloadWidget	finished;	// GtkTreeModelFilter
	UgDownloadWidget	recycled;	// GtkTreeModelFilter

	// iterator for UgCategoryWidget.store
	GtkTreeIter			tree_iter;
};

// If primary == NULL, this category will be a primary category.
// One uget-gtk program has only one primary category.
// All dataset store in primary category.
// Other categories use filter to classify dataset in primary category.
UgCategory*		ug_category_new_with_gtk (UgCategory* primary);

// set additional data and functions to UgCategory.
void	ug_category_use_gtk		(UgCategory* category, UgCategory* primary);

// functions will be used by UgCategoryFuncs (UgCategory::funcs)
void	ug_category_gtk_add (UgCategory* category, UgDataset* dataset);
GList*	ug_category_gtk_get_all (UgCategory* category);
GList*	ug_category_gtk_get_tasks (UgCategory* category);
void	ug_category_gtk_changed (UgCategory* category, UgDataset* dataset);

// other functions used by uget-gtk
void	ug_category_gtk_remove (UgCategory* category, UgDataset* dataset);
void	ug_category_gtk_clear  (UgCategory* category, UgCategoryHints hint, guint from_nth);

void	ug_category_gtk_move_to (UgCategory* category, UgDataset* dataset, UgCategory* cdest);

gboolean	ug_category_gtk_move_selected_up (UgCategory* category, UgDownloadWidget* dlwidget);
gboolean	ug_category_gtk_move_selected_down (UgCategory* category, UgDownloadWidget* dlwidget);
gboolean	ug_category_gtk_move_selected_to_top (UgCategory* category, UgDownloadWidget* dlwidget);
gboolean	ug_category_gtk_move_selected_to_bottom (UgCategory* category, UgDownloadWidget* dlwidget);

// If no data cleared, return FALSE.
gboolean	ug_category_gtk_clear_excess (UgCategory* category);


#ifdef __cplusplus
}
#endif

#endif  // UG_CATEGORY_GTK_H

