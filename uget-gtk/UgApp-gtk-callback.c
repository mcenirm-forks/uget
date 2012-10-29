static const char	uget_license[] =
{
" Copyright (C) 2005-2012 by C.H. Huang"								"\n"
" plushuang.tw@gmail.com"												"\n"
																		"\n"
"This library is free software; you can redistribute it and/or"			"\n"
"modify it under the terms of the GNU Lesser General Public"			"\n"
"License as published by the Free Software Foundation; either"			"\n"
"version 2.1 of the License, or (at your option) any later version."	"\n"
																		"\n"
"This library is distributed in the hope that it will be useful,"		"\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of"		"\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU"		"\n"
"Lesser General Public License for more details."						"\n"
																		"\n"
"You should have received a copy of the GNU Lesser General Public"		"\n"
"License along with this library; if not, write to the Free Software"	"\n"
"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA"	"\n"
																		"\n"
"---------"																"\n"
																		"\n"
"In addition, as a special exception, the copyright holders give"		"\n"
"permission to link the code of portions of this program with the"		"\n"
"OpenSSL library under certain conditions as described in each"			"\n"
"individual source file, and distribute linked combinations"			"\n"
"including the two."													"\n"
"You must obey the GNU Lesser General Public License in all respects"	"\n"
"for all of the code used other than OpenSSL.  If you modify"			"\n"
"file(s) with this exception, you may extend this exception to your"	"\n"
"version of the file(s), but you are not obligated to do so.  If you"	"\n"
"do not wish to do so, delete this exception statement from your"		"\n"
"version.  If you delete this exception statement from all source"		"\n"
"files in the program, then also delete it here."						"\n"	"\0"
};

#ifdef _WIN32
#include <windows.h>
#endif

#include <gdk/gdkkeysyms.h>    // for GDK_KEY_xxx...

// uglib
#include <UgHtml.h>
#include <UgUtils.h>
#include <UgData-download.h>
#include <UgDownloadDialog.h>
#include <UgCategoryDialog.h>
#include <UgSettingDialog.h>
#include <UgApp-gtk.h>

#include <glib/gi18n.h>

#define	UGET_URL_WEBSITE	"http://uget.visuex.com/"

// static data
static const gchar*	uget_authors[] = { "C.H. Huang  (\xE9\xBB\x83\xE6\xAD\xA3\xE9\x9B\x84)", NULL };
static const gchar*	uget_artists[] =
{
	"Logo designer: Michael Tunnell (visuex.com)",
	"Website by: Michael Tunnell (visuex.com)",
	"Former Logo designer: saf1 (linuxac.org)",
	"Former Logo improver: Skeleton_Eel (linuxac.org)",
	NULL
};
static const gchar*	uget_version   = UG_APP_GTK_VERSION;
static const gchar*	uget_comments  = N_("Download Manager");
static const gchar*	uget_copyright = "Copyright (C) 2005-2012 C.H. Huang";
static const gchar*	translator_credits = N_("translator-credits");

// static functions
static void	ug_window_init_callback    (struct UgWindow*  window,  UgAppGtk* app);
static void	ug_toolbar_init_callback   (struct UgToolbar* toolbar, UgAppGtk* app);
static void	ug_menubar_init_callback   (struct UgMenubar* menubar, UgAppGtk* app);
static void	ug_trayicon_init_callback (struct UgTrayIcon* icon,   UgAppGtk* app);

static GtkWidget*	create_file_chooser (const gchar* title, GtkWindow* parent, const gchar* filter_name, const gchar* mine_type);


void	ug_app_init_callback (UgAppGtk* app)
{
//	gtk_accel_group_connect (app->accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (ug_app_quit), app, NULL));
//	gtk_accel_group_connect (app->accel_group, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (ug_app_save), app, NULL));
//	gtk_accel_group_connect (app->accel_group, GDK_KEY_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (on_summary_copy_selected), app, NULL));
	ug_window_init_callback  (&app->window,  app);
	ug_toolbar_init_callback (&app->toolbar, app);
	ug_menubar_init_callback (&app->menubar, app);
	ug_trayicon_init_callback (&app->trayicon, app);
}

// ----------------------------------------------------------------------------
// Category
//
static void	on_create_category_response (GtkDialog *dialog, gint response_id, UgCategoryDialog* cdialog)
{
	UgCategory*		category;
	UgAppGtk*		app;

	if (response_id == GTK_RESPONSE_OK) {
		app = cdialog->user.app;
		ug_download_form_get_folder_list (&cdialog->download,
				&app->setting.folder_list);
		category = ug_category_new_with_gtk (app->cwidget.primary.category);
		ug_category_dialog_get (cdialog, category);
		ug_category_widget_append (&app->cwidget, category);
		ug_app_menubar_sync_category (app, TRUE);
	}
	ug_category_dialog_free (cdialog);
}

static void	on_create_category (GtkWidget* widget, UgAppGtk* app)
{
	UgCategoryDialog*	cdialog;
	gchar*				string;

	string = g_strconcat (UG_APP_GTK_NAME " - ", _("New Category"), NULL);
	cdialog = ug_category_dialog_new (string, app->window.self);
	g_free (string);
	ug_download_form_set_folder_list (&cdialog->download,
			app->setting.folder_list);
	// copy setting from current category
	if (app->cwidget.current.category != app->cwidget.primary.category) {
		ug_category_dialog_set (cdialog, app->cwidget.current.category);
		string = g_strconcat (_("Copy - "), app->cwidget.current.category->name, NULL);
		gtk_entry_set_text (GTK_ENTRY (cdialog->category.name_entry), string);
		g_free (string);
	}
	else {
		string = _("New Category");
		gtk_entry_set_text (GTK_ENTRY (cdialog->category.name_entry), string);
	}
	// show category dialog
	cdialog->user.app = app;
	g_signal_connect (cdialog->self, "response",
			G_CALLBACK (on_create_category_response), cdialog);
	gtk_widget_show ((GtkWidget*) cdialog->self);
}

static void	on_delete_category (GtkWidget* widget, UgAppGtk* app)
{
	UgCategory*		current;
	GtkWidget*		download_view;

	if (app->cwidget.current.category == app->cwidget.primary.category)
		return;
	// remove current view before removing category
	download_view = app->cwidget.current.widget->self;
	g_object_ref (download_view);
	gtk_container_remove (GTK_CONTAINER (app->window.vpaned), download_view);
	// remove and delete category
	current = app->cwidget.current.category;
	ug_category_widget_remove (&app->cwidget, current);
	// refresh
	gtk_widget_queue_draw ((GtkWidget*) app->cwidget.primary.view);
	ug_app_menubar_sync_category (app, TRUE);
}

static void	on_config_category_response (GtkDialog *dialog, gint response_id, UgCategoryDialog* cdialog)
{
	UgAppGtk*		app;

	app = cdialog->user.app;
	if (response_id == GTK_RESPONSE_OK) {
		ug_download_form_get_folder_list (&cdialog->download,
				&app->setting.folder_list);
		ug_category_dialog_get (cdialog, app->cwidget.current.category);
	}
	gtk_widget_set_sensitive ((GtkWidget*)app->window.self, TRUE);
	ug_category_dialog_free (cdialog);
}

static void	on_config_category (GtkWidget* widget, UgAppGtk* app)
{
	UgCategoryDialog*	cdialog;
	gchar*				title;

	if (app->cwidget.current.category == app->cwidget.primary.category)
		return;
	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Category Properties"), NULL);
	cdialog = ug_category_dialog_new (title, app->window.self);
	g_free (title);
	ug_download_form_set_folder_list (&cdialog->download,
			app->setting.folder_list);
	ug_category_dialog_set (cdialog, app->cwidget.current.category);
	// show category dialog
	cdialog->user.app = app;
	g_signal_connect (cdialog->self, "response",
			G_CALLBACK (on_config_category_response), cdialog);
	gtk_widget_set_sensitive ((GtkWidget*)app->window.self, FALSE);
	gtk_widget_show ((GtkWidget*) cdialog->self);
}

