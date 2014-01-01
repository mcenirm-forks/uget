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

#include <UgUri.h>
#include <UgUtils.h>
#include <UgString.h>
#include <UgetData.h>
#include <UgCategory-gtk.h>
#include <UgDownloadWidget.h>

#include <glib/gi18n.h>

// Function used by GtkTreeModelSort.
static gint	ug_download_model_cmp_name (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_complete (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_size (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_percent (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_elapsed (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_left (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_speed (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_upload_speed (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_uploaded (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_ratio (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_retry (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_category (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_url (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_added_on (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
static gint	ug_download_model_cmp_completed_on (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data);
// signal handler
static void	on_name_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_complete_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_size_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_percent_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_elapsed_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_left_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_speed_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_upload_speed_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_uploaded_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_ratio_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_retry_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_category_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_url_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_added_on_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);
static void	on_completed_on_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget);


void	ug_download_widget_init (UgDownloadWidget* dwidget, GtkTreeModel* model)
{
	GtkTreeSelection*	selection;
	GtkScrolledWindow*	scroll;

	dwidget->self  = gtk_scrolled_window_new (NULL, NULL);
	dwidget->view  = ug_download_view_new ();
	dwidget->model = model;
	gtk_tree_view_set_model (dwidget->view, model);
	selection = gtk_tree_view_get_selection (dwidget->view);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

	scroll = GTK_SCROLLED_WINDOW (dwidget->self);
	gtk_scrolled_window_set_shadow_type (scroll, GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (scroll, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scroll), GTK_WIDGET (dwidget->view));
	gtk_widget_show_all (dwidget->self);
}

void	ug_download_widget_finalize (UgDownloadWidget* dwidget)
{
	gtk_widget_destroy (dwidget->self);
	g_object_unref (dwidget->model);
}

void	ug_download_widget_set_filter (UgDownloadWidget* dwidget, GtkTreeModel* model, GtkTreeModelFilterVisibleFunc func, gpointer data)
{
	GtkTreeModel*		filter;

	filter = gtk_tree_model_filter_new (model, NULL);
	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter), func, data, NULL);

	gtk_tree_view_set_model (dwidget->view, filter);
	if (dwidget->model)
		g_object_unref (dwidget->model);
	dwidget->model = filter;
}

void	ug_download_widget_use_sortable (UgDownloadWidget* dwidget, GtkTreeModel* model)
{
	GtkTreeModel*		sort;
	GtkTreeSortable*	sortable;
	GtkTreeViewColumn*	column;

	sort = gtk_tree_model_sort_new_with_model (model);
	sortable = GTK_TREE_SORTABLE (sort);
	gtk_tree_sortable_set_default_sort_func (sortable,
			ug_download_model_cmp_name, NULL, NULL);

	gtk_tree_view_set_model (dwidget->view, sort);
	if (dwidget->model)
		g_object_unref (dwidget->model);
	dwidget->model = sort;

	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_NAME
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_NAME);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_sort_indicator (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_name_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_COMPLETE
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_COMPLETE);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_complete_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_SIZE
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_SIZE);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_size_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_PERCENT
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_PERCENT);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_percent_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_ELAPSED
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_ELAPSED);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_elapsed_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_LEFT
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_LEFT);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_left_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_SPEED
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_SPEED);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_speed_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_UPLOAD_SPEED
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_UPLOAD_SPEED);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_upload_speed_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_UPLOADED
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_UPLOADED);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_uploaded_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_RATIO
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_RATIO);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_ratio_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_RETRY
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_RETRY);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_retry_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_CATEGORY
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_CATEGORY);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_category_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_URL
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_URL);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_url_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_ADDED_ON
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_ADDED_ON);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_added_on_column_clicked), dwidget);
	// GtkTreeViewColumn - UG_DOWNLOAD_COLUMN_COMPLETED_ON
	column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_COMPLETED_ON);
	gtk_tree_view_column_set_clickable (column, TRUE);
	g_signal_connect (column, "clicked",
			G_CALLBACK (on_completed_on_column_clicked), dwidget);
}

