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

#ifndef UG_DOWNLOAD_WIDGET_H
#define UG_DOWNLOAD_WIDGET_H

#include <gtk/gtk.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgDownloadWidget			UgDownloadWidget;

// ----------------------------------------------------------------------------
// UgDownloadWidget
//
struct UgDownloadWidget
{
	GtkWidget*		self;			// (GtkScrolledWindow) container widget for view
	GtkTreeView*	view;
	GtkTreeModel*	model;			// GtkTreeModelFilter or GtkTreeModelSort

	gboolean		signal_connected;
	guint			changed_count;	// sync with UgDownloadColumnSetting.changed_count
};

void	ug_download_widget_init (UgDownloadWidget* dwidget, GtkTreeModel* model);
void	ug_download_widget_finalize (UgDownloadWidget* dwidget);

void	ug_download_widget_set_filter (UgDownloadWidget* dwidget, GtkTreeModel* model, GtkTreeModelFilterVisibleFunc func, gpointer data);
void	ug_download_widget_use_sortable (UgDownloadWidget* dwidget, GtkTreeModel* model);
// (UgDataset*) list->data
GList*	ug_download_widget_get_selected (UgDownloadWidget* dwidget);
// GPOINTER_TO_INT (link->data)
GList*	ug_download_widget_get_selected_indices (UgDownloadWidget* dwidget);
gint	ug_download_widget_count_selected (UgDownloadWidget* dwidget);

UgDataset*	ug_download_widget_get_cursor (UgDownloadWidget* dwidget);

// ----------------------------------------------------------------------------
// GtkTreeView for UgDataset

// GtkTreeViewColumn* column;
// column = gtk_tree_view_get_column (tree_view, UG_DOWNLOAD_COLUMN_NAME);
// gtk_tree_view_column_set_visible (column, TRUE);
enum UgDownloadViewColumn
{
	UG_DOWNLOAD_COLUMN_NAME,
	UG_DOWNLOAD_COLUMN_COMPLETE,
	UG_DOWNLOAD_COLUMN_SIZE,
	UG_DOWNLOAD_COLUMN_PERCENT,
	UG_DOWNLOAD_COLUMN_ELAPSED,		// consuming time
	UG_DOWNLOAD_COLUMN_LEFT,		// remaining time
	UG_DOWNLOAD_COLUMN_SPEED,
	UG_DOWNLOAD_COLUMN_UPLOAD_SPEED,// torrent
	UG_DOWNLOAD_COLUMN_UPLOADED,	// torrent
	UG_DOWNLOAD_COLUMN_RATIO,		// torrent
	UG_DOWNLOAD_COLUMN_RETRY,
	UG_DOWNLOAD_COLUMN_CATEGORY,	// category name
	UG_DOWNLOAD_COLUMN_URL,
	UG_DOWNLOAD_COLUMN_ADDED_ON,
	UG_DOWNLOAD_COLUMN_COMPLETED_ON,
	UG_DOWNLOAD_N_COLUMN
};

GtkTreeView*	ug_download_view_new (void);
void			ug_download_view_use_all_icon (GtkTreeView* view, gboolean visible_all);

#ifdef __cplusplus
}
#endif

#endif  // End of UG_DOWNLOAD_WIDGET_H

