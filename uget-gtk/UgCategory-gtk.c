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

#include <stdlib.h>
#include <string.h>
// uglib
#include <UgString.h>
#include <UgData-download.h>
#include <UgCategory-gtk.h>

#include <glib/gi18n.h>

// UgCategory::user.category used for getting UgCategoruGtk
// UgRelation::user.category used for getting UgCategoruGtk
// UgRelation::user.storage  used for getting GtkListStore in primary UgCategoruGtk
// UgRelation::user.position used for getting GtkTreeIter from GtkListStore in primary UgCategoruGtk
// UgRelation::user.data     used for getting UgDownloadWidget

// static functions
static UgCategoryGtk*	ug_category_gtk_new  (void);
static void				ug_category_gtk_free (UgCategoryGtk* cgtk);

// free GtkTreeIter in UgRelation::position
static void		ug_slice_free_tree_iter (GtkTreeIter* iter);
// Function used by GtkTreeModelFilter.
static gboolean	ug_download_model_filter_category (GtkTreeModel *model, GtkTreeIter *iter, UgCategory* category);
static gboolean	ug_download_model_filter_active   (GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
static gboolean	ug_download_model_filter_queuing  (GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
static gboolean	ug_download_model_filter_finished (GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
static gboolean	ug_download_model_filter_recycled (GtkTreeModel *model, GtkTreeIter *iter, gpointer data);

const UgCategoryFuncs	cgtk_funcs =
{
	ug_category_gtk_add,
	ug_category_gtk_get_all,
	ug_category_gtk_get_tasks,
	ug_category_gtk_changed,
};

static UgCategoryGtk*	ug_category_gtk_new  (void)
{
	UgCategoryGtk*		cgtk;
	GtkTreeView*		view;
	GtkTreeViewColumn*	column;

	cgtk = g_malloc0 (sizeof (UgCategoryGtk));
	// initialize UgDownloadWidget
	ug_download_widget_init (&cgtk->all,      NULL);
	ug_download_widget_init (&cgtk->active,   NULL);
	ug_download_widget_init (&cgtk->queuing,  NULL);
	ug_download_widget_init (&cgtk->finished, NULL);
	ug_download_widget_init (&cgtk->recycled, NULL);

	// All
	view = cgtk->all.view;
	ug_download_view_use_all_icon (view, TRUE);
	// Finished column
	view = cgtk->finished.view;
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_COMPLETE);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_PERCENT);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_LEFT);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_SPEED);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_UPLOAD_SPEED);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_UPLOADED);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_RATIO);
	gtk_tree_view_column_set_visible (column, FALSE);
	// Recycled column
	view = cgtk->recycled.view;
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_ELAPSED);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_LEFT);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_SPEED);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_UPLOAD_SPEED);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_UPLOADED);
	gtk_tree_view_column_set_visible (column, FALSE);
	column = gtk_tree_view_get_column (view, UG_DOWNLOAD_COLUMN_RATIO);
	gtk_tree_view_column_set_visible (column, FALSE);

	return cgtk;
}

static void	ug_category_gtk_free (UgCategoryGtk* cgtk)
{
	UgDataset*		dataset;
	UgRelation*		relation;
	GtkTreeModel*	model;
	GtkTreeIter		iter;

	// free UgDownloadWidget
	ug_download_widget_finalize (&cgtk->all);
	ug_download_widget_finalize (&cgtk->active);
	ug_download_widget_finalize (&cgtk->queuing);
	ug_download_widget_finalize (&cgtk->finished);
	ug_download_widget_finalize (&cgtk->recycled);
	// free all tasks
	model = cgtk->filter;
	while (gtk_tree_model_get_iter_first (model, &iter)) {
		gtk_tree_model_get (model, &iter, 0, &dataset, -1);
		// ug_category_gtk_remove (cgtk, dataset);
		relation = UG_DATASET_RELATION (dataset);
		gtk_list_store_remove (
				relation->user.storage,
				relation->user.position);
		relation->user.storage = NULL;
		relation->category = NULL;
		// delete data and files
		ug_download_delete_temp (dataset);
		ug_dataset_unref (dataset);
	}

	// If it is primary category, UgCategoryGtk::store == UgCategoryGtk::filter
	// Only primary one has UgCategoryGtk::store
	if (cgtk->store)
		g_object_unref (cgtk->store);
	else if (cgtk->filter)
		g_object_unref (cgtk->filter);
}