GList*	ug_download_widget_get_selected (UgDownloadWidget* dwidget)
{
	UgDataset*			dataset;
	GtkTreeSelection*	selection;
	GtkTreeModel*		model;
	GtkTreeIter			iter;
	GList*				list;
	GList*				link;

	selection = gtk_tree_view_get_selection (dwidget->view);
	list = gtk_tree_selection_get_selected_rows (selection, &model);
	for (link = list;  link;  link = link->next) {
		gtk_tree_model_get_iter (model, &iter, link->data);
		gtk_tree_model_get (model, &iter, 0, &dataset, -1);
		gtk_tree_path_free (link->data);
		link->data = dataset;
	}
	return list;
}

GList*	ug_download_widget_get_selected_indices (UgDownloadWidget* dwidget)
{
	GtkTreeSelection*	selection;
	GList*				list;
	GList*				link;
	gint				index;

	selection = gtk_tree_view_get_selection (dwidget->view);
	list = gtk_tree_selection_get_selected_rows (selection, NULL);
	for (link = list;  link;  link = link->next) {
		index = *gtk_tree_path_get_indices (link->data);
		gtk_tree_path_free (link->data);
		link->data = GINT_TO_POINTER (index);
	}
	return list;
}

gint	ug_download_widget_count_selected (UgDownloadWidget* dwidget)
{
	GtkTreeSelection*	selection;

	selection = gtk_tree_view_get_selection (dwidget->view);
	return gtk_tree_selection_count_selected_rows (selection);
}