// ----------------------------------------------------------------------------
// Download
//
static void	ug_app_setup_download_dialog (UgAppGtk* app, UgDownloadDialog* ddialog)
{
	UgDataCommon*	common;
	GtkTreePath*	path;
	GtkTreeModel*	model;

	ug_download_form_set_folder_list (&ddialog->download,
			app->setting.folder_list);
	ug_download_dialog_set_category (ddialog, &app->cwidget);

	if (app->setting.ui.apply_recently && app->last.download) {
		model = gtk_tree_view_get_model (ddialog->category_view);
		if (app->last.category_index < gtk_tree_model_iter_n_children (model, NULL)) {
			path = gtk_tree_path_new_from_indices (app->last.category_index, -1);
			gtk_tree_view_set_cursor (ddialog->category_view, path, NULL, FALSE);
			gtk_tree_path_free (path);
		}

		common = ug_dataset_realloc (app->last.download, UG_DATA_COMMON_I, 0);
		if (common && common->file) {
			g_free (common->file);
			common->file = NULL;
		}
		ug_download_dialog_set (ddialog, app->last.download);
	}
}

static void	on_create_download_response (GtkDialog *dialog, gint response_id, UgDownloadDialog* ddialog)
{
	UgCategory*		category;
	UgAppGtk*		app;
	GList*			list;
	GList*			link;
	GtkTreePath*	path;

	if (response_id == GTK_RESPONSE_OK) {
		app = ddialog->user.app;
		ug_download_form_get_folder_list (&ddialog->download,
				&app->setting.folder_list);
		category = ug_download_dialog_get_category (ddialog);
		if (category) {
			list = ug_download_dialog_get_downloads (ddialog);
			for (link = list;  link;  link = link->next)
				ug_category_add (category, link->data);
			// get last download settings
			if (list->data) {
				if (app->last.download)
					ug_dataset_unref (app->last.download);
				ug_dataset_ref (list->data);
				app->last.download = list->data;
			}
			// get last category index
			gtk_tree_view_get_cursor (ddialog->category_view, &path, NULL);
			if (path) {
				app->last.category_index = *gtk_tree_path_get_indices (path);
				gtk_tree_path_free (path);
			}
			// free unused
			g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
			g_list_free (list);
			gtk_widget_queue_draw ((GtkWidget*) app->cwidget.self);
		}
	}
	ug_download_dialog_free (ddialog);
}

static void	on_create_download (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadDialog*	ddialog;
	gchar*				title;
	GList*				list;

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("New Download"), NULL);
	ddialog = ug_download_dialog_new (title, app->window.self);
	g_free (title);
	if (gtk_widget_get_visible ((GtkWidget*) app->window.self) == FALSE)
		gtk_window_set_transient_for ((GtkWindow*) ddialog->self, NULL);
	// setup download dialog
	ug_app_setup_download_dialog (app, ddialog);
	// use first URI from clipboard to set URL entry
	list = ug_clipboard_get_uris (&app->clipboard);
	if (list) {
		ddialog->download.changed.url = TRUE;
		gtk_entry_set_text ((GtkEntry*) ddialog->download.url_entry, list->data);
		g_list_foreach (list, (GFunc) g_free, NULL);
		g_list_free (list);
		ug_download_form_complete_entry (&ddialog->download);
	}
	// connect signal and set data in download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	on_create_sequence (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadDialog*	ddialog;
	gchar*				title;

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("URL Sequence batch"), NULL);
	ddialog = ug_download_dialog_new (title, app->window.self);
	g_free (title);
	ug_download_dialog_use_batch (ddialog);
	// setup download dialog
	ug_app_setup_download_dialog (app, ddialog);
	// connect signal and set data in download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show (GTK_WIDGET (ddialog->self));
}

static void	on_create_from_clipboard (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadDialog*	ddialog;
	UgSelectorPage*		page;
	GList*				list;
	gchar*				title;

	list = ug_clipboard_get_uris (&app->clipboard);
	if (list == NULL) {
		ug_app_show_message (app, GTK_MESSAGE_ERROR,
				_("No URLs found in clipboard."));
		return;
	}

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Clipboard batch"), NULL);
	ddialog = ug_download_dialog_new (title, app->window.self);
	g_free (title);
	if (gtk_widget_get_visible ((GtkWidget*) app->window.self) == FALSE)
		gtk_window_set_transient_for ((GtkWindow*) ddialog->self, NULL);
	ug_download_dialog_use_selector (ddialog);
	// selector
	ug_selector_hide_href (&ddialog->selector);
	page = ug_selector_add_page (&ddialog->selector, _("Clipboard"));
	ug_selector_page_add_uris (page, list);
	g_list_free (list);
	// setup download dialog
	ug_app_setup_download_dialog (app, ddialog);
	// connect signal and set data in download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show (GTK_WIDGET (ddialog->self));
}

static void	on_create_torrent_response (GtkWidget* dialog, gint response, UgAppGtk* app)
{
	UgDownloadDialog*	ddialog;
	gchar*				string;
	gchar*				uri;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// get filename
	uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);

	string = g_strconcat (UG_APP_GTK_NAME " - ", _("New Torrent"), NULL);
	ddialog = ug_download_dialog_new (string, app->window.self);
	g_free (string);
	if (gtk_widget_get_visible ((GtkWidget*) app->window.self) == FALSE)
		gtk_window_set_transient_for ((GtkWindow*) ddialog->self, NULL);
	ug_download_form_set_folder_list (&ddialog->download,
			app->setting.folder_list);
	ug_download_dialog_set_category (ddialog, &app->cwidget);
	gtk_entry_set_text (GTK_ENTRY (ddialog->download.url_entry), uri);
	g_free (uri);
	// connect signal and set data in download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	on_create_torrent (GtkWidget* widget, UgAppGtk* app)
{
	GtkWidget*		dialog;
	gchar*			title;

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Open Torrent file"), NULL);
	dialog = create_file_chooser (title, app->window.self,
			"Torrent file (*.torrent)", "application/x-bittorrent");
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_create_torrent_response), app);
	gtk_widget_show (dialog);
}

static void	on_create_metalink_response (GtkWidget* dialog, gint response, UgAppGtk* app)
{
	UgDownloadDialog*	ddialog;
	gchar*				string;
	gchar*				uri;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// get filename
	uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);

	string = g_strconcat (UG_APP_GTK_NAME " - ", _("New Metalink"), NULL);
	ddialog = ug_download_dialog_new (string, app->window.self);
	g_free (string);
	if (gtk_widget_get_visible ((GtkWidget*) app->window.self) == FALSE)
		gtk_window_set_transient_for ((GtkWindow*) ddialog->self, NULL);
	ug_download_form_set_folder_list (&ddialog->download,
			app->setting.folder_list);
	ug_download_dialog_set_category (ddialog, &app->cwidget);
	gtk_entry_set_text (GTK_ENTRY (ddialog->download.url_entry), uri);
	g_free (uri);
	// connect signal and set data in download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	on_create_metalink (GtkWidget* widget, UgAppGtk* app)
{
	GtkWidget*		dialog;
	GtkFileFilter*	filter;
	gchar*			title;

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Open Metalink file"), NULL);
	dialog = gtk_file_chooser_dialog_new (title, app->window.self,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK,     GTK_RESPONSE_OK,
			NULL);
	g_free (title);
	gtk_window_set_destroy_with_parent ((GtkWindow*) dialog, TRUE);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, "Metalink file (*.metalink, *.meta4)");
	gtk_file_filter_add_pattern (filter, "*.metalink");
	gtk_file_filter_add_pattern (filter, "*.meta4");
//	gtk_file_filter_add_mime_type (filter, "application/metalink+xml");
//	gtk_file_filter_add_mime_type (filter, "application/metalink4+xml");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_create_metalink_response), app);
	gtk_widget_show (dialog);
}