// If primary == NULL, this category will be a primary category.
// One uget-gtk program has only one primary category.
// All dataset store in primary category.
// Other categories use filter to classify dataset in primary category.
UgCategory*		ug_category_new_with_gtk (UgCategory* primary)
{
	UgCategory*		category;

	category = ug_category_new ();
	ug_category_use_gtk (category, primary);
	return category;
}

void	ug_category_use_gtk (UgCategory* category, UgCategory* primary)
{
	UgCategoryGtk*	cgtk;

	cgtk = category->user.category;
	if (cgtk == NULL) {
		cgtk = ug_category_gtk_new ();
		category->user.category = cgtk;
		// destroy notify
		category->destroy.func  = (UgNotifyFunc) ug_category_gtk_free;
		category->destroy.data  = category->user.category;
		// functions
		category->funcs = &cgtk_funcs;
	}

	// primary related code -------------------------------
	// If it is primary category, UgCategoryGtk::store == UgCategoryGtk::filter
	// Only primary one has UgCategoryGtk::store
	if (cgtk->store)
		g_object_unref (cgtk->store);
	else if (cgtk->filter)
		g_object_unref (cgtk->filter);

	// If primary == NULL, this is primary category
	if (primary == NULL) {
		cgtk->primary = NULL;
		cgtk->store   = gtk_list_store_new (1, G_TYPE_POINTER);
		cgtk->filter  = GTK_TREE_MODEL (cgtk->store);
	}
	else {
		cgtk->primary = primary->user.category;
		cgtk->store   = NULL;
		cgtk->filter  = gtk_tree_model_filter_new (
				GTK_TREE_MODEL (cgtk->primary->store), NULL);
		gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (cgtk->filter),
				(GtkTreeModelFilterVisibleFunc) ug_download_model_filter_category,
				category, NULL);
	}

	ug_download_widget_use_sortable (&cgtk->all, cgtk->filter);
	// filter
	ug_download_widget_set_filter (&cgtk->active, cgtk->all.model,
			ug_download_model_filter_active, NULL);
	ug_download_widget_set_filter (&cgtk->queuing, cgtk->filter,
			ug_download_model_filter_queuing, NULL);
	ug_download_widget_set_filter (&cgtk->finished, cgtk->filter,
			ug_download_model_filter_finished, NULL);
	ug_download_widget_set_filter (&cgtk->recycled, cgtk->filter,
			ug_download_model_filter_recycled, NULL);
}

void	ug_category_gtk_add (UgCategory* category, UgDataset* dataset)
{
	UgCategoryGtk*		cgtk;
	UgCategoryGtk*		primary;
	UgRelation*			relation;
	GtkTreeModel*		model;
	GtkTreePath*		path;

	// add and set UgRelation to dataset
	relation = UG_DATASET_RELATION (dataset);
	if (relation == NULL)
		relation = ug_dataset_alloc_front (dataset, UG_RELATION_I);
	// GtkTreeIter in relation->user.position
	if (relation->user.position == NULL) {
		relation->user.position = g_slice_alloc (sizeof (GtkTreeIter));
		relation->destroy.func = (UgNotifyFunc) ug_slice_free_tree_iter;
		relation->destroy.data = relation->user.position;
	}

	cgtk = category->user.category;
	primary = (cgtk->primary) ? cgtk->primary : cgtk;
	// If dataset wasn't added to secondary category
	if (relation->category == NULL  &&  cgtk != primary) {
		relation->category = category;
		// if dataset was added to primary GtkListStore
		if (relation->user.storage) {
			model = GTK_TREE_MODEL (relation->user.storage);
			path = gtk_tree_model_get_path (model, relation->user.position);
			gtk_tree_model_row_changed (model, path, relation->user.position);
			gtk_tree_path_free (path);
		}
	}
	// If dataset wasn't added to primary category
	if (relation->user.storage == NULL  &&  primary) {
		// setup relation->user.data for ug_category_gtk_changed().
		relation->hints &= ~UG_HINT_ACTIVE;
		if (relation->hints & UG_HINT_RECYCLED)
			relation->user.data = &cgtk->recycled;
		else if (relation->hints & UG_HINT_FINISHED)
			relation->user.data = &cgtk->finished;
		else
			relation->user.data = &cgtk->queuing;
		// add to primary category
		relation->user.storage = primary->store;
		gtk_list_store_append (relation->user.storage, relation->user.position);
		gtk_list_store_set (relation->user.storage, relation->user.position,
							0, dataset, -1);
		// reference count
		ug_dataset_ref (dataset);
	}
}