UgDataset*	ug_download_widget_get_cursor (UgDownloadWidget* dwidget)
{
	UgDataset*		dataset;
	GtkTreePath*	path;
	GtkTreeIter		iter;
	gboolean		valid;

	gtk_tree_view_get_cursor (dwidget->view, &path, NULL);
	if (path) {
		valid = gtk_tree_model_get_iter (dwidget->model, &iter, path);
		gtk_tree_path_free (path);
		if (valid) {
			gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
			return dataset;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// GtkTreeView for UgDataset
//
#define UG_DOWNLOAD_ICON_NORMAL			GTK_STOCK_FILE
#define UG_DOWNLOAD_ICON_PAUSED			GTK_STOCK_MEDIA_PAUSE
#define UG_DOWNLOAD_ICON_ACTIVE			GTK_STOCK_MEDIA_PLAY
#define UG_DOWNLOAD_ICON_COMPLETED		GTK_STOCK_YES
#define UG_DOWNLOAD_ICON_ERROR			GTK_STOCK_DIALOG_ERROR
#define UG_DOWNLOAD_ICON_RECYCLED		GTK_STOCK_DELETE
#define UG_DOWNLOAD_ICON_FINISHED		GTK_STOCK_GOTO_LAST

static void col_set_icon (GtkTreeViewColumn *tree_column,
                          GtkCellRenderer   *cell,
                          GtkTreeModel      *model,
                          GtkTreeIter       *iter,
                          gpointer           data)
{
	UgDataset*		dataset;
	UgetRelation*		relation;
	const gchar*	stock_id;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	relation = UG_DATASET_RELATION (dataset);
	// set stock_id
	if (relation->hints & UG_HINT_DOWNLOADING)
		stock_id = UG_DOWNLOAD_ICON_ACTIVE;
	else if (relation->hints & UG_HINT_PAUSED)
		stock_id = UG_DOWNLOAD_ICON_PAUSED;
	else if (relation->hints & UG_HINT_ERROR)
		stock_id = UG_DOWNLOAD_ICON_ERROR;
	else if (relation->hints & UG_HINT_COMPLETED)
		stock_id = UG_DOWNLOAD_ICON_COMPLETED;
	else
		stock_id = UG_DOWNLOAD_ICON_NORMAL;

	g_object_set (cell, "stock-id", stock_id, NULL);
}

static void col_set_icon_all (GtkTreeViewColumn *tree_column,
                              GtkCellRenderer   *cell,
                              GtkTreeModel      *model,
                              GtkTreeIter       *iter,
                              gpointer           data)
{
	UgDataset*		dataset;
	UgetRelation*		relation;
	const gchar*	stock_id;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	relation = UG_DATASET_RELATION (dataset);
	// set stock_id
	if (relation->hints & UG_HINT_RECYCLED)
		stock_id = UG_DOWNLOAD_ICON_RECYCLED;
	else if (relation->hints & UG_HINT_FINISHED)
		stock_id = UG_DOWNLOAD_ICON_FINISHED;
	else if (relation->hints & UG_HINT_DOWNLOADING)
		stock_id = UG_DOWNLOAD_ICON_ACTIVE;
	else if (relation->hints & UG_HINT_PAUSED)
		stock_id = UG_DOWNLOAD_ICON_PAUSED;
	else if (relation->hints & UG_HINT_ERROR)
		stock_id = UG_DOWNLOAD_ICON_ERROR;
	else if (relation->hints & UG_HINT_COMPLETED)
		stock_id = UG_DOWNLOAD_ICON_COMPLETED;
	else
		stock_id = UG_DOWNLOAD_ICON_NORMAL;

	g_object_set (cell, "stock-id", stock_id, NULL);
}

static void col_set_name (GtkTreeViewColumn *tree_column,
                          GtkCellRenderer   *cell,
                          GtkTreeModel      *model,
                          GtkTreeIter       *iter,
                          gpointer           data)
{
	UgDataset*		dataset;
	UgetCommon*	common;
	gchar*			string = NULL;
	gchar*			name = NULL;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	common = UG_DATASET_COMMON (dataset);
	if (common->name)
		string = common->name;
	else if (common->file)
		string = common->file;
	else {
		if (common->url)
			name = ug_uri_get_filename (common->url);
		if (name == NULL)
			name = g_strconcat (_("unnamed"), " ", common->url, NULL);
		string = name;
	}

	g_object_set (cell, "text", string, NULL);
	g_free (name);
}

static void col_set_complete (GtkTreeViewColumn *tree_column,
                              GtkCellRenderer   *cell,
                              GtkTreeModel      *model,
                              GtkTreeIter       *iter,
                              gpointer           data)
{
	UgDataset*		dataset;
	UgetProgress*		progress;
	gchar*			string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	if (progress)
		string = ug_str_dtoa_unit ((gdouble) progress->complete, 1, NULL);
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_total (GtkTreeViewColumn *tree_column,
                           GtkCellRenderer   *cell,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	if (progress)
		string = ug_str_dtoa_unit ((gdouble) progress->total, 1, NULL);
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_percent (GtkTreeViewColumn *tree_column,
                             GtkCellRenderer   *cell,
                             GtkTreeModel      *model,
                             GtkTreeIter       *iter,
                             gpointer           data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	if (progress) {
//		string = g_strdup_printf ("%.*f%c", 1, progress->percent, '%');
		string = g_strdup_printf ("%.1f%c", progress->percent, '%');
		g_object_set (cell, "visible", 1, NULL);
		g_object_set (cell, "value", (gint) progress->percent, NULL);
		g_object_set (cell, "text", string, NULL);
		g_free (string);
	}
	else {
		g_object_set (cell, "visible", 0, NULL);
		g_object_set (cell, "value", 0, NULL);
		g_object_set (cell, "text", "", NULL);
	}
}

static void col_set_consume_time (GtkTreeViewColumn *tree_column,
                                  GtkCellRenderer   *cell,
                                  GtkTreeModel      *model,
                                  GtkTreeIter       *iter,
                                  gpointer           data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	if (progress)
		string = ug_str_from_seconds ((guint) progress->consume_time, TRUE);
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_remain_time (GtkTreeViewColumn *tree_column,
                                 GtkCellRenderer   *cell,
                                 GtkTreeModel      *model,
                                 GtkTreeIter       *iter,
                                 gpointer           data)
{
	UgDataset*	dataset;
	UgetRelation*	relation;
	UgetProgress*	progress;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	relation = UG_DATASET_RELATION (dataset);
	if (progress && relation && relation->plugin)
		string = ug_str_from_seconds ((guint) progress->remain_time, TRUE);
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_speed (GtkTreeViewColumn *tree_column,
                           GtkCellRenderer   *cell,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           data)
{
	UgDataset*	dataset;
	UgetRelation*	relation;
	UgetProgress*	progress;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	relation = UG_DATASET_RELATION (dataset);
	if (progress && relation && relation->plugin)
		string = ug_str_dtoa_unit ((gdouble) progress->download_speed, 1, "/s");
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_upload_speed (GtkTreeViewColumn *tree_column,
                           GtkCellRenderer   *cell,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           data)
{
	UgDataset*	dataset;
	UgetRelation*	relation;
	UgetProgress*	progress;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	relation = UG_DATASET_RELATION (dataset);
	if (progress && relation && relation->plugin && progress->upload_speed)
		string = ug_str_dtoa_unit ((gdouble) progress->upload_speed, 1, "/s");
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_uploaded (GtkTreeViewColumn *tree_column,
                           GtkCellRenderer   *cell,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	if (progress && progress->uploaded)
		string = ug_str_dtoa_unit ((gdouble) progress->uploaded, 1, NULL);
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_ratio (GtkTreeViewColumn *tree_column,
                           GtkCellRenderer   *cell,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	progress = UG_DATASET_PROGRESS (dataset);
	if (progress && progress->ratio)
		string = g_strdup_printf ("%.2f", progress->ratio);
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_retry (GtkTreeViewColumn *tree_column,
                           GtkCellRenderer   *cell,
                           GtkTreeModel      *model,
                           GtkTreeIter       *iter,
                           gpointer           data)
{
	UgDataset*		dataset;
	UgetCommon*	common;
	gchar*			string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	common = UG_DATASET_COMMON (dataset);
	if (common->retry_count != 0)
		string = g_strdup_printf ("%d", common->retry_count);
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
	g_free (string);
}

static void col_set_category (GtkTreeViewColumn *tree_column,
                              GtkCellRenderer   *cell,
                              GtkTreeModel      *model,
                              GtkTreeIter       *iter,
                              gpointer           data)
{
	UgDataset*		dataset;
	UgetRelation*		relation;
	gchar*			string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	relation = UG_DATASET_RELATION (dataset);
	if (relation && relation->category)
		string = relation->category->name;
	else
		string = NULL;

	g_object_set (cell, "text", string, NULL);
}

static void col_set_url (GtkTreeViewColumn *tree_column,
                         GtkCellRenderer   *cell,
                         GtkTreeModel      *model,
                         GtkTreeIter       *iter,
                         gpointer           data)
{
	UgDataset*	dataset;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	g_object_set (cell, "text", UG_DATASET_COMMON (dataset)->url, NULL);
}

static void col_set_added_on (GtkTreeViewColumn *tree_column,
                              GtkCellRenderer   *cell,
                              GtkTreeModel      *model,
                              GtkTreeIter       *iter,
                              gpointer           data)
{
	UgDataset*	dataset;
	UgetLog*	datalog;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	datalog = ug_dataset_get (dataset, UgetLogInfo, 0);
	string = (datalog) ? datalog->added_on : NULL;

	g_object_set (cell, "text", string, NULL);
}

static void col_set_completed_on (GtkTreeViewColumn *tree_column,
                                  GtkCellRenderer   *cell,
                                  GtkTreeModel      *model,
                                  GtkTreeIter       *iter,
                                  gpointer           data)
{
	UgDataset*	dataset;
	UgetLog*	datalog;
	gchar*		string;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	// avoid crash in GTK3
	if (dataset == NULL)
		return;

	datalog = ug_dataset_get (dataset, UgetLogInfo, 0);
	string = (datalog) ? datalog->completed_on : NULL;

	g_object_set (cell, "text", string, NULL);
}

GtkTreeView*	ug_download_view_new (void)
{
	GtkTreeView*       tview;
	GtkTreeSelection*  selection;
	GtkCellRenderer*   renderer;
	GtkCellRenderer*   renderer_progress;
	GtkTreeViewColumn* column;

	tview = (GtkTreeView*)gtk_tree_view_new ();
	gtk_tree_view_set_fixed_height_mode (tview, TRUE);
	selection = gtk_tree_view_get_selection (tview);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

	// column name
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Name"));
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_icon,
	                                         NULL, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_name,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 180);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (tview, column);

	// column completed
	column = gtk_tree_view_column_new ();
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_set_title (column, _("Complete"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_complete,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 80);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// column total
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Size"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_total,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 80);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// column percent
	column = gtk_tree_view_column_new ();
	renderer_progress = gtk_cell_renderer_progress_new ();
	gtk_tree_view_column_set_title (column, _("%"));
	gtk_tree_view_column_pack_start (column, renderer_progress, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer_progress,
	                                         col_set_percent,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 60);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// column "Elapsed" for consuming time
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Elapsed"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_consume_time,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 65);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// column "Left" for remaining time
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Left"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_remain_time,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 65);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// columns speed
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Speed"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_speed,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 90);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// columns upload speed
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Up Speed"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_upload_speed,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 90);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// columns uploaded
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Uploaded"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_uploaded,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 80);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// columns ratio
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Ratio"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_ratio,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 45);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// column retries
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Retry"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_retry,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 45);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_alignment (column, 1.0);
	gtk_tree_view_append_column (tview, column);

	// column category
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Category"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_category,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 100);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (tview, column);

	// column url
//	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("URL"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_url,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 300);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (tview, column);

	// column addon_on
//	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Added On"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_added_on,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 140);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (tview, column);

	// column completed_on
//	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Completed On"));
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
	                                         renderer,
	                                         col_set_completed_on,
	                                         NULL, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 140);
	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (tview, column);

	gtk_widget_show (GTK_WIDGET (tview));
	return tview;
}

void	ug_download_view_use_all_icon (GtkTreeView* view, gboolean visible_all)
{
	GtkCellRenderer*   renderer;
	GtkTreeViewColumn* column;

	// clear
	column = gtk_tree_view_get_column (view, 0);
	gtk_tree_view_column_clear (column);
	// pack icon renderer
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	if (visible_all) {
		gtk_tree_view_column_set_cell_data_func (column, renderer,
				col_set_icon_all, NULL, NULL);
	}
	else {
		gtk_tree_view_column_set_cell_data_func (column, renderer,
				col_set_icon, NULL, NULL);
	}
	// pack text renderer
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column, renderer,
			col_set_name, NULL, NULL);
}

static void	ug_download_view_clear_sort_status (GtkTreeView* view)
{
	GtkTreeViewColumn*	column;
	gint	index;

	for (index = 0;  index < UG_DOWNLOAD_N_COLUMN;  index++) {
		column = gtk_tree_view_get_column (view, index);
		gtk_tree_view_column_set_sort_order (column, GTK_SORT_ASCENDING);
		gtk_tree_view_column_set_sort_indicator (column, FALSE);
	}
}

void	ug_download_view_set_sort_order (GtkTreeView* view, guint nth_column, GtkSortType sorttype)
{
	GtkTreeViewColumn*	column;
	GtkTreeSortable*	sortable;
	GtkTreeIterCompareFunc	func;

	sortable = GTK_TREE_SORTABLE (gtk_tree_view_get_model (view));
	column   = gtk_tree_view_get_column (view, nth_column);
	ug_download_view_clear_sort_status (view);
	gtk_tree_view_column_set_sort_order (column, sorttype);
	gtk_tree_view_column_set_sort_indicator (column, TRUE);

	switch (nth_column) {
	case UG_DOWNLOAD_COLUMN_NAME:
		func = ug_download_model_cmp_name;
		break;

	case UG_DOWNLOAD_COLUMN_COMPLETE:
		func = ug_download_model_cmp_complete;
		break;

	case UG_DOWNLOAD_COLUMN_SIZE:
		func = ug_download_model_cmp_size;
		break;

	case UG_DOWNLOAD_COLUMN_PERCENT:
		func = ug_download_model_cmp_percent;
		break;

	case UG_DOWNLOAD_COLUMN_ELAPSED:
		func = ug_download_model_cmp_elapsed;
		break;

	case UG_DOWNLOAD_COLUMN_LEFT:
		func = ug_download_model_cmp_left;
		break;

	case UG_DOWNLOAD_COLUMN_SPEED:
		func = ug_download_model_cmp_speed;
		break;

	case UG_DOWNLOAD_COLUMN_UPLOAD_SPEED:
		func = ug_download_model_cmp_upload_speed;
		break;

	case UG_DOWNLOAD_COLUMN_UPLOADED:
		func = ug_download_model_cmp_uploaded;
		break;

	case UG_DOWNLOAD_COLUMN_RATIO:
		func = ug_download_model_cmp_ratio;
		break;

	case UG_DOWNLOAD_COLUMN_RETRY:
		func = ug_download_model_cmp_retry;
		break;

	case UG_DOWNLOAD_COLUMN_CATEGORY:
		func = ug_download_model_cmp_category;
		break;

	case UG_DOWNLOAD_COLUMN_URL:
		func = ug_download_model_cmp_url;
		break;

	case UG_DOWNLOAD_COLUMN_ADDED_ON:
		func = ug_download_model_cmp_added_on;
		break;

	case UG_DOWNLOAD_COLUMN_COMPLETED_ON:
		func = ug_download_model_cmp_completed_on;
		break;

	default:
		func = NULL;
		break;
	}

	gtk_tree_sortable_set_default_sort_func (sortable,
			func, GINT_TO_POINTER (sorttype), NULL);
}


// ----------------------------------------------------------------------------
// Function used by GtkTreeModelSort.
//
static gint	ug_download_model_cmp_name (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*		dataset;
	UgetCommon*	common;
	gchar*			name1;
	gchar*			name2;

	name1 = NULL;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		common = UG_DATASET_COMMON (dataset);
		if (common) {
			if (common->name)
				name1 = common->name;
			else if (common->file)
				name1 = common->file;
		}
	}

	name2 = NULL;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		common = UG_DATASET_COMMON (dataset);
		if (common) {
			if (common->name)
				name2 = common->name;
			else if (common->file)
				name2 = common->file;
		}
	}

	// g_strcmp0 can handle NULL gracefully by sorting it before non-NULL strings.
	if (user_data == NULL)
		return g_strcmp0 (name1, name2);
	else
		return g_strcmp0 (name2, name1);
}

static gint	ug_download_model_cmp_complete (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gint64		completed1;
	gint64		completed2;

	completed1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			completed1 = progress->complete;
	}

	completed2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			completed2 = progress->complete;
	}

	if (user_data == NULL)
		return (gint) (completed1 - completed2);
	else
		return (gint) (completed2 - completed1);
}

static gint	ug_download_model_cmp_size (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gint64		size1;
	gint64		size2;

	size1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			size1 = progress->total;
	}

	size2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			size2 = progress->total;
	}

	if (user_data == NULL)
		return (gint) (size1 - size2);
	else
		return (gint) (size2 - size1);
}

static gint	ug_download_model_cmp_percent (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gdouble		percent1;
	gdouble		percent2;

	percent1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			percent1 = progress->percent;
	}

	percent2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			percent2 = progress->percent;
	}

	if (user_data == NULL)
		return (gint) (percent1 - percent2);
	else
		return (gint) (percent2 - percent1);
}

static gint	ug_download_model_cmp_elapsed (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gdouble		elapsed1;
	gdouble		elapsed2;

	elapsed1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			elapsed1 = progress->consume_time;
	}

	elapsed2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			elapsed2 = progress->consume_time;
	}

	if (user_data == NULL)
		return (gint) (elapsed1 - elapsed2);
	else
		return (gint) (elapsed2 - elapsed1);
}

