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
#include <UgDownloadDialog.h>
#include <UgetData.h>

#include <glib/gi18n.h>

// static functions
static void	ug_download_dialog_set_page0 (UgDownloadDialog* ddialog, gboolean completed);
static void	ug_download_dialog_set_page1 (UgDownloadDialog* ddialog, gboolean completed);
// signal handlers
static void	on_dialog_show (GtkWidget *widget, UgDownloadDialog* ddialog);
static void	on_button_back (GtkWidget* button, UgDownloadDialog* ddialog);
static void	on_button_forward (GtkWidget* button, UgDownloadDialog* ddialog);
static void	on_category_cursor_changed (GtkTreeView* view, UgDownloadDialog* ddialog);

UgDownloadDialog*	ug_download_dialog_new (const gchar* title, GtkWindow* parent)
{
	UgDownloadDialog*	ddialog;
	GtkWidget*			widget;
	GtkBox*				box;

	ddialog = g_malloc0 (sizeof (UgDownloadDialog));
	ddialog->self = (GtkDialog*) gtk_dialog_new_with_buttons (title, parent,
			GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);
	box = (GtkBox*) gtk_dialog_get_content_area (ddialog->self);
	widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (box, widget, TRUE, TRUE, 0);
	ddialog->hbox = (GtkBox*) widget;
	gtk_widget_show_all (GTK_WIDGET (box));

	// back button
	box = (GtkBox*) gtk_dialog_get_action_area (ddialog->self);
	widget = gtk_button_new_from_stock (GTK_STOCK_GO_BACK);
	gtk_box_pack_start (box, widget, FALSE, FALSE, 0);
	ddialog->button_back = widget;
	g_signal_connect (widget, "clicked", G_CALLBACK (on_button_back), ddialog);
	// forward button
	widget = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
	gtk_box_pack_start (box, widget, FALSE, FALSE, 0);
	ddialog->button_forward = widget;
	g_signal_connect (widget, "clicked", G_CALLBACK (on_button_forward), ddialog);
	// OK & cancel buttons
	gtk_dialog_add_button (ddialog->self, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	gtk_dialog_add_button (ddialog->self, GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_set_default_response (ddialog->self, GTK_RESPONSE_OK);

	// form (Page 1)
	ug_proxy_form_init (&ddialog->proxy);
	ug_download_form_init (&ddialog->download,
			&ddialog->proxy, (GtkWindow*) ddialog->self);
	// initialize page status
	ddialog->page.array[1] = ug_download_from_use_notebook (&ddialog->download, NULL, NULL);
	ddialog->page.completed[1] = FALSE;
	ddialog->page.completed[0] = TRUE;
	ug_download_dialog_set_current_page (ddialog, 1);
	// set callback
	ddialog->download.notify.func = (gpointer) ug_download_dialog_set_page1;
	ddialog->download.notify.data = ddialog;

	// signal
	g_signal_connect_after (ddialog->self, "show",
			G_CALLBACK (on_dialog_show), ddialog);

	return ddialog;
}

void	ug_download_dialog_free (UgDownloadDialog* ddialog)
{
	// remove pages from container and destroy them.
	g_object_ref (ddialog->page.current);
	gtk_container_remove (GTK_CONTAINER (ddialog->hbox), ddialog->page.current);
	if (ddialog->selector.self) {
		gtk_widget_destroy (ddialog->selector.self);
		ug_selector_finalize (&ddialog->selector);
	}
	if (ddialog->download.page1) {
		gtk_widget_destroy (ddialog->page.array[1]);
//		gtk_widget_destroy (ddialog->download.page1);
//		gtk_widget_destroy (ddialog->download.page2);
	}
	if (ddialog->batch.self)
		gtk_widget_destroy (ddialog->batch.self);
	// free external data
	if (ddialog->dataset)
		ug_dataset_unref (ddialog->dataset);
	// destroy dialog
	gtk_widget_destroy (GTK_WIDGET (ddialog->self));
	g_free (ddialog);
}

void	ug_download_dialog_use_batch (UgDownloadDialog* ddialog)
{
	GtkRequisition		requisition;

	// (Page 0)
	ug_batch_form_init (&ddialog->batch);
	ug_download_form_set_multiple (&ddialog->download, TRUE);
	gtk_widget_get_preferred_size (ddialog->download.page1, &requisition, NULL);
	gtk_widget_set_size_request (ddialog->batch.self,
			requisition.width, requisition.height);
	// setup page and button
	ddialog->page.array[0] = ddialog->batch.self;
	ddialog->page.completed[0] = FALSE;
	ddialog->page.completed[1] = TRUE;
	ug_download_dialog_set_current_page (ddialog, 0);
	gtk_widget_show (ddialog->button_back);
	gtk_widget_show (ddialog->button_forward);
	// set callback
	ddialog->batch.notify.func = (gpointer) ug_download_dialog_set_page0;
	ddialog->batch.notify.data = ddialog;
}

void	ug_download_dialog_use_selector (UgDownloadDialog* ddialog)
{
	GtkRequisition		requisition;

	// (Page 0)
	ug_selector_init (&ddialog->selector, (GtkWindow*) ddialog->self);
	ug_download_form_set_multiple (&ddialog->download, TRUE);
	gtk_widget_get_preferred_size (ddialog->download.page1, &requisition, NULL);
	gtk_widget_set_size_request (ddialog->selector.self,
			requisition.width, requisition.height);
	// setup page and button
	ddialog->page.array[0] = ddialog->selector.self;
	ddialog->page.completed[0] = FALSE;
	ddialog->page.completed[1] = TRUE;
	ug_download_dialog_set_current_page (ddialog, 0);
	gtk_widget_show (ddialog->button_back);
	gtk_widget_show (ddialog->button_forward);
	// set callback
	ddialog->selector.notify.func = (gpointer) ug_download_dialog_set_page0;
	ddialog->selector.notify.data = ddialog;
}

void	ug_download_dialog_set_category (UgDownloadDialog* ddialog, UgCategoryWidget* cwidget)
{
	GtkTreeModel*	model;
	GtkTreePath*	path;
	GtkWidget*		scrolled;
	GtkBox*			vbox;

	ddialog->category_view = ug_category_view_new ();
	ug_category_view_set_icon_visible (ddialog->category_view, FALSE);
	model = cwidget->filter;
	gtk_tree_view_set_model (ddialog->category_view, model);
	// signal
	g_signal_connect (ddialog->category_view, "cursor-changed",
			G_CALLBACK (on_category_cursor_changed), ddialog);
	// select current category and emit "cursor-changed" signal.
	gtk_tree_view_get_cursor (cwidget->view, &path, NULL);
	if (path == NULL)
		path = gtk_tree_path_new_from_indices (0, -1);
	else if (gtk_tree_path_get_depth (path) == 2)
		gtk_tree_path_up (path);
	gtk_tree_view_set_cursor (ddialog->category_view, path, NULL, FALSE);
	gtk_tree_path_free (path);
	// scrolled window
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_set_size_request (scrolled, 165, 100);
	gtk_widget_show (scrolled);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled),
			GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled),
			GTK_WIDGET (ddialog->category_view));
	// pack vbox
	vbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_pack_start (vbox, gtk_label_new (_("Category")), FALSE, FALSE, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) scrolled, TRUE, TRUE, 0);
	gtk_widget_show_all ((GtkWidget*) vbox);
	// pack hbox
	gtk_box_pack_start (ddialog->hbox, (GtkWidget*) vbox, FALSE, FALSE, 1);
}