GList*	ug_category_gtk_get_all (UgCategory* category)
{
	UgDataset*		dataset;
	GtkTreeModel*	model;
	GtkTreeIter		iter;
	GList*			list;
	gboolean		valid;

	list = NULL;
	model = ((UgCategoryGtk*)category->user.category)->filter;
	valid = gtk_tree_model_get_iter_first (model, &iter);
	while (valid) {
		gtk_tree_model_get (model, &iter, 0, &dataset, -1);
		list = g_list_prepend (list, dataset);
		valid = gtk_tree_model_iter_next (model, &iter);
	}
	return g_list_reverse (list);
}

GList*	ug_category_gtk_get_tasks (UgCategory* category)
{
	UgCategoryGtk*	cgtk;
	UgDataset*		dataset;
	UgRelation*		relation;
	GtkTreeIter		iter;
	gboolean		valid;
	GList*			tasks;
	guint			tasks_len;
	guint			temp_len;

	cgtk = category->user.category;
	temp_len = gtk_tree_model_iter_n_children (cgtk->active.model, NULL);
	if (category->active_limit <= temp_len)
		return NULL;

	tasks = NULL;
	tasks_len = 0;
	temp_len = category->active_limit - temp_len;

	valid = gtk_tree_model_get_iter_first (cgtk->queuing.model, &iter);
	while (valid) {
		gtk_tree_model_get (cgtk->queuing.model, &iter, 0, &dataset, -1);
		valid = gtk_tree_model_iter_next (cgtk->queuing.model, &iter);
		// check
		relation = UG_DATASET_RELATION (dataset);
		if (relation->hints & UG_HINT_UNRUNNABLE)
			continue;
		tasks = g_list_prepend (tasks, dataset);
		tasks_len++;
		if (tasks_len >= temp_len)
			break;
	}

	return g_list_reverse (tasks);
}

void	ug_category_gtk_changed	(UgCategory* category, UgDataset* dataset)
{
	UgDownloadWidget*	dlwidget;
	UgRelation*			relation;
	UgCategoryGtk*		cgtk;
	GtkTreeModel*		model;
	GtkTreePath*		path;

	cgtk     = category->user.category;
	relation = UG_DATASET_RELATION (dataset);

	if (relation->hints & UG_HINT_RECYCLED) {
		dlwidget = &cgtk->recycled;
		relation->hints &= ~UG_HINT_FINISHED;
	}
	else if (relation->hints & UG_HINT_FINISHED) {
		dlwidget = &cgtk->finished;
		relation->hints &= ~UG_HINT_RECYCLED;
	}
	else if (relation->hints & UG_HINT_ACTIVE)
		dlwidget = &cgtk->active;
	else
		dlwidget = &cgtk->queuing;

	// Don't move task to the same UgDownloadWidget.
	if (relation->user.data == &cgtk->active  &&  dlwidget == &cgtk->queuing) {
		relation->user.data = dlwidget;
		gtk_list_store_move_after (relation->user.storage,
				relation->user.position, NULL);
	}
	else if (relation->user.data != dlwidget) {
		relation->user.data = dlwidget;
		if (dlwidget == &cgtk->recycled || dlwidget == &cgtk->finished) {
			gtk_list_store_move_after (relation->user.storage,
					relation->user.position, NULL);
		}
		else {
			gtk_list_store_move_before (relation->user.storage,
					relation->user.position, NULL);
		}
	}

	model = GTK_TREE_MODEL (relation->user.storage);
	path = gtk_tree_model_get_path (model, relation->user.position);
	gtk_tree_model_row_changed (model, path, relation->user.position);
	gtk_tree_path_free (path);
}