static gint	ug_download_model_cmp_left (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gdouble		left1;
	gdouble		left2;

	left1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			left1 = progress->remain_time;
	}

	left2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			left2 = progress->remain_time;
	}

	if (user_data == NULL)
		return (gint) (left1 - left2);
	else
		return (gint) (left2 - left1);
}

static gint	ug_download_model_cmp_speed (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gdouble		speed1;
	gdouble		speed2;

	speed1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			speed1 = progress->download_speed;
	}

	speed2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			speed2 = progress->download_speed;
	}

	if (user_data == NULL)
		return (gint) (speed1 - speed2);
	else
		return (gint) (speed2 - speed1);
}

static gint	ug_download_model_cmp_upload_speed (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gdouble		speed1;
	gdouble		speed2;

	speed1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			speed1 = progress->upload_speed;
	}

	speed2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			speed2 = progress->upload_speed;
	}

	if (user_data == NULL)
		return (gint) (speed1 - speed2);
	else
		return (gint) (speed2 - speed1);
}

static gint	ug_download_model_cmp_uploaded (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gint64		uploaded1;
	gint64		uploaded2;

	uploaded1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			uploaded1 = progress->uploaded;
	}

	uploaded2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			uploaded2 = progress->uploaded;
	}

	if (user_data == NULL)
		return (gint) (uploaded1 - uploaded2);
	else
		return (gint) (uploaded2 - uploaded1);
}