static void	on_delete_download (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgRelation*			relation;
	GList*				list;
	GList*				link;
	// check shift key status
	GdkWindow*			gdk_win;
	GdkDevice*			dev_pointer;
	GdkModifierType		mask;

	dwidget = app->cwidget.current.widget;
	// check shift key status
	gdk_win = gtk_widget_get_parent_window ((GtkWidget*) dwidget->view);
	dev_pointer = gdk_device_manager_get_client_pointer (
			gdk_display_get_device_manager (gdk_window_get_display (gdk_win)));
	gdk_window_get_device_position (gdk_win, dev_pointer, NULL, NULL, &mask);

	// clear summary
	ug_summary_show (&app->summary, NULL);
	// set action status "stop by user" and "deleted"
	app->action.stop = TRUE;
	app->action.deleted = TRUE;

	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		// stop task
		ug_running_remove (&app->running, link->data);
		// delete data or move it to recycled
		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		if ((relation->hints & UG_HINT_RECYCLED) || (mask & GDK_SHIFT_MASK))
			ug_category_gtk_remove (relation->category, link->data);
		else {
			relation->hints |= UG_HINT_RECYCLED;
			ug_category_gtk_changed (relation->category, link->data);
		}
	}
	g_list_free (list);

	app->action.deleted = FALSE;
	// update
	gtk_widget_queue_draw ((GtkWidget*) app->cwidget.self);
	ug_summary_show (&app->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_delete_download_file_response (GtkWidget* widget, gint response_id, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgDataCommon*		common;
	gchar*				path;
	GList*				list;
	GList*				link;

	if (response_id != GTK_RESPONSE_YES)
		return;

	dwidget = app->cwidget.current.widget;
	// clear summary
	ug_summary_show (&app->summary, NULL);
	// set action status "stop by user" and "deleted"
	app->action.stop = TRUE;
	app->action.deleted = TRUE;

	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		// stop task
		ug_running_remove (&app->running, link->data);
		// delete file
		common = UG_DATASET_COMMON ((UgDataset*) link->data);
		if (common->folder) {
			path = g_build_filename (common->folder, common->file, NULL);
			ug_delete_file (path);
			g_free (path);
		}
		else if (common->file)
			ug_delete_file (common->file);
		// delete data
		ug_category_gtk_remove (app->cwidget.current.category, link->data);
	}
	g_list_free (list);

	app->action.deleted = FALSE;
	// update
	gtk_widget_queue_draw ((GtkWidget*) app->cwidget.self);
	ug_summary_show (&app->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_delete_download_file (GtkWidget* widget, UgAppGtk* app)
{
	if (app->setting.ui.delete_confirmation == FALSE)
		on_delete_download_file_response (widget, GTK_RESPONSE_YES, app);
	else {
		ug_app_confirm_to_delete (app,
				G_CALLBACK (on_delete_download_file_response), app);
	}
}

static void	on_open_download_file (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgDataCommon*		common;
	UgDataset*			dataset;
	GtkWidget*			dialog;
	gchar*				string;

	dwidget = app->cwidget.current.widget;
	dataset = ug_download_widget_get_cursor (dwidget);
	if (dataset == NULL)
		return;
	common = ug_dataset_get (dataset, UG_DATA_COMMON_I, 0);
	if (common->folder == NULL || common->file == NULL)
		return;

	if (ug_launch_default_app (common->folder, common->file) == FALSE) {
		string = g_strdup_printf (_("Can't launch default application for file '%s'."), common->file);
		dialog = gtk_message_dialog_new (app->window.self,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"%s", string);
		g_free (string);
		string = g_strconcat (UG_APP_GTK_NAME " - ", _("Error"), NULL);
		gtk_window_set_title ((GtkWindow*) dialog, string);
		g_free (string);
		g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show ((GtkWidget*) dialog);
	}
}

static void	on_open_download_folder (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgDataCommon*		common;
	UgDataset*			dataset;
	GtkWidget*			dialog;
	gchar*				string;

	dwidget = app->cwidget.current.widget;
	dataset = ug_download_widget_get_cursor (dwidget);
	if (dataset == NULL)
		return;
	common = ug_dataset_get (dataset, UG_DATA_COMMON_I, 0);
	if (common->folder == NULL)
		return;

	string = g_filename_from_utf8 (common->folder, -1, NULL, NULL, NULL);
	if (g_file_test (string, G_FILE_TEST_EXISTS) == FALSE) {
		g_free (string);
		string = g_strdup_printf (_("'%s' - This folder does not exist."), common->folder);
		dialog = gtk_message_dialog_new (app->window.self,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"%s", string);
		g_free (string);
		string = g_strconcat (UG_APP_GTK_NAME " - ", _("Error"), NULL);
		gtk_window_set_title ((GtkWindow*) dialog, string);
		g_free (string);
		g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show ((GtkWidget*) dialog);
		return;
	}
	g_free (string);

#ifdef _WIN32
	{
		gchar*		path;
		gchar*		argument;
		gunichar2*	argument_os;

		path = g_build_filename (common->folder, common->file, NULL);
		if (g_file_test (path, G_FILE_TEST_EXISTS))
			argument = g_strconcat ("/e,/select,\"", path, "\"", NULL);
		else
			argument = g_strconcat ("/e,\"", common->folder, "\"", NULL);
		g_free (path);
		argument_os = g_utf8_to_utf16 (argument, -1, NULL, NULL, NULL);
		g_free (argument);
		ShellExecuteW (NULL, NULL, L"explorer", argument_os, NULL, SW_SHOW);
		g_free (argument_os);
	}
#else
	{
		GError*	error = NULL;
		GFile*	gfile;
		gchar*	uri;

		gfile = g_file_new_for_path (common->folder);
		uri = g_file_get_uri (gfile);
		g_object_unref (gfile);
		g_app_info_launch_default_for_uri (uri, NULL, &error);
		g_free (uri);

		if (error)
			g_error_free (error);
	}
#endif
}

static void	on_config_download_response (GtkDialog *dialog, gint response_id, UgDownloadDialog* ddialog)
{
	UgDownloadWidget*	dwidget;
	UgAppGtk*			app;
	GList*				list;
	GList*				link;

	app = ddialog->user.app;
	dwidget = app->cwidget.current.widget;
	if (response_id == GTK_RESPONSE_OK) {
		ug_download_form_get_folder_list (&ddialog->download,
				&app->setting.folder_list);
		list = ug_download_widget_get_selected (dwidget);
		for (link = list;  link;  link = link->next)
			ug_download_dialog_get (ddialog, link->data);
		g_list_free (list);
	}
	ug_download_dialog_free (ddialog);
	// refresh other data & status
	gtk_widget_set_sensitive ((GtkWidget*) app->window.self, TRUE);
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&app->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_config_download (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadDialog*	ddialog;
	gchar*				title;
	GList*				list;

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Download Properties"), NULL);
	ddialog = ug_download_dialog_new (title, app->window.self);
	g_free (title);
	// UgDownloadForm
	list = ug_download_widget_get_selected (app->cwidget.current.widget);
	ug_download_form_set_multiple (&ddialog->download,
			(list->next) ? TRUE : FALSE);
	ug_download_form_set_folder_list (&ddialog->download,
			app->setting.folder_list);
	ug_download_form_set_relation (&ddialog->download, FALSE);
	ug_download_dialog_set (ddialog, list->data);
	g_list_free (list);
	// connect signal and set data in download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_config_download_response), ddialog);
	gtk_widget_set_sensitive ((GtkWidget*) app->window.self, FALSE);
	gtk_widget_show (GTK_WIDGET (ddialog->self));
}

static void	on_set_download_force_start (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgRelation*			relation;
	GList*				list;
	GList*				link;

	dwidget = app->cwidget.current.widget;
	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		relation->hints &= ~UG_HINT_UNRUNNABLE;
		ug_running_add (&app->running, link->data);
	}
	g_list_free (list);
	// refresh other data & status
	gtk_widget_queue_draw (app->cwidget.self);
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&app->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_set_download_runnable (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgRelation*			relation;
	GList*				list;
	GList*				link;

	dwidget = app->cwidget.current.widget;
	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		relation->hints &= ~UG_HINT_UNRUNNABLE;
		// If task is in Finished or Recycled, move it to Queuing.
		ug_category_gtk_changed (relation->category, link->data);
	}
	g_list_free (list);
	// refresh other data & status
	gtk_widget_queue_draw (app->cwidget.self);
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&app->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_set_download_to_pause (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgRelation*			relation;
	GList*				list;
	GList*				link;

	// set action status "stop by user"
	app->action.stop = TRUE;

	dwidget = app->cwidget.current.widget;
	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		relation->hints |=  UG_HINT_PAUSED;
		// stop task
		ug_running_remove (&app->running, link->data);
	}
	g_list_free (list);
	// refresh other data & status
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&app->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_move_download_up (GtkWidget* widget, UgAppGtk* app)
{
	if (app->cwidget.current.category == NULL)
		return;

	if (ug_category_gtk_move_selected_up (app->cwidget.current.category,
			app->cwidget.current.widget))
	{
		gtk_widget_set_sensitive (app->toolbar.move_down, TRUE);
		gtk_widget_set_sensitive (app->toolbar.move_bottom, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_down, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_bottom, TRUE);
	}
	else {
		gtk_widget_set_sensitive (app->toolbar.move_up, FALSE);
		gtk_widget_set_sensitive (app->toolbar.move_top, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.move_up, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.move_top, FALSE);
	}
}

static void	on_move_download_down (GtkWidget* widget, UgAppGtk* app)
{
	if (app->cwidget.current.category == NULL)
		return;

	if (ug_category_gtk_move_selected_down (app->cwidget.current.category,
			app->cwidget.current.widget))
	{
		gtk_widget_set_sensitive (app->toolbar.move_up, TRUE);
		gtk_widget_set_sensitive (app->toolbar.move_top, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_up, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_top, TRUE);
	}
	else {
		gtk_widget_set_sensitive (app->toolbar.move_down, FALSE);
		gtk_widget_set_sensitive (app->toolbar.move_bottom, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.move_down, FALSE);
		gtk_widget_set_sensitive (app->menubar.download.move_bottom, FALSE);
	}
}

static void	on_move_download_to_top (GtkWidget* widget, UgAppGtk* app)
{
	if (app->cwidget.current.category == NULL)
		return;

	if (ug_category_gtk_move_selected_to_top (app->cwidget.current.category,
			app->cwidget.current.widget))
	{
		gtk_widget_set_sensitive (app->toolbar.move_down, TRUE);
		gtk_widget_set_sensitive (app->toolbar.move_bottom, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_down, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_bottom, TRUE);
	}
	gtk_widget_set_sensitive (app->toolbar.move_up, FALSE);
	gtk_widget_set_sensitive (app->toolbar.move_top, FALSE);
	gtk_widget_set_sensitive (app->menubar.download.move_up, FALSE);
	gtk_widget_set_sensitive (app->menubar.download.move_top, FALSE);
}

static void	on_move_download_to_bottom (GtkWidget* widget, UgAppGtk* app)
{
	if (app->cwidget.current.category == NULL)
		return;

	if (ug_category_gtk_move_selected_to_bottom (app->cwidget.current.category,
			app->cwidget.current.widget))
	{
		gtk_widget_set_sensitive (app->toolbar.move_up, TRUE);
		gtk_widget_set_sensitive (app->toolbar.move_top, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_up, TRUE);
		gtk_widget_set_sensitive (app->menubar.download.move_top, TRUE);
	}
	gtk_widget_set_sensitive (app->toolbar.move_down, FALSE);
	gtk_widget_set_sensitive (app->toolbar.move_bottom, FALSE);
	gtk_widget_set_sensitive (app->menubar.download.move_down, FALSE);
	gtk_widget_set_sensitive (app->menubar.download.move_bottom, FALSE);
}

// ----------------------------------------------------------------------------
// UgFileMenu
//
static GtkWidget*	create_file_chooser (const gchar* title, GtkWindow* parent, const gchar* filter_name, const gchar* mine_type)
{
	GtkWidget*		dialog;
	GtkFileFilter*	filter;

	dialog = gtk_file_chooser_dialog_new (title, parent,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK,     GTK_RESPONSE_OK,
			NULL);
	gtk_window_set_destroy_with_parent ((GtkWindow*) dialog, TRUE);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, filter_name);
	gtk_file_filter_add_mime_type (filter, mine_type);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	return dialog;
}

static void	on_import_html_file_response (GtkWidget* dialog, gint response, UgAppGtk* app)
{
	UgHtmlContext*	context;
	UgHtmlFilter*	filter_a;
	UgHtmlFilter*	filter_img;
	UgDownloadDialog*	ddialog;
	UgSelectorPage*		page;
	gchar*	string;
	gchar*	file;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// read URLs from html file
	string = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	file = g_filename_to_utf8 (string, -1, NULL, NULL, NULL);
	g_free (string);
	string = NULL;
	// parse html
	context	= ug_html_context_new ();
	filter_a = ug_html_filter_new ("A", "HREF");		// <A HREF="Link">
	ug_html_context_add_filter (context, filter_a);
	filter_img = ug_html_filter_new ("IMG", "SRC");		// <IMG SRC="Link">
	ug_html_context_add_filter (context, filter_img);
	ug_html_context_parse_file (context, file);
	g_free (file);
	if (context->base_href)
		string = g_strdup (context->base_href);
	ug_html_context_free (context);
	// UgDownloadDialog
	ddialog = ug_download_dialog_new (
			gtk_window_get_title ((GtkWindow*) dialog), app->window.self);
	ug_download_form_set_folder_list (&ddialog->download,
			app->setting.folder_list);
	ug_download_dialog_set_category (ddialog, &app->cwidget);
	ug_download_dialog_use_selector (ddialog);
	// set <base href>
	if (string) {
		gtk_entry_set_text (ddialog->selector.href_entry, string);
		g_free (string);
	}
	// add link
	page = ug_selector_add_page (&ddialog->selector, _("Link <A>"));
	ug_selector_page_add_uris (page, filter_a->attr_values);
	filter_a->attr_values = NULL;
	ug_html_filter_unref (filter_a);
	// add image
	page = ug_selector_add_page (&ddialog->selector, _("Image <IMG>"));
	ug_selector_page_add_uris (page, filter_img->attr_values);
	filter_img->attr_values = NULL;
	ug_html_filter_unref (filter_img);
	// setup & show download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	on_import_html_file (GtkWidget* widget, UgAppGtk* app)
{
	GtkWidget*		dialog;
	gchar*			title;

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Import URLs from HTML file"), NULL);
	dialog = create_file_chooser (title, app->window.self,
			"HTML file (*.htm, *.html)", "text/html");
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_import_html_file_response), app);
	gtk_widget_show (dialog);
}

static void	on_import_text_file_response (GtkWidget* dialog, gint response, UgAppGtk* app)
{
	UgDownloadDialog*	ddialog;
	UgSelectorPage*		page;
	gchar*		string;
	gchar*		file;
	GList*		list;
	GError*		error = NULL;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// read URLs from text file
	string = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	file = g_filename_to_utf8 (string, -1, NULL, NULL, NULL);
	g_free (string);
	list = ug_text_file_get_uris (file, &error);
	g_free (file);
	if (error) {
		ug_app_show_message (app, GTK_MESSAGE_ERROR, error->message);
		g_error_free (error);
		return;
	}
	// UgDownloadDialog
	ddialog = ug_download_dialog_new (
			gtk_window_get_title ((GtkWindow*) dialog), app->window.self);
	ug_download_form_set_folder_list (&ddialog->download,
			app->setting.folder_list);
	ug_download_dialog_set_category (ddialog, &app->cwidget);
	ug_download_dialog_use_selector (ddialog);
	page = ug_selector_add_page (&ddialog->selector, _("Text File"));
	ug_selector_hide_href (&ddialog->selector);
	ug_selector_page_add_uris (page, list);
	g_list_free (list);
	// setup & show download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	on_import_text_file (GtkWidget* widget, UgAppGtk* app)
{
	GtkWidget*		dialog;
	gchar*			title;

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Import URLs from text file"), NULL);
	dialog = create_file_chooser (title, app->window.self,
			"Plain text file", "text/plain");
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_import_text_file_response), app);
	gtk_widget_show (dialog);
}