void	ug_category_gtk_remove (UgCategory* category, UgDataset* dataset)
{
	UgRelation*		relation;

	relation = UG_DATASET_RELATION (dataset);
	if (relation->user.storage) {
		gtk_list_store_remove (
				relation->user.storage,
				relation->user.position);
	}
	relation->user.storage = NULL;
	relation->category = NULL;
	// delete data and files
	ug_download_delete_temp (dataset);
	ug_dataset_unref (dataset);
}

void	ug_category_gtk_clear (UgCategory* category, UgCategoryHints hint, guint from_nth)
{
	UgCategoryGtk*	cgtk;
	UgDataset*		dataset;
	GtkTreeModel*	model;
	GtkTreeIter		iter;
	GList*			list;
	GList*			link;
	gboolean		valid;

	cgtk = category->user.category;
	if (hint & UG_HINT_RECYCLED)
		model = cgtk->recycled.model;
	else if (hint & UG_HINT_FINISHED)
		model = cgtk->finished.model;
	else
		model = cgtk->queuing.model;

	// get tasks to clear
	list = NULL;
	valid = gtk_tree_model_iter_nth_child (model, &iter, NULL, from_nth);
	while (valid) {
		gtk_tree_model_get (model, &iter, 0, &dataset, -1);
		list = g_list_prepend (list, dataset);
		valid = gtk_tree_model_iter_next (model, &iter);
	}
	// remove tasks
	for (link = list;  link;  link = link->next)
		ug_category_gtk_remove (category, link->data);
	g_list_free (list);
}

void	ug_category_gtk_move_to (UgCategory* category, UgDataset* dataset, UgCategory* category_dest)
{
	UgRelation*		relation;

	relation = UG_DATASET_RELATION (dataset);
	if (relation->category == category_dest)
		return;

	// move
	relation->category  = category_dest;
	relation->user.data = NULL;
	ug_category_gtk_changed (category_dest, dataset);
}

gboolean	ug_category_gtk_move_selected_up (UgCategory* category, UgDownloadWidget* dwidget)
{
	UgDataset*		dataset;
	UgRelation*		relation;
	UgRelation*		relation_prev;
	GtkTreeIter		iter;
	GtkTreePath*	path;
	GList*			list;
	GList*			link;
	gint			index;
	gint			index_prev;
	gboolean		retval = FALSE;

	index_prev = -1;
	list = ug_download_widget_get_selected_indices (dwidget);
	// scroll to first item
	if (list) {
		index = GPOINTER_TO_INT (list->data);
		if (index > 0)
			index--;
		path = gtk_tree_path_new_from_indices (index, -1);
		gtk_tree_view_scroll_to_cell (dwidget->view, path, NULL, FALSE, 0, 0);
		gtk_tree_path_free (path);
	}
	// move
	for (link = list;  link;  link = link->next) {
		index = GPOINTER_TO_INT (link->data);
		if (index == index_prev+1) {
			index_prev++;
			continue;
		}
		index_prev = index - 1;
		// get previous
		gtk_tree_model_iter_nth_child (dwidget->model, &iter, NULL, index_prev);
		gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
		relation_prev = UG_DATASET_RELATION (dataset);
		// get current
		gtk_tree_model_iter_next (dwidget->model, &iter);
		gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
		relation = UG_DATASET_RELATION (dataset);
		// swap
		gtk_list_store_swap (relation->user.storage,
				relation->user.position, relation_prev->user.position);
		retval = TRUE;
	}

	g_list_free (list);
	return retval;
}