void	ug_download_dialog_set_current_page (UgDownloadDialog* ddialog, gint nth_page)
{
	if (ddialog->page.current) {
		g_object_ref (ddialog->page.current);
		gtk_container_remove (GTK_CONTAINER (ddialog->hbox), ddialog->page.current);
		ddialog->page.current = NULL;
	}

	if (nth_page == 0 && ddialog->page.array[0]) {
		ddialog->page.current = ddialog->page.array[0];
		gtk_box_pack_end (ddialog->hbox, ddialog->page.current, TRUE, TRUE, 1);
		gtk_widget_set_sensitive (ddialog->button_back,    FALSE);
		gtk_widget_set_sensitive (ddialog->button_forward, TRUE);
	}
	else if (nth_page == 1) {
		ddialog->page.current = ddialog->page.array[1];
		gtk_box_pack_end (ddialog->hbox, ddialog->page.current, TRUE, TRUE, 1);
		gtk_widget_set_sensitive (ddialog->button_back,    TRUE);
		gtk_widget_set_sensitive (ddialog->button_forward, FALSE);
	}
}

void	ug_download_dialog_get (UgDownloadDialog* ddialog, UgDataset* dataset)
{
	ug_proxy_form_get (&ddialog->proxy, dataset);
	ug_download_form_get (&ddialog->download, dataset);
}

void	ug_download_dialog_set (UgDownloadDialog* ddialog, UgDataset* dataset)
{
	ug_proxy_form_set (&ddialog->proxy, dataset, FALSE);
	ug_download_form_set (&ddialog->download, dataset, FALSE);
}

UgCategory*	ug_download_dialog_get_category (UgDownloadDialog* ddialog)
{
	UgCategory*		category;
	GtkTreeModel*	model;
	GtkTreePath*	path;
	GtkTreeIter		iter;

	gtk_tree_view_get_cursor (ddialog->category_view, &path, NULL);
	model = gtk_tree_view_get_model (ddialog->category_view);
	if (path) {
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_tree_path_free (path);
	}
	else if (gtk_tree_model_get_iter_first (model, &iter) == FALSE)
		return NULL;

	gtk_tree_model_get (model, &iter, 0, &category, -1);
	return category;
}