static void	on_export_text_file_response (GtkWidget* dialog, gint response, UgAppGtk* app)
{
	GIOChannel*		channel;
	gchar*			string;
	GList*			list;
	GList*			link;

	if (response != GTK_RESPONSE_OK ) {
		gtk_widget_destroy (dialog);
		return;
	}
	// write all URLs to text file
	string = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	gtk_widget_destroy (dialog);
	channel = g_io_channel_new_file (string, "w", NULL);
	g_free (string);
	list = ug_category_gtk_get_all (app->cwidget.primary.category);
	for (link = list;  link;  link = link->next) {
		string = UG_DATASET_COMMON ((UgDataset*) link->data)->url;
		if (string) {
			g_io_channel_write_chars (channel, string, -1, NULL, NULL);
#ifdef _WIN32
			g_io_channel_write_chars (channel, "\r\n", 2, NULL, NULL);
#else
			g_io_channel_write_chars (channel, "\n", 1, NULL, NULL);
#endif
		}
	}
	g_list_free (list);
	g_io_channel_unref (channel);
}

static void	on_export_text_file (GtkWidget* widget, UgAppGtk* app)
{
	GtkWidget*		dialog;
	gchar*			title;

	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Export to"), NULL);
	dialog = gtk_file_chooser_dialog_new (_("Export to"), app->window.self,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK,     GTK_RESPONSE_OK,
			NULL);
	g_free (title);
	gtk_window_set_destroy_with_parent ((GtkWindow*) dialog, TRUE);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_export_text_file_response), app);
	gtk_widget_show (dialog);
}