gboolean	ug_category_gtk_move_selected_down (UgCategory* category, UgDownloadWidget* dwidget)
{
	UgDataset*		dataset;
	UgRelation*		relation;
	UgRelation*		relation_next;
	GtkTreeIter		iter;
	GtkTreePath*	path;
	GList*			list;
	GList*			link;
	gint			index;
	gint			index_next;
	gboolean		retval = FALSE;

	index_next = gtk_tree_model_iter_n_children (dwidget->model, NULL);
	list = ug_download_widget_get_selected_indices (dwidget);
	link = g_list_last (list);
	// scroll to last item
	if (link) {
		index = GPOINTER_TO_INT (link->data);
		if (index < index_next -1)
			index++;
		path = gtk_tree_path_new_from_indices (index, -1);
		gtk_tree_view_scroll_to_cell (dwidget->view, path, NULL, FALSE, 0, 0);
		gtk_tree_path_free (path);
	}
	// move
	for (;  link;  link = link->prev) {
		index = GPOINTER_TO_INT (link->data);
		if (index == index_next-1) {
			index_next--;
			continue;
		}
		index_next = index + 1;
		// get current
		gtk_tree_model_iter_nth_child (dwidget->model, &iter, NULL, index);
		gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
		relation = UG_DATASET_RELATION (dataset);
		// get next
		gtk_tree_model_iter_next (dwidget->model, &iter);
		gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
		relation_next = UG_DATASET_RELATION (dataset);
		// swap
		gtk_list_store_swap (relation->user.storage,
				relation->user.position, relation_next->user.position);
		retval = TRUE;
	}

	g_list_free (list);
	return retval;
}

gboolean	ug_category_gtk_move_selected_to_top (UgCategory* category, UgDownloadWidget* dwidget)
{
	UgDataset*		dataset;
	UgRelation*		relation;
	UgRelation*		relation_top;
	GtkTreeIter		iter;
	GList*			list;
	GList*			link;
	gint			index;
	gint			index_top;

	// get movable tasks
	relation_top = NULL;
	index_top = 0;
	list = ug_download_widget_get_selected_indices (dwidget);
	for (link=list;  link;  link=link->next, index_top++) {
		index = GPOINTER_TO_INT (link->data);
		if (index == index_top) {
			link->data = NULL;
			continue;
		}
		else {
			gtk_tree_model_iter_nth_child (dwidget->model, &iter, NULL, index);
			gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
			link->data = UG_DATASET_RELATION (dataset);
		}
		if (relation_top == NULL) {
			gtk_tree_model_iter_nth_child (dwidget->model, &iter, NULL, index_top);
			gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
			relation_top = UG_DATASET_RELATION (dataset);
		}
	}
	list = g_list_remove_all (list, NULL);
	if (list == NULL)
		return FALSE;

	// move to top
	for (link = list;  link;  link = link->next) {
		relation = link->data;
		gtk_list_store_move_before (relation->user.storage,
				relation->user.position, relation_top->user.position);
	}
	g_list_free (list);

	// scroll to top
	gtk_tree_view_scroll_to_point (dwidget->view, -1, 0);
	return TRUE;
}