static gint	ug_download_model_cmp_ratio (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetProgress*	progress;
	gdouble		ratio1;
	gdouble		ratio2;

	ratio1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			ratio1 = progress->ratio;
	}

	ratio2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		progress = UG_DATASET_PROGRESS (dataset);
		if (progress)
			ratio2 = progress->ratio;
	}

	if (user_data == NULL)
		return (gint) (ratio1 - ratio2);
	else
		return (gint) (ratio2 - ratio1);
}

static gint	ug_download_model_cmp_retry (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*		dataset;
	UgetCommon*	common;
	gint			retry1;
	gint			retry2;

	retry1 = 0;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		common = UG_DATASET_COMMON (dataset);
		if (common)
			retry1 = common->retry_count;
	}

	retry2 = 0;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		common = UG_DATASET_COMMON (dataset);
		if (common)
			retry2 = common->retry_count;
	}

	if (user_data == NULL)
		return retry1 - retry2;
	else
		return retry2 - retry1;
}

static gint	ug_download_model_cmp_category (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgCategory*	category;
	gchar*		name1;
	gchar*		name2;

	name1 = NULL;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		category = UG_DATASET_RELATION (dataset)->category;
		if (category)
			name1 = category->name;
	}

	name2 = NULL;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		category = UG_DATASET_RELATION (dataset)->category;
		if (category)
			name2 = category->name;
	}

	// g_strcmp0 can handle NULL gracefully by sorting it before non-NULL strings.
	if (user_data == NULL)
		return g_strcmp0 (name1, name2);
	else
		return g_strcmp0 (name2, name1);
}