static void	on_offline_mode (GtkWidget* widget, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	GtkCheckMenuItem*	item;

	item = GTK_CHECK_MENU_ITEM (widget);
	app->setting.offline_mode = gtk_check_menu_item_get_active (item);
	item = GTK_CHECK_MENU_ITEM (app->menubar.file.offline_mode);
	gtk_check_menu_item_set_active (item, app->setting.offline_mode);
	item = GTK_CHECK_MENU_ITEM (app->trayicon.menu.offline_mode);
	gtk_check_menu_item_set_active (item, app->setting.offline_mode);

	// into offline mode
	if (app->setting.offline_mode == TRUE) {
		// set action status "stop by user"
		app->action.stop = TRUE;
		// stop all active tasks
		ug_running_clear (&app->running);
		// refresh
		gtk_widget_queue_draw (app->cwidget.self);
		dwidget = app->cwidget.current.widget;
		gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
//		ug_summary_show (&app->summary, ug_download_widget_get_cursor (dwidget));
	}
}

// ----------------------------------------------------------------------------
// UgEditMenu
//
static void	on_monitor_clipboard (GtkWidget* widget, UgAppGtk* app)
{
	gboolean	active;

	active = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	app->setting.clipboard.monitor = active;
}

static void	on_config_settings_response (GtkDialog *dialog, gint response, UgSettingDialog* sdialog)
{
	UgAppGtk*	app;

	app = sdialog->user_data;
	app->dialogs.setting = NULL;
	if (response == GTK_RESPONSE_OK) {
		ug_setting_dialog_get (sdialog, &app->setting);
		// clipboard
		ug_clipboard_set_pattern (&app->clipboard,
				app->setting.clipboard.pattern);
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*) app->menubar.edit.clipboard_monitor,
				app->setting.clipboard.monitor);
		// launch
		g_regex_unref (app->launch_regex);
		app->launch_regex = g_regex_new (app->setting.launch.types,
				G_REGEX_CASELESS, 0, NULL);
		// ui
		ug_app_trayicon_decide_visible (app);
		// aria2
		ug_app_decide_bt_meta_sensitive (app);
		ug_app_aria2_setup (app);
		app->aria2.remote_updated = FALSE;
	}
	ug_setting_dialog_free (sdialog);
	// refresh
//	gtk_check_menu_item_toggled ((GtkCheckMenuItem*) app->menubar.edit.clipboard_monitor);
	on_monitor_clipboard (app->menubar.edit.clipboard_monitor, app);
}

static void	on_config_shutdown (GtkWidget* widget, UgAppGtk* app)
{
	gboolean	active;

	active = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	app->setting.shutdown = active;
}

static void	on_config_settings (GtkWidget* widget, UgAppGtk* app)
{
	UgSettingDialog*	sdialog;
	gchar*				title;

	if (app->dialogs.setting) {
		gtk_window_present ((GtkWindow*) app->dialogs.setting);
		return;
	}
	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Settings"), NULL);
	sdialog = ug_setting_dialog_new (title, app->window.self);
	g_free (title);
	ug_setting_dialog_set (sdialog, &app->setting);
	app->dialogs.setting = (GtkWidget*) sdialog->self;
	// set page
	if (widget == app->menubar.edit.clipboard_option)
		gtk_notebook_set_current_page (sdialog->notebook, UG_SETTING_PAGE_CLIPBOARD);
	else
		gtk_notebook_set_current_page (sdialog->notebook, UG_SETTING_PAGE_UI);
	// show settings dialog
	sdialog->user_data = app;
	g_signal_connect (sdialog->self, "response",
			G_CALLBACK (on_config_settings_response), sdialog);
	gtk_widget_show ((GtkWidget*) sdialog->self);
}

static void	on_config_clipboard (GtkWidget* widget, UgAppGtk* app)
{
	on_config_settings (widget, app);
}

// ----------------------------------------------------------------------------
// UgViewMenu
//
void	on_change_visible_widget (GtkWidget* widget, UgAppGtk* app)
{
	struct UgWindowSetting*	setting;
	gboolean				visible;

	setting = &app->setting.window;
	visible = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// Toolbar
	if (widget == app->menubar.view.toolbar) {
		setting->toolbar = visible;
		if (visible)
			gtk_widget_show (app->toolbar.self);
		else
			gtk_widget_hide (app->toolbar.self);
		return;
	}
	// Statusbar
	if (widget == app->menubar.view.statusbar) {
		setting->statusbar = visible;
		if (visible)
			gtk_widget_show ((GtkWidget*) app->statusbar.self);
		else
			gtk_widget_hide ((GtkWidget*) app->statusbar.self);
		return;
	}
	// Category
	if (widget == app->menubar.view.category) {
		setting->category = visible;
		if (visible)
			gtk_widget_show (app->cwidget.self);
		else
			gtk_widget_hide (app->cwidget.self);
		return;
	}
	// Summary
	if (widget == app->menubar.view.summary) {
		setting->summary = visible;
		if (visible)
			gtk_widget_show (app->summary.self);
		else
			gtk_widget_hide (app->summary.self);
		return;
	}
}

void	on_change_visible_summary (GtkWidget* widget, UgAppGtk* app)
{
	struct UgSummarySetting*	setting;
	gboolean					visible;

	setting = &app->setting.summary;
	visible = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// which widget
	if (widget == app->menubar.view.summary_items.name) {
		setting->name = visible;
		app->summary.visible.name = visible;
	}
	else if (widget == app->menubar.view.summary_items.folder) {
		setting->folder = visible;
		app->summary.visible.folder = visible;
	}
	else if (widget == app->menubar.view.summary_items.category) {
		setting->category = visible;
		app->summary.visible.category = visible;
	}
	else if (widget == app->menubar.view.summary_items.url) {
		setting->url = visible;
		app->summary.visible.url = visible;
	}
	else if (widget == app->menubar.view.summary_items.message) {
		setting->message = visible;
		app->summary.visible.message = visible;
	}

	ug_summary_show (&app->summary,
			ug_download_widget_get_cursor (app->cwidget.current.widget));
}

void	on_change_visible_column (GtkWidget* widget, UgAppGtk* app)
{
	struct UgDownloadColumnSetting*	setting;
	UgDownloadWidget*	dwidget;
	GtkTreeViewColumn*	column;
	gboolean			visible;
	gint				column_index;

	setting = &app->setting.download_column;
	dwidget = app->cwidget.current.widget;
	visible = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// which widget
	if (widget == app->menubar.view.columns.completed) {
		column_index = UG_DOWNLOAD_COLUMN_COMPLETE;
		setting->completed = visible;
	}
	else if (widget == app->menubar.view.columns.total) {
		column_index = UG_DOWNLOAD_COLUMN_SIZE;
		setting->total = visible;
	}
	else if (widget == app->menubar.view.columns.percent) {
		column_index = UG_DOWNLOAD_COLUMN_PERCENT;
		setting->percent = visible;
	}
	else if (widget == app->menubar.view.columns.elapsed) {
		column_index = UG_DOWNLOAD_COLUMN_ELAPSED;
		setting->elapsed = visible;
	}
	else if (widget == app->menubar.view.columns.left) {
		column_index = UG_DOWNLOAD_COLUMN_LEFT;
		setting->left = visible;
	}
	else if (widget == app->menubar.view.columns.speed) {
		column_index = UG_DOWNLOAD_COLUMN_SPEED;
		setting->speed = visible;
	}
	else if (widget == app->menubar.view.columns.upload_speed) {
		column_index = UG_DOWNLOAD_COLUMN_UPLOAD_SPEED;
		setting->upload_speed = visible;
	}
	else if (widget == app->menubar.view.columns.uploaded) {
		column_index = UG_DOWNLOAD_COLUMN_UPLOADED;
		setting->uploaded = visible;
	}
	else if (widget == app->menubar.view.columns.ratio) {
		column_index = UG_DOWNLOAD_COLUMN_RATIO;
		setting->ratio = visible;
	}
	else if (widget == app->menubar.view.columns.retry) {
		column_index = UG_DOWNLOAD_COLUMN_RETRY;
		setting->retry = visible;
	}
	else if (widget == app->menubar.view.columns.category) {
		column_index = UG_DOWNLOAD_COLUMN_CATEGORY;
		setting->category = visible;
	}
	else if (widget == app->menubar.view.columns.url) {
		column_index = UG_DOWNLOAD_COLUMN_URL;
		setting->url = visible;
	}
	else if (widget == app->menubar.view.columns.added_on) {
		column_index = UG_DOWNLOAD_COLUMN_ADDED_ON;
		setting->added_on = visible;
	}
	else if (widget == app->menubar.view.columns.completed_on) {
		column_index = UG_DOWNLOAD_COLUMN_COMPLETED_ON;
		setting->completed_on = visible;
	}
	else
		return;

	column = gtk_tree_view_get_column (dwidget->view, column_index);
	gtk_tree_view_column_set_visible (column, visible);
	// sync changed_count
	setting->changed_count++;
	dwidget->changed_count = setting->changed_count;
}

