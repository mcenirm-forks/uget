/*
 *
 *   Copyright (C) 2005-2012 by C.H. Huang
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

#include <UgCategoryWidget.h>

#include <glib/gi18n.h>


#define	UG_CATEGORY_GTK_ALL_NAME			_("All")
#define	UG_CATEGORY_GTK_ACTIVE_NAME			_("Active")
#define	UG_CATEGORY_GTK_QUEUING_NAME		_("Queuing")
#define	UG_CATEGORY_GTK_FINISHED_NAME		_("Finished")
#define	UG_CATEGORY_GTK_RECYCLED_NAME		_("Recycled")

// This is used by GtkTreeStore
enum UgCategoryStoreColumn
{
	UG_CATEGORY_STORE_INSTANCE,		// UgCategory
	UG_CATEGORY_STORE_DOWNLOAD,		// UgDownloadWidget
	UG_CATEGORY_N_COLUMN
};

// signal handler
static void	ug_category_cursor_changed (GtkTreeView* view, UgCategoryWidget* cwidget);
// Function used by GtkTreeModelFilter.
static gboolean	ug_category_model_filter (GtkTreeModel *model, GtkTreeIter *iter, gpointer data);

// ----------------------------------------------------------------------------
// UgCategoryWidget
//
void	ug_category_widget_init (UgCategoryWidget* cwidget)
{
	GtkScrolledWindow*	scroll;
	GtkListStore*		primary_store;
	GtkTreeIter			iter;
	GtkBox*				vbox;

	// primary category
	cwidget->primary.category = ug_category_new_with_gtk (NULL);
	cwidget->primary.category->name = g_strdup (UG_CATEGORY_GTK_ALL_NAME);
	cwidget->primary.cgtk = cwidget->primary.category->user.category;
	// primary view & store
	cwidget->primary.view = ug_category_view_new ();
	primary_store = gtk_list_store_new (UG_CATEGORY_N_COLUMN, G_TYPE_POINTER, G_TYPE_POINTER);
	gtk_tree_view_set_model (cwidget->primary.view, GTK_TREE_MODEL (primary_store));
	g_object_unref (primary_store);

	gtk_list_store_append (primary_store, &iter);
	gtk_list_store_set (primary_store, &iter,
			UG_CATEGORY_STORE_INSTANCE, cwidget->primary.category,
			UG_CATEGORY_STORE_DOWNLOAD, &cwidget->primary.cgtk->all,
			-1);
	gtk_list_store_append (primary_store, &iter);
	gtk_list_store_set (primary_store, &iter,
			UG_CATEGORY_STORE_INSTANCE, cwidget->primary.category,
			UG_CATEGORY_STORE_DOWNLOAD, &cwidget->primary.cgtk->active,
			-1);
	gtk_list_store_append (primary_store, &iter);
	gtk_list_store_set (primary_store, &iter,
			UG_CATEGORY_STORE_INSTANCE, cwidget->primary.category,
			UG_CATEGORY_STORE_DOWNLOAD, &cwidget->primary.cgtk->queuing,
			-1);
	gtk_list_store_append (primary_store, &iter);
	gtk_list_store_set (primary_store, &iter,
			UG_CATEGORY_STORE_INSTANCE, cwidget->primary.category,
			UG_CATEGORY_STORE_DOWNLOAD, &cwidget->primary.cgtk->finished,
			-1);
	gtk_list_store_append (primary_store, &iter);
	gtk_list_store_set (primary_store, &iter,
			UG_CATEGORY_STORE_INSTANCE, cwidget->primary.category,
			UG_CATEGORY_STORE_DOWNLOAD, &cwidget->primary.cgtk->recycled,
			-1);

	// secondary categories: model and view
	cwidget->store = gtk_tree_store_new (UG_CATEGORY_N_COLUMN, G_TYPE_POINTER, G_TYPE_POINTER);
	cwidget->view = ug_category_view_new ();
	gtk_tree_view_set_model (cwidget->view, GTK_TREE_MODEL (cwidget->store));
	// secondary categories: scrolled for view
	cwidget->scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (cwidget->scroll, 165, 100);
	scroll = GTK_SCROLLED_WINDOW (cwidget->scroll);
	gtk_scrolled_window_set_shadow_type (scroll, GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scroll), GTK_WIDGET (cwidget->view));
	// filter for UgDownloadDialog
	cwidget->filter = gtk_tree_model_filter_new (
			GTK_TREE_MODEL (cwidget->store), NULL);
	gtk_tree_model_filter_set_visible_func (
			GTK_TREE_MODEL_FILTER (cwidget->filter),
			ug_category_model_filter, NULL, NULL);

	// container widget
	cwidget->self = gtk_vbox_new (FALSE, 2);
	vbox = GTK_BOX (cwidget->self);
	gtk_box_pack_start (vbox, GTK_WIDGET (cwidget->primary.view), FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, gtk_label_new (_("Category")), FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, GTK_WIDGET (cwidget->scroll), TRUE, TRUE, 0);
	gtk_widget_show_all (cwidget->self);

	// signal
	g_signal_connect (cwidget->primary.view, "cursor-changed",
			G_CALLBACK (ug_category_cursor_changed), cwidget);
	g_signal_connect (cwidget->view, "cursor-changed",
			G_CALLBACK (ug_category_cursor_changed), cwidget);
}

void	ug_category_widget_append (UgCategoryWidget* cwidget, UgCategory* category)
{
	UgCategoryGtk*	cgtk;
	GtkTreeIter		iter;

	if (cwidget->primary.category == category)
		return;

	cgtk = category->user.category;
	// append data to GtkTreeStore
	gtk_tree_store_append (cwidget->store, &cgtk->tree_iter, NULL);
	gtk_tree_store_set (cwidget->store, &cgtk->tree_iter,
			UG_CATEGORY_STORE_INSTANCE, category,
			UG_CATEGORY_STORE_DOWNLOAD, &cgtk->all,
			-1);

	gtk_tree_store_append (cwidget->store, &iter, &cgtk->tree_iter);
	gtk_tree_store_set (cwidget->store, &iter,
			UG_CATEGORY_STORE_INSTANCE, category,
			UG_CATEGORY_STORE_DOWNLOAD, &cgtk->active,
			-1);
	gtk_tree_store_append (cwidget->store, &iter, &cgtk->tree_iter);
	gtk_tree_store_set (cwidget->store, &iter,
			UG_CATEGORY_STORE_INSTANCE, category,
			UG_CATEGORY_STORE_DOWNLOAD, &cgtk->queuing,
			-1);
	gtk_tree_store_append (cwidget->store, &iter, &cgtk->tree_iter);
	gtk_tree_store_set (cwidget->store, &iter,
			UG_CATEGORY_STORE_INSTANCE, category,
			UG_CATEGORY_STORE_DOWNLOAD, &cgtk->finished,
			-1);
	gtk_tree_store_append (cwidget->store, &iter, &cgtk->tree_iter);
	gtk_tree_store_set (cwidget->store, &iter,
			UG_CATEGORY_STORE_INSTANCE, category,
			UG_CATEGORY_STORE_DOWNLOAD, &cgtk->recycled,
			-1);
}

void	ug_category_widget_remove (UgCategoryWidget* cwidget, UgCategory* category)
{
	GtkTreeSelection*	selection;
	GtkTreeModel*		model;
	GtkTreePath*		path;
	gint				n_rows;

	if (cwidget->primary.category == category)
		return;

	model  = gtk_tree_view_get_model (cwidget->view);
	n_rows = gtk_tree_model_iter_n_children (model, NULL);
	if (n_rows == 1) {
		selection = gtk_tree_view_get_selection (cwidget->view);
		gtk_tree_selection_unselect_all (selection);
		// switch to primary_view
		gtk_tree_view_get_cursor (cwidget->primary.view, &path, NULL);
		gtk_tree_view_set_cursor (cwidget->primary.view, path,  NULL, FALSE);
		gtk_tree_path_free (path);
	}
	else if (cwidget->current.category == category) {
		gtk_tree_view_get_cursor (cwidget->view, &path, NULL);
		if (gtk_tree_path_get_depth (path) == 2)
			gtk_tree_path_up (path);
		if (*gtk_tree_path_get_indices (path) == n_rows-1)
			gtk_tree_path_prev (path);
		else
			gtk_tree_path_next (path);
		gtk_tree_view_set_cursor (cwidget->view, path, NULL, FALSE);
		gtk_tree_path_free (path);
	}

	gtk_tree_store_remove (cwidget->store,
			&((UgCategoryGtk*)category->user.category)->tree_iter);
	ug_category_free (category);
}

void	ug_category_widget_add_list (UgCategoryWidget* cwidget, GList*	list)
{
	for (;  list;  list = list->next)
		ug_category_widget_append (cwidget, list->data);
}

GList*	ug_category_widget_get_list (UgCategoryWidget* cwidget)
{
	GtkTreeIter		iter;
	GtkTreeModel*	model;
	UgCategoryGtk*	category;
	gboolean		valid;
	GList*			list;

	list = NULL;
	model = GTK_TREE_MODEL (cwidget->store);
	valid = gtk_tree_model_get_iter_first (model, &iter);
	while (valid) {
		gtk_tree_model_get (model, &iter,
				UG_CATEGORY_STORE_INSTANCE, &category,
				-1);
		list = g_list_prepend (list, category);
		valid = gtk_tree_model_iter_next (model, &iter);
	}
	return g_list_reverse (list);
}

guint	ug_category_widget_n_category (UgCategoryWidget* cwidget)
{
	return gtk_tree_model_iter_n_children (GTK_TREE_MODEL (cwidget->store), NULL);
}

void	ug_category_widget_set_cursor (UgCategoryWidget* cwidget, gint nth_category, gint nth_dwidget)
{
	GtkTreePath*		path;

	if (nth_category == -1) {
		path = gtk_tree_path_new_from_indices (nth_dwidget, -1);
		gtk_tree_view_set_cursor (cwidget->primary.view, path, NULL, FALSE);
	}
	else {
		path = gtk_tree_path_new_from_indices (nth_category, nth_dwidget-1 , -1);
		gtk_tree_view_set_cursor (cwidget->view, path, NULL, FALSE);
	}
	gtk_tree_path_free (path);
}

// signal handler
static void	ug_category_cursor_changed (GtkTreeView* view, UgCategoryWidget* cwidget)
{
	GtkTreeSelection*	selection;
	GtkTreeModel*		model;
	GtkTreePath*		path;
	GtkTreeIter			iter;

	// get setting of download column
	if (cwidget->current.widget == &cwidget->current.cgtk->all) {
		cwidget->sort.nth   = cwidget->current.widget->sort.nth;
		cwidget->sort.order = cwidget->current.widget->sort.order;
	}

	if (cwidget->current.view != view) {
		cwidget->current.view = view;
		if (cwidget->primary.view == view)
			selection = gtk_tree_view_get_selection (cwidget->view);
		else
			selection = gtk_tree_view_get_selection (cwidget->primary.view);
		gtk_tree_selection_unselect_all (selection);
	}

	gtk_tree_view_get_cursor (view, &path, NULL);
	if (path) {
		model = gtk_tree_view_get_model (view);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_tree_path_free (path);
		gtk_tree_model_get (model, &iter,
				UG_CATEGORY_STORE_INSTANCE, &cwidget->current.category,
				UG_CATEGORY_STORE_DOWNLOAD, &cwidget->current.widget,
				-1);
	}
	else {
		cwidget->current.category = cwidget->primary.category;
		cwidget->current.widget = &cwidget->primary.cgtk->all;
	}

	// set UgCategoryGtk from UgCategory
	cwidget->current.cgtk = cwidget->current.category->user.category;

	// set setting of download column
	if (cwidget->current.widget == &cwidget->current.cgtk->all) {
		ug_download_view_set_sort_order (cwidget->current.widget->view,
				cwidget->sort.nth, cwidget->sort.order);
		cwidget->current.widget->sort.nth   = cwidget->sort.nth;
		cwidget->current.widget->sort.order = cwidget->sort.order;
	}
}

// Function used by GtkTreeModelFilter.
static gboolean	ug_category_model_filter (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	UgCategory*			category;
	UgDownloadWidget*	dwidget;

	gtk_tree_model_get (model, iter,
			UG_CATEGORY_STORE_INSTANCE, &category,
			UG_CATEGORY_STORE_DOWNLOAD, &dwidget,
			-1);

	if (category && &((UgCategoryGtk*)category->user.category)->all == dwidget)
		return TRUE;
	return FALSE;
}


// ----------------------------------------------------------------------------
// GtkTreeView for UgCategoryGtk
//
#define UG_CATEGORY_ICON_ALL		GTK_STOCK_DND_MULTIPLE
#define UG_CATEGORY_ICON_PAUSED		GTK_STOCK_MEDIA_PAUSE
#define UG_CATEGORY_ICON_EXECUTE	GTK_STOCK_EXECUTE
#define UG_CATEGORY_ICON_FINISHED	GTK_STOCK_GOTO_LAST
#define UG_CATEGORY_ICON_RECYCLED	GTK_STOCK_DELETE

static void category_col_set_icon (GtkTreeViewColumn *tree_column,
                                   GtkCellRenderer   *cell,
                                   GtkTreeModel      *model,
                                   GtkTreeIter       *iter,
                                   gpointer           data)
{
	UgCategory*			category;
	UgCategoryGtk*		cgtk;
	UgDownloadWidget*	dwidget;
	const gchar*		stock_id;

	gtk_tree_model_get (model, iter,
			UG_CATEGORY_STORE_INSTANCE, &category,
			UG_CATEGORY_STORE_DOWNLOAD, &dwidget,
			-1);
	cgtk = category->user.category;

	if (dwidget == &cgtk->all)
		stock_id = UG_CATEGORY_ICON_ALL;
	else if (dwidget == &cgtk->finished)
		stock_id = UG_CATEGORY_ICON_FINISHED;
	else if (dwidget == &cgtk->recycled)
		stock_id = UG_CATEGORY_ICON_RECYCLED;
	else if (dwidget == &cgtk->queuing)
		stock_id = UG_CATEGORY_ICON_PAUSED;
	else
		stock_id = UG_CATEGORY_ICON_EXECUTE;

	g_object_set (cell, "stock-id", stock_id, NULL);
}

static void category_col_set_name (GtkTreeViewColumn *tree_column,
                                   GtkCellRenderer   *cell,
                                   GtkTreeModel      *model,
                                   GtkTreeIter       *iter,
                                   gpointer           data)
{
	UgCategory*			category;
	UgCategoryGtk*		cgtk;
	UgDownloadWidget*	dwidget;
	gchar*				string;

	gtk_tree_model_get (model, iter,
			UG_CATEGORY_STORE_INSTANCE, &category,
			UG_CATEGORY_STORE_DOWNLOAD, &dwidget,
			-1);
	cgtk = category->user.category;

	if (dwidget == &cgtk->active)
		string = UG_CATEGORY_GTK_ACTIVE_NAME;
	else if (dwidget == &cgtk->queuing)
		string = UG_CATEGORY_GTK_QUEUING_NAME;
	else if (dwidget == &cgtk->finished)
		string = UG_CATEGORY_GTK_FINISHED_NAME;
	else if (dwidget == &cgtk->recycled)
		string = UG_CATEGORY_GTK_RECYCLED_NAME;
	else
		string = category->name;

	g_object_set (cell, "text", string, NULL);
}

static void category_col_set_quantity (GtkTreeViewColumn *tree_column,
                                       GtkCellRenderer   *cell,
                                       GtkTreeModel      *model,
                                       GtkTreeIter       *iter,
                                       gpointer           data)
{
	UgDownloadWidget*	dwidget;
	gchar*				string;
	gint				quantity;

	gtk_tree_model_get (model, iter,
			UG_CATEGORY_STORE_DOWNLOAD, &dwidget,
			-1);
	quantity = gtk_tree_model_iter_n_children (dwidget->model, NULL);
	string = g_strdup_printf ("%u", quantity);
	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

GtkTreeView* ug_category_view_new (void)
{
	GtkTreeView*       view;
	GtkCellRenderer*   renderer;
	GtkTreeViewColumn* column;

	view = (GtkTreeView*) gtk_tree_view_new ();
	gtk_tree_view_set_headers_visible (view, FALSE);

	// column name
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Category"));
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         category_col_set_icon,
	                                         NULL, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         category_col_set_name,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
//	gtk_tree_view_column_set_min_width (column, 120);
	gtk_tree_view_column_set_expand (column, TRUE);
//	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (view, column);

	// column Quantity = number of tasks
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Quantity"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         category_col_set_quantity,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_alignment (column, 1.0);
//	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (view, column);

	gtk_widget_show (GTK_WIDGET (view));
	return view;
}

UgCategory*	ug_category_view_get_nth (GtkTreeView* view, gint n)
{
	GtkTreeModel*	model;
	GtkTreeIter		iter;
	UgCategory*		category;

	model = gtk_tree_view_get_model (view);
	if (gtk_tree_model_iter_nth_child (model, &iter, NULL, n) == FALSE)
		return NULL;

	gtk_tree_model_get (model, &iter,
			UG_CATEGORY_STORE_INSTANCE, &category,
			-1);
	return category;
}

UgCategory*	ug_category_view_get_cursor (GtkTreeView* view)
{
	GtkTreeModel*	model;
	GtkTreePath*	path;
	GtkTreeIter		iter;
	UgCategory*		category;

	model = gtk_tree_view_get_model (view);
	gtk_tree_view_get_cursor (view, &path, NULL);
	if (path) {
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_tree_path_free (path);
	}
	else if (gtk_tree_model_get_iter_first (model, &iter) == FALSE)
		return NULL;

	gtk_tree_model_get (model, &iter,
			UG_CATEGORY_STORE_INSTANCE, &category,
			-1);
	return category;
}

void	ug_category_view_set_cursor (GtkTreeView* view, gint nth, gint nth_child)
{
	GtkTreePath*		path;

	if (nth != -1) {
		path = gtk_tree_path_new_from_indices (nth, nth_child , -1);
		gtk_tree_view_set_cursor (view, path, NULL, FALSE);
		gtk_tree_path_free (path);
	}
}

void	ug_category_view_set_icon_visible (GtkTreeView* view, gboolean visible)
{
	GtkCellRenderer*   renderer;
	GtkTreeViewColumn* column;

	// clear
	column = gtk_tree_view_get_column (view, 0);
	gtk_tree_view_column_clear (column);
	// pack icon renderer
	if (visible) {
		renderer = gtk_cell_renderer_pixbuf_new ();
		gtk_tree_view_column_pack_start (column, renderer, FALSE);
		gtk_tree_view_column_set_cell_data_func (column,
		                                         renderer,
		                                         category_col_set_icon,
		                                         NULL, NULL);
	}
	// pack text renderer
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         category_col_set_name,
	                                         NULL, NULL);
}