gboolean	ug_category_gtk_move_selected_to_bottom (UgCategory* category, UgDownloadWidget* dwidget)
{
	UgDataset*		dataset;
	UgRelation*		relation;
	UgRelation*		relation_bottom;
	GtkTreeIter		iter;
	GtkTreePath*	path;
	GList*			list;
	GList*			link;
	gint			index;
	gint			index_bottom;

	// get movable tasks
	relation_bottom = NULL;
	index_bottom = gtk_tree_model_iter_n_children (dwidget->model, NULL) - 1;
	list = ug_download_widget_get_selected_indices (dwidget);
	list = g_list_reverse (list);
	for (link=list;  link;  link=link->next, index_bottom--) {
		index = GPOINTER_TO_INT (link->data);
		if (index == index_bottom) {
			link->data = NULL;
			continue;
		}
		else {
			gtk_tree_model_iter_nth_child (dwidget->model, &iter, NULL, index);
			gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
			link->data = UG_DATASET_RELATION (dataset);
		}
		if (relation_bottom == NULL) {
			gtk_tree_model_iter_nth_child (dwidget->model, &iter, NULL, index_bottom);
			gtk_tree_model_get (dwidget->model, &iter, 0, &dataset, -1);
			relation_bottom = UG_DATASET_RELATION (dataset);
		}
	}
	list = g_list_remove_all (list, NULL);
	if (list == NULL)
		return FALSE;

	// move to bottom
	for (link = list;  link;  link = link->next) {
		relation = link->data;
		gtk_list_store_move_after (relation->user.storage,
				relation->user.position, relation_bottom->user.position);
	}
	g_list_free (list);

	// scroll to bottom
	index = gtk_tree_model_iter_n_children (dwidget->model, NULL) -1;
	path = gtk_tree_path_new_from_indices (index, -1);
	gtk_tree_view_scroll_to_cell (dwidget->view, path, NULL, FALSE, 0, 0);
	gtk_tree_path_free (path);
	return TRUE;
}

// If no data cleared, return FALSE.
gboolean	ug_category_gtk_clear_excess (UgCategory* category)
{
	UgCategoryGtk*	cgtk;
	guint			n;
	gboolean		retval = FALSE;

	cgtk = category->user.category;
	n = gtk_tree_model_iter_n_children (cgtk->finished.model, NULL);
	if (n > category->finished_limit)
		retval = TRUE;
	n = gtk_tree_model_iter_n_children (cgtk->recycled.model, NULL);
	if (n > category->recycled_limit)
		retval = TRUE;

	ug_category_gtk_clear (category, UG_HINT_FINISHED, category->finished_limit);
	ug_category_gtk_clear (category, UG_HINT_RECYCLED, category->recycled_limit);

	return retval;
}

// ----------------------------------------------------------------------------
// free GtkTreeIter in UgRelation::user.position
static void		ug_slice_free_tree_iter (GtkTreeIter* iter)
{
	g_slice_free1 (sizeof (GtkTreeIter), iter);
}

// Function used by GtkTreeModelFilter.
static gboolean	ug_download_model_filter_category (GtkTreeModel *model, GtkTreeIter *iter, UgCategory* category)
{
	UgDataset*		dataset;
	UgRelation*		relation;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	if (dataset) {
		relation = UG_DATASET_RELATION (dataset);
		if (relation->category == category)
			return TRUE;
	}
	return FALSE;
}

static gboolean	ug_download_model_filter_active (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	UgDataset*		dataset;
	UgRelation*		relation;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	if (dataset) {
		relation = UG_DATASET_RELATION (dataset);
		if (relation->hints & UG_HINT_ACTIVE)
			return TRUE;
	}
	return FALSE;
}

static gboolean	ug_download_model_filter_queuing (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	UgDataset*		dataset;
	UgRelation*		relation;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	if (dataset) {
		relation = UG_DATASET_RELATION (dataset);
		if (relation->hints & (UG_HINT_FINISHED | UG_HINT_RECYCLED | UG_HINT_ACTIVE))
			return FALSE;
	}
	return TRUE;
}

// Function used by GtkTreeModelFilter.
static gboolean	ug_download_model_filter_finished (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	UgDataset*		dataset;
	UgRelation*		relation;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	if (dataset) {
		relation = UG_DATASET_RELATION (dataset);
		if (relation->hints & UG_HINT_FINISHED)
			return TRUE;
	}
	return FALSE;
}

// Function used by GtkTreeModelFilter.
static gboolean	ug_download_model_filter_recycled (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	UgDataset*		dataset;
	UgRelation*		relation;

	gtk_tree_model_get (model, iter, 0, &dataset, -1);
	if (dataset) {
		relation = UG_DATASET_RELATION (dataset);
		if (relation->hints & UG_HINT_RECYCLED)
			return TRUE;
	}
	return FALSE;
}