// ----------------------------------------------------------------------------
// UgHelpMenu
//
void	on_about (GtkWidget* widget, UgAppGtk* app)
{
//	if (gtk_widget_get_visible ((GtkWidget*) app->window.self) == FALSE) {
//		gtk_window_deiconify (app->window.self);
//		gtk_widget_show ((GtkWidget*) app->window.self);
//	}

	gtk_show_about_dialog (app->window.self,
			"logo-icon-name", UG_APP_GTK_ICON_NAME,
			"program-name", UG_APP_GTK_NAME,
			"version", uget_version,
			"comments", gettext (uget_comments),
			"copyright", uget_copyright,
#ifdef _WIN32
			"website-label", UGET_URL_WEBSITE,
#else
			"website", UGET_URL_WEBSITE,
#endif
			"license", uget_license,
			"authors", uget_authors,
			"artists", uget_artists,
			"translator-credits", gettext (translator_credits),
			NULL);
}

// ----------------------------------------------------------------------------
// UgTrayIcon
//
static void	on_trayicon_activate (GtkStatusIcon* status_icon, UgAppGtk* app)
{
	if (gtk_widget_get_visible ((GtkWidget*) app->window.self) == TRUE) {
		// get position and size
		gtk_window_get_position (app->window.self,
				&app->setting.window.x, &app->setting.window.y);
		gtk_window_get_size (app->window.self,
				&app->setting.window.width, &app->setting.window.height);
		// hide window
		gtk_window_iconify (app->window.self);
		gtk_widget_hide ((GtkWidget*) app->window.self);
	}
	else {
		gtk_widget_show ((GtkWidget*) app->window.self);
		gtk_window_deiconify (app->window.self);
		gtk_window_present (app->window.self);
		ug_app_trayicon_decide_visible (app);
	}
	// clear error status
	if (app->trayicon.error_occurred) {
		app->trayicon.error_occurred = FALSE;
		gtk_status_icon_set_from_icon_name (status_icon, UG_APP_GTK_ICON_NAME);
	}
}

static void	on_trayicon_popup_menu (GtkStatusIcon* status_icon, guint button, guint activate_time, UgAppGtk* app)
{
	gtk_menu_set_screen ((GtkMenu*) app->trayicon.menu.self,
			gtk_status_icon_get_screen (status_icon));
#ifdef _WIN32
	gtk_menu_popup ((GtkMenu*) app->trayicon.menu.self,
			NULL, NULL,
			NULL, NULL,
			button, activate_time);
#else
	gtk_menu_popup ((GtkMenu*) app->trayicon.menu.self,
			NULL, NULL,
			gtk_status_icon_position_menu, status_icon,
			button, activate_time);
#endif
}

static void	on_trayicon_show_window (GtkWidget* widget, UgAppGtk* app)
{
	if (gtk_widget_get_visible ((GtkWidget*) app->window.self))
		gtk_window_present (app->window.self);
	else {
		gtk_widget_show ((GtkWidget*) app->window.self);
		gtk_window_deiconify (app->window.self);
		gtk_window_present (app->window.self);
		ug_app_trayicon_decide_visible (app);
	}
}

// ----------------------------------------------------------------------------
// UgWindow

// button-press-event
static gboolean	on_button_press_event (GtkTreeView* treeview, GdkEventButton* event, UgAppGtk* app)
{
	GtkTreeSelection*	selection;
	GtkTreePath*		path;
	GtkMenu*			menu;
	gboolean			is_selected;

	// right button press
//	if (event->type != GDK_BUTTON_PRESS)
//		return FALSE;
	if (event->button != 3)		// right mouse button
		return FALSE;
	// popup a menu
	if (treeview == app->cwidget.view || treeview == app->cwidget.primary.view)
		menu = (GtkMenu*) app->menubar.category.self;
	else if (treeview == app->summary.view)
		menu = app->summary.menu.self;
	else
		menu = (GtkMenu*) app->menubar.download.self;
	gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
			event->button, gtk_get_current_event_time());

	if (gtk_tree_view_get_path_at_pos (treeview, (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL)) {
		selection = gtk_tree_view_get_selection (treeview);
		is_selected = gtk_tree_selection_path_is_selected (selection, path);
		gtk_tree_path_free (path);
		if (is_selected)
			return TRUE;
	}
	return FALSE;
}

// This function is used by on_window_key_press_event()
static void menu_position_func (GtkMenu*	menu,
                                gint*		x,
                                gint*		y,
                                gboolean*	push_in,
                                gpointer	user_data)
{
	GtkRequisition	menu_requisition;
	GtkAllocation	allocation;
	GtkWidget*		widget;
	gint			max_x, max_y;

	widget = user_data;
	gdk_window_get_origin (gtk_widget_get_window (widget), x, y);
	gtk_widget_get_preferred_size (GTK_WIDGET (menu), &menu_requisition, NULL);

	gtk_widget_get_allocation (widget, &allocation);
	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
		*x += allocation.width - allocation.width / 3;
	else
		*x += allocation.width / 3;
	*y += allocation.height / 3;

//	if (gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
//		*x += allocation.width - menu_requisition.width;
//	else
//		*x += allocation.x;
//	*y += allocation.y + allocation.height;

	// Make sure we are on the screen.
	max_x = MAX (0, gdk_screen_width ()  - menu_requisition.width);
	max_y = MAX (0, gdk_screen_height () - menu_requisition.height);

	*x = CLAMP (*x, 0, max_x);
	*y = CLAMP (*y, 0, max_y);
}

// key-press-event
static gboolean	on_window_key_press_event (GtkWidget *widget, GdkEventKey *event, UgAppGtk* app)
{
	GtkTreeView*	focus;
	GtkMenu*		menu;

//	g_print ("key-press : 0x%x\n", event->keyval);
	if (event->keyval != GDK_KEY_Menu)
		return FALSE;

	focus = (GtkTreeView*) gtk_window_get_focus (app->window.self);
	if (focus == app->cwidget.primary.view) {
		widget = (GtkWidget*) app->cwidget.primary.view;
		menu = (GtkMenu*) app->menubar.category.self;
	}
	else if (focus == app->cwidget.view) {
		widget = (GtkWidget*) app->cwidget.view;
		menu = (GtkMenu*) app->menubar.category.self;
	}
	else if (focus == app->summary.view) {
		widget = (GtkWidget*) app->summary.view;
		menu = (GtkMenu*) app->summary.menu.self;
	}
	else if (focus == app->cwidget.current.widget->view) {
		widget = (GtkWidget*) app->cwidget.current.widget->view;
		menu = (GtkMenu*) app->menubar.download.self;
	}
	else
		return FALSE;

	gtk_menu_popup (menu, NULL, NULL,
			menu_position_func, widget,
			0, gtk_get_current_event_time());
	return TRUE;
}

// UgWindow.self "delete-event"
static gboolean	on_window_delete_event (GtkWidget* widget, GdkEvent* event, UgAppGtk* app)
{
	if (app->setting.ui.close_confirmation == FALSE) {
		ug_app_window_close (app);
		return TRUE;
	}
	ug_app_confirm_to_quit (app);
	return TRUE;
}