static gint	ug_download_model_cmp_url (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*		dataset;
	UgetCommon*	common;
	gchar*			url1;
	gchar*			url2;

	url1 = NULL;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		common = UG_DATASET_COMMON (dataset);
		if (common)
			url1 = common->url;
	}

	url2 = NULL;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		common = UG_DATASET_COMMON (dataset);
		if (common)
			url2 = common->url;
	}

	// g_strcmp0 can handle NULL gracefully by sorting it before non-NULL strings.
	if (user_data == NULL)
		return g_strcmp0 (url1, url2);
	else
		return g_strcmp0 (url2, url1);
}

static gint	ug_download_model_cmp_added_on (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetLog*	datalog;
	gchar*		log1;
	gchar*		log2;

	log1 = NULL;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		datalog = ug_dataset_get (dataset, UgetLogInfo, 0);
		if (datalog)
			log1 = datalog->added_on;
	}

	log2 = NULL;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		datalog = ug_dataset_get (dataset, UgetLogInfo, 0);
		if (datalog)
			log2 = datalog->added_on;
	}

	// g_strcmp0 can handle NULL gracefully by sorting it before non-NULL strings.
	if (user_data == NULL)
		return g_strcmp0 (log1, log2);
	else
		return g_strcmp0 (log2, log1);
}