GList*	ug_download_dialog_get_downloads (UgDownloadDialog* ddialog)
{
	UgDataset*		dataset;
	UgetCommon*	common;
	GList*			list;
	GList*			link;

	// batch
	if (ddialog->batch.self) {
		list = ug_batch_form_get_list (&ddialog->batch, FALSE);
		for (link = list;  link;  link = link->next) {
			dataset = ug_dataset_new ();
			ug_download_dialog_get (ddialog, dataset);
			common = ug_dataset_realloc (dataset, UgetCommonInfo, 0);
			g_free (common->url);
			common->url = link->data;
			ug_download_complete_data (dataset);
			link->data = dataset;
		}
		return list;
	}

	// selector
	if (ddialog->selector.self) {
		dataset = ug_dataset_new ();
		ug_download_dialog_get (ddialog, dataset);
		list = ug_selector_get_marked_downloads (&ddialog->selector);
		for (link = list;  link;  link = link->next) {
			ug_data_assign (link->data, dataset);
			ug_download_complete_data (link->data);
		}
		ug_dataset_unref (dataset);
		return list;
	}

	// external data
	if (ddialog->dataset) {
		// ug_download_dialog_free() will call ug_dataset_unref()
		dataset = ddialog->dataset;
		ug_dataset_ref (dataset);
	}
	else
		dataset = ug_dataset_new ();
	ug_download_dialog_get (ddialog, dataset);
	list = g_list_prepend (NULL, dataset);
	return list;
}

// ----------------------------------------------------------------------------
// static functions
//
static void	ug_download_dialog_decide_completed (UgDownloadDialog* ddialog)
{
	GtkTreePath*	path;
	gboolean		completed;

	// enable or disable GTK_RESPONSE_OK
	if (ddialog->page.completed[0]==TRUE && ddialog->page.completed[1]==TRUE)
		completed = TRUE;
	else
		completed = FALSE;
	// if no category selected, disable GTK_RESPONSE_OK.
	if (ddialog->category_view) {
		gtk_tree_view_get_cursor (ddialog->category_view, &path, NULL);
		if (path)
			gtk_tree_path_free (path);
		else
			completed = FALSE;
	}
	gtk_dialog_set_response_sensitive (ddialog->self, GTK_RESPONSE_OK, completed);
}

static void	ug_download_dialog_set_page0 (UgDownloadDialog* ddialog, gboolean completed)
{
	ddialog->page.completed[0] = completed;
	gtk_widget_set_sensitive (ddialog->button_forward, completed);
	ug_download_dialog_decide_completed (ddialog);
}

static void	ug_download_dialog_set_page1 (UgDownloadDialog* ddialog, gboolean completed)
{
	ddialog->page.completed[1] = completed;
	ug_download_dialog_decide_completed (ddialog);
}

// ----------------------------------------------------------------------------
// signal handler
//
static void	on_dialog_show (GtkWidget *widget, UgDownloadDialog* ddialog)
{
	// emit callback & set focus
	if (ddialog->batch.self) {
		ug_batch_form_update_preview (&ddialog->batch);
		gtk_window_set_focus ((GtkWindow*) ddialog->self,
				(GtkWidget*) ddialog->batch.entry);
	}
	else if (ddialog->selector.self) {
		ug_selector_count_marked (&ddialog->selector);
		gtk_window_set_focus ((GtkWindow*) ddialog->self,
				(GtkWidget*) ddialog->selector.notebook);
	}
	else if (gtk_widget_is_sensitive (ddialog->download.url_entry)) {
		gtk_window_set_focus ((GtkWindow*) ddialog->self,
				(GtkWidget*) ddialog->download.url_entry);
	}
	else {
		gtk_window_set_focus ((GtkWindow*) ddialog->self,
				(GtkWidget*) ddialog->download.folder_entry);
	}
}

static void	on_button_back (GtkWidget* button, UgDownloadDialog* ddialog)
{
	ug_download_dialog_set_current_page (ddialog, 0);
}

static void	on_button_forward (GtkWidget* button, UgDownloadDialog* ddialog)
{
	ug_download_dialog_set_current_page (ddialog, 1);
}

static void	on_category_cursor_changed (GtkTreeView* view, UgDownloadDialog* ddialog)
{
	GtkTreeModel*	model;
	GtkTreePath*	path;
	GtkTreeIter		iter;
	UgCategory*		category;

	gtk_tree_view_get_cursor (view, &path, NULL);
	if (path == NULL) {
		gtk_dialog_set_response_sensitive (ddialog->self, GTK_RESPONSE_OK, FALSE);
		return;
	}
	if (ddialog->page.completed[0]==TRUE && ddialog->page.completed[1]==TRUE)
		gtk_dialog_set_response_sensitive (ddialog->self, GTK_RESPONSE_OK, TRUE);

	// apply settings
	model = gtk_tree_view_get_model (view);
	gtk_tree_view_get_cursor (view, &path, NULL);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_path_free (path);
	gtk_tree_model_get (model, &iter, 0, &category, -1);
	if (category->defaults) {
		ug_download_form_set (&ddialog->download, category->defaults, TRUE);
		ug_proxy_form_set (&ddialog->proxy, category->defaults, TRUE);
	}
}