// UgDownloadWidget.view "key-press-event"
static gboolean	on_download_key_press_event  (GtkWidget* widget, GdkEventKey* event, UgAppGtk* app)
{
	if (event->keyval == GDK_KEY_Delete  &&  app->cwidget.current.category) {
		on_delete_download (widget, app);
		return TRUE;
	}
/*
	// check shift key status
	GdkWindow*		gdk_win;
	GdkDevice*		dev_pointer;
	GdkModifierType	mask;

	// check shift key status
	gdk_win = gtk_widget_get_parent_window ((GtkWidget*) app->cwidget.current.widget->view);
	dev_pointer = gdk_device_manager_get_client_pointer (
			gdk_display_get_device_manager (gdk_window_get_display (gdk_win)));
	gdk_window_get_device_position (gdk_win, dev_pointer, NULL, NULL, &mask);

	if (app->cwidget.current.category) {
		switch (event->keyval) {
		case GDK_KEY_Delete:
		case GDK_KEY_KP_Delete:
			if (mask & GDK_SHIFT_MASK)
				on_delete_download_file (widget, app);
			else
				on_delete_download (widget, app);
			return TRUE;

		case GDK_KEY_Return:
		case GDK_KEY_KP_Enter:
			if (mask & GDK_SHIFT_MASK)
				on_open_download_folder (widget, app);
			else
				on_open_download_file (widget, app);
			return TRUE;
		}
	}
*/
	return FALSE;
}

// UgDownloadWidget.view selection "changed"
static void	on_download_selection_changed (GtkTreeSelection* selection, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;

	dwidget = app->cwidget.current.widget;
	ug_statusbar_set_info (&app->statusbar, dwidget);
	ug_app_decide_download_sensitive (app);
}

// UgDownloadWidget.view "cursor-changed"
static void	on_download_cursor_changed (GtkTreeView* view, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgDataset*			dataset;

//	Add "action.deleted" to avoid crash in GTK+ 3.4. This maybe GTK+ bug.
//	1. user delete an iter in GtkTreeModel.
//	2. emit "cursor-changed" signal because cursor changed after deleting.
//	3. call gtk_tree_view_get_cursor(view, &path)
//	4. call gtk_tree_model_get_iter(model, &iter, path) and result TRUE.
//	5. call gtk_tree_model_get(model, &iter, ...) may result invalid iter
//	   and crash...
	if (app->action.deleted == FALSE) {
		dwidget = app->cwidget.current.widget;
		dataset = ug_download_widget_get_cursor (dwidget);
		ug_summary_show (&app->summary, dataset);
	}
}

// UgCategoryWidget.view and primary_view "cursor-changed"
static void	on_category_cursor_changed (GtkTreeView* view, UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	GtkTreeSelection*	selection;
	GList*				list;

	// ----------------------------------------------------
	// switch UgDownloadWidget in right side
	dwidget = app->cwidget.current.widget;
	list = gtk_container_get_children (GTK_CONTAINER (app->window.vpaned));
	list = g_list_remove (list, app->summary.self);
	if (list) {
		g_object_ref (list->data);
		gtk_container_remove (GTK_CONTAINER (app->window.vpaned), list->data);
		g_list_free (list);
	}
	gtk_paned_pack1 (app->window.vpaned, dwidget->self, TRUE, TRUE);

	// connect signals
	if (dwidget) {
		if (dwidget->signal_connected == FALSE) {
			dwidget->signal_connected =  TRUE;
			selection = gtk_tree_view_get_selection (dwidget->view);
			g_signal_connect (selection, "changed",
					G_CALLBACK (on_download_selection_changed), app);
			g_signal_connect (dwidget->view, "cursor-changed",
					G_CALLBACK (on_download_cursor_changed), app);
			g_signal_connect (dwidget->view, "key-press-event",
					G_CALLBACK (on_download_key_press_event), app);
			g_signal_connect (dwidget->view, "button-press-event",
					G_CALLBACK (on_button_press_event), app);
		}
		// refresh summary
		ug_summary_show (&app->summary,
				ug_download_widget_get_cursor (dwidget));
	}

	// refresh
	ug_app_menubar_sync_category (app, FALSE);
	ug_statusbar_set_info (&app->statusbar, dwidget);
	ug_app_reset_download_column (app);
	ug_app_decide_category_sensitive (app);
}

// UgSummary.menu.copy signal handler
static void	on_summary_copy_selected (GtkWidget* widget, UgAppGtk* app)
{
	gchar*	text;

	text = ug_summary_get_text_selected (&app->summary);
	ug_clipboard_set_text (&app->clipboard, text);
}

// UgSummary.menu.copy_all signal handler
static void	on_summary_copy_all (GtkWidget* widget, UgAppGtk* app)
{
	gchar*	text;

	text = ug_summary_get_text_all (&app->summary);
	ug_clipboard_set_text (&app->clipboard, text);
}

// ----------------------------------------------------------------------------
// used by ug_app_init_callback()

// UgWindow
static void ug_window_init_callback (struct UgWindow* window, UgAppGtk* app)
{
	// UgCategoryWidget
	g_signal_connect_after (app->cwidget.primary.view, "cursor-changed",
			G_CALLBACK (on_category_cursor_changed), app);
	g_signal_connect_after (app->cwidget.view, "cursor-changed",
			G_CALLBACK (on_category_cursor_changed), app);

	// pop-up menu by mouse button
//	g_signal_connect (app->cwidget.primary.view, "button-press-event",
//			G_CALLBACK (on_button_press_event), app);
	g_signal_connect (app->cwidget.view, "button-press-event",
			G_CALLBACK (on_button_press_event), app);
	g_signal_connect (app->summary.view, "button-press-event",
			G_CALLBACK (on_button_press_event), app);

	// UgSummary.menu signal handlers
	g_signal_connect (app->summary.menu.copy, "activate",
			G_CALLBACK (on_summary_copy_selected), app);
	g_signal_connect (app->summary.menu.copy_all, "activate",
			G_CALLBACK (on_summary_copy_all), app);

	// UgWindow.self signal handlers
	g_signal_connect (window->self, "key-press-event",
			G_CALLBACK (on_window_key_press_event), app);
	g_signal_connect (window->self, "delete-event",
			G_CALLBACK (on_window_delete_event), app);
	g_signal_connect_swapped (window->self, "destroy",
			G_CALLBACK (ug_app_quit), app);
}

#ifdef _WIN32
static gboolean	tray_menu_timeout (GtkMenu* menu)
{
	gtk_menu_popdown (menu);
	// return FALSE if the source should be removed.
	return FALSE;
}

static gboolean	tray_menu_leave_enter (GtkWidget* menu, GdkEventCrossing* event, gpointer data)
{
	static guint	tray_menu_timer = 0;

	if (event->type == GDK_LEAVE_NOTIFY &&
		(event->detail == GDK_NOTIFY_ANCESTOR || event->detail == GDK_NOTIFY_UNKNOWN))
	{
		if (tray_menu_timer == 0) {
			tray_menu_timer = g_timeout_add (500,
					(GSourceFunc) tray_menu_timeout, menu);
		}
	}
	else if (event->type == GDK_ENTER_NOTIFY && event->detail == GDK_NOTIFY_ANCESTOR)
	{
		if (tray_menu_timer != 0) {
			g_source_remove (tray_menu_timer);
			tray_menu_timer = 0;
		}
	}
	return FALSE;
}
#endif	// _WIN32