static gint	ug_download_model_cmp_completed_on (GtkTreeModel* model, GtkTreeIter* a, GtkTreeIter* b, gpointer user_data)
{
	UgDataset*	dataset;
	UgetLog*	datalog;
	gchar*		log1;
	gchar*		log2;

	log1 = NULL;
	gtk_tree_model_get (model, a, 0, &dataset, -1);
	if (dataset) {
		datalog = ug_dataset_get (dataset, UgetLogInfo, 0);
		if (datalog)
			log1 = datalog->completed_on;
	}

	log2 = NULL;
	gtk_tree_model_get (model, b, 0, &dataset, -1);
	if (dataset) {
		datalog = ug_dataset_get (dataset, UgetLogInfo, 0);
		if (datalog)
			log2 = datalog->completed_on;
	}

	// g_strcmp0 can handle NULL gracefully by sorting it before non-NULL strings.
	if (user_data == NULL)
		return g_strcmp0 (log1, log2);
	else
		return g_strcmp0 (log2, log1);
}

// ----------------------------------------------------------------------------
// signal handler
//
static void ug_tree_view_column_clicked (GtkTreeViewColumn *treecolumn, GtkTreeView* view, GtkTreeIterCompareFunc func)
{
	GtkSortType			sorttype;
	GtkTreeSortable*	sortable;

	sortable = GTK_TREE_SORTABLE (gtk_tree_view_get_model (view));
	sorttype = gtk_tree_view_column_get_sort_order (treecolumn);
	if (gtk_tree_view_column_get_sort_indicator (treecolumn)) {
		if (sorttype == GTK_SORT_ASCENDING)
			sorttype  = GTK_SORT_DESCENDING;
		else
			sorttype  = GTK_SORT_ASCENDING;
	}
	ug_download_view_clear_sort_status (view);
	gtk_tree_view_column_set_sort_order (treecolumn, sorttype);
	gtk_tree_view_column_set_sort_indicator (treecolumn, TRUE);
	gtk_tree_sortable_set_default_sort_func (sortable,
			func, GINT_TO_POINTER (sorttype), NULL);
}

static void	on_name_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_name);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_NAME;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_complete_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_complete);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_COMPLETE;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_size_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_size);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_SIZE;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_percent_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_percent);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_PERCENT;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_elapsed_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_elapsed);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_ELAPSED;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_left_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_left);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_LEFT;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_speed_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_speed);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_SPEED;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_upload_speed_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_upload_speed);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_UPLOAD_SPEED;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_uploaded_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_uploaded);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_UPLOADED;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_ratio_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_ratio);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_RATIO;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_retry_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_retry);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_RETRY;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_category_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_category);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_CATEGORY;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_url_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_url);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_URL;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_added_on_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_added_on);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_ADDED_ON;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

static void	on_completed_on_column_clicked (GtkTreeViewColumn *treecolumn, UgDownloadWidget* dwidget)
{
	ug_tree_view_column_clicked (treecolumn, dwidget->view, ug_download_model_cmp_completed_on);
	dwidget->sort.nth   = UG_DOWNLOAD_COLUMN_COMPLETED_ON;
	dwidget->sort.order = gtk_tree_view_column_get_sort_order (treecolumn);
}