// UgTrayIcon
static void ug_trayicon_init_callback (struct UgTrayIcon* icon, UgAppGtk* app)
{
	g_signal_connect (icon->self, "activate",
			G_CALLBACK (on_trayicon_activate), app);
	g_signal_connect (icon->self, "popup-menu",
			G_CALLBACK (on_trayicon_popup_menu), app);

#ifdef _WIN32
	g_signal_connect (icon->menu.self, "leave-notify-event",
			G_CALLBACK (tray_menu_leave_enter), NULL);
	g_signal_connect (icon->menu.self, "enter-notify-event",
			G_CALLBACK (tray_menu_leave_enter), NULL);
#endif

	g_signal_connect (icon->menu.create_download, "activate",
			G_CALLBACK (on_create_download), app);
	g_signal_connect (icon->menu.create_clipboard, "activate",
			G_CALLBACK (on_create_from_clipboard), app);
	g_signal_connect (icon->menu.create_torrent, "activate",
			G_CALLBACK (on_create_torrent), app);
	g_signal_connect (icon->menu.create_metalink, "activate",
			G_CALLBACK (on_create_metalink), app);
	g_signal_connect (icon->menu.settings, "activate",
			G_CALLBACK (on_config_settings), app);
	g_signal_connect (icon->menu.show_window, "activate",
			G_CALLBACK (on_trayicon_show_window), app);
	g_signal_connect (icon->menu.offline_mode, "toggled",
			G_CALLBACK (on_offline_mode), app);
	g_signal_connect (icon->menu.about, "activate",
			G_CALLBACK (on_about), app);
	g_signal_connect_swapped (icon->menu.quit, "activate",
			G_CALLBACK (ug_app_quit), app);
}

// UgToolbar
static void ug_toolbar_init_callback (struct UgToolbar* toolbar, UgAppGtk* app)
{
	// create new
	g_signal_connect (toolbar->create, "clicked",
			G_CALLBACK (on_create_download), app);
	g_signal_connect (toolbar->create_download, "activate",
			G_CALLBACK (on_create_download), app);
	g_signal_connect (toolbar->create_category, "activate",
			G_CALLBACK (on_create_category), app);
	g_signal_connect (toolbar->create_sequence, "activate",
			G_CALLBACK (on_create_sequence), app);
	g_signal_connect (toolbar->create_clipboard, "activate",
			G_CALLBACK (on_create_from_clipboard), app);
	g_signal_connect (toolbar->create_torrent, "activate",
			G_CALLBACK (on_create_torrent), app);
	g_signal_connect (toolbar->create_metalink, "activate",
			G_CALLBACK (on_create_metalink), app);
	// save
	g_signal_connect_swapped (toolbar->save, "clicked",
			G_CALLBACK (ug_app_save), app);
	// change status
	g_signal_connect (toolbar->runnable, "clicked",
			G_CALLBACK (on_set_download_runnable), app);
	g_signal_connect (toolbar->pause, "clicked",
			G_CALLBACK (on_set_download_to_pause), app);
	// change data
	g_signal_connect (toolbar->properties, "clicked",
			G_CALLBACK (on_config_download), app);
	// move
	g_signal_connect (toolbar->move_up, "clicked",
			G_CALLBACK (on_move_download_up), app);
	g_signal_connect (toolbar->move_down, "clicked",
			G_CALLBACK (on_move_download_down), app);
	g_signal_connect (toolbar->move_top, "clicked",
			G_CALLBACK (on_move_download_to_top), app);
	g_signal_connect (toolbar->move_bottom, "clicked",
			G_CALLBACK (on_move_download_to_bottom), app);
}

// UgMenubar
static void ug_menubar_init_callback (struct UgMenubar* menubar, UgAppGtk* app)
{
	// ----------------------------------------------------
	// UgFileMenu
	g_signal_connect (menubar->file.create.download, "activate",
			G_CALLBACK (on_create_download), app);
	g_signal_connect (menubar->file.create.category, "activate",
			G_CALLBACK (on_create_category), app);
	g_signal_connect (menubar->file.create.torrent, "activate",
			G_CALLBACK (on_create_torrent), app);
	g_signal_connect (menubar->file.create.metalink, "activate",
			G_CALLBACK (on_create_metalink), app);
	g_signal_connect (menubar->file.batch.clipboard, "activate",
			G_CALLBACK (on_create_from_clipboard), app);
	g_signal_connect (menubar->file.batch.sequence, "activate",
			G_CALLBACK (on_create_sequence), app);
	g_signal_connect (menubar->file.batch.text_import, "activate",
			G_CALLBACK (on_import_text_file), app);
	g_signal_connect (menubar->file.batch.html_import, "activate",
			G_CALLBACK (on_import_html_file), app);
	g_signal_connect (menubar->file.batch.text_export, "activate",
			G_CALLBACK (on_export_text_file), app);
	g_signal_connect_swapped (menubar->file.save, "activate",
			G_CALLBACK (ug_app_save), app);
	g_signal_connect (menubar->file.offline_mode, "toggled",
			G_CALLBACK (on_offline_mode), app);
	g_signal_connect_swapped (menubar->file.quit, "activate",
			G_CALLBACK (ug_app_quit), app);

	// ----------------------------------------------------
	// UgEditMenu
	g_signal_connect (menubar->edit.clipboard_monitor, "activate",
			G_CALLBACK (on_monitor_clipboard), app);
	g_signal_connect (menubar->edit.clipboard_option, "activate",
			G_CALLBACK (on_config_clipboard), app);
	g_signal_connect (menubar->edit.shutdown, "activate",
			G_CALLBACK (on_config_shutdown), app);
	g_signal_connect (menubar->edit.settings, "activate",
			G_CALLBACK (on_config_settings), app);

	// ----------------------------------------------------
	// UgViewMenu
	g_signal_connect (menubar->view.toolbar, "toggled",
			G_CALLBACK (on_change_visible_widget), app);
	g_signal_connect (menubar->view.statusbar, "toggled",
			G_CALLBACK (on_change_visible_widget), app);
	g_signal_connect (menubar->view.category, "toggled",
			G_CALLBACK (on_change_visible_widget), app);
	g_signal_connect (menubar->view.summary, "toggled",
			G_CALLBACK (on_change_visible_widget), app);
	// summary items
	g_signal_connect (menubar->view.summary_items.name, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	g_signal_connect (menubar->view.summary_items.folder, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	g_signal_connect (menubar->view.summary_items.category, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	g_signal_connect (menubar->view.summary_items.url, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	g_signal_connect (menubar->view.summary_items.message, "toggled",
			G_CALLBACK (on_change_visible_summary), app);
	// download columns
	g_signal_connect (menubar->view.columns.completed, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.total, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.percent, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.elapsed, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.left, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.speed, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.upload_speed, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.uploaded, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.ratio, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.retry, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.category, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.url, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.added_on, "toggled",
			G_CALLBACK (on_change_visible_column), app);
	g_signal_connect (menubar->view.columns.completed_on, "toggled",
			G_CALLBACK (on_change_visible_column), app);

	// ----------------------------------------------------
	// UgCategoryMenu
	g_signal_connect (menubar->category.create, "activate",
			G_CALLBACK (on_create_category), app);
	g_signal_connect (menubar->category.delete, "activate",
			G_CALLBACK (on_delete_category), app);
	g_signal_connect (menubar->category.properties, "activate",
			G_CALLBACK (on_config_category), app);

	// ----------------------------------------------------
	// UgDownloadMenu
	g_signal_connect (menubar->download.create, "activate",
			G_CALLBACK (on_create_download), app);
	g_signal_connect (menubar->download.delete, "activate",
			G_CALLBACK (on_delete_download), app);
	// file & folder
	g_signal_connect (menubar->download.delete_file, "activate",
			G_CALLBACK (on_delete_download_file), app);
	g_signal_connect (menubar->download.open, "activate",
			G_CALLBACK (on_open_download_file), app);
	g_signal_connect (menubar->download.open_folder, "activate",
			G_CALLBACK (on_open_download_folder), app);
	// change status
	g_signal_connect (menubar->download.force_start, "activate",
			G_CALLBACK (on_set_download_force_start), app);
	g_signal_connect (menubar->download.runnable, "activate",
			G_CALLBACK (on_set_download_runnable), app);
	g_signal_connect (menubar->download.pause, "activate",
			G_CALLBACK (on_set_download_to_pause), app);
	// move
	g_signal_connect (menubar->download.move_up, "activate",
			G_CALLBACK (on_move_download_up), app);
	g_signal_connect (menubar->download.move_down, "activate",
			G_CALLBACK (on_move_download_down), app);
	g_signal_connect (menubar->download.move_top, "activate",
			G_CALLBACK (on_move_download_to_top), app);
	g_signal_connect (menubar->download.move_bottom, "activate",
			G_CALLBACK (on_move_download_to_bottom), app);
	// change data
	g_signal_connect (menubar->download.properties, "activate",
			G_CALLBACK (on_config_download), app);

	// ----------------------------------------------------
	// UgHelpMenu
	g_signal_connect (menubar->help.about_uget, "activate",
			G_CALLBACK (on_about), app);
}

