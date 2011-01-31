static const char	uget_license[] =
{
" Copyright (C) 2005-2011 by Raymond Huang"								"\n"
" plushuang at users.sourceforge.net"									"\n"
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
// for GTK+ 2.18
#ifndef GDK_KEY_Menu
#define GDK_KEY_Menu		GDK_Menu
#define GDK_KEY_Delete		GDK_Delete
#endif

// uglib
#include <UgHtml.h>
#include <UgUtils.h>
#include <UgData-download.h>
#include <UgDownloadDialog.h>
#include <UgCategoryDialog.h>
#include <UgSettingDialog.h>
#include <UgetGtk.h>

#include <glib/gi18n.h>

#define	UGET_URL_WEBSITE	"http://urlget.sourceforge.net/"

// static data
static const gchar*	uget_authors[] = { "Raymond Huang  (\xE9\xBB\x83\xE6\xAD\xA3\xE9\x9B\x84)", NULL };
static const gchar*	uget_artists[] =
{
	"Logo designer: Michael Tunnell (visuex.com)",
	"Former Logo designer: saf1 (linuxac.org)",
	"Former Logo improver: Skeleton_Eel (linuxac.org)",
	NULL
};
static const gchar*	uget_version   = UGET_GTK_VERSION;
static const gchar*	uget_comments  = N_("Download Manager");
static const gchar*	uget_copyright = "Copyright (C) 2005-2011 Raymond Huang";
static const gchar*	translator_credits = N_("translator-credits");

// static functions
static void	uget_gtk_window_init_callback    (struct UgetGtkWindow*  window,  UgetGtk* ugtk);
static void	uget_gtk_toolbar_init_callback   (struct UgetGtkToolbar* toolbar, UgetGtk* ugtk);
static void	uget_gtk_menubar_init_callback   (struct UgetGtkMenubar* menubar, UgetGtk* ugtk);
static void	uget_gtk_tray_icon_init_callback (struct UgetGtkTrayIcon* icon,   UgetGtk* ugtk);


void	uget_gtk_init_callback (UgetGtk* ugtk)
{
//	gtk_accel_group_connect (ugtk->accel_group, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (uget_gtk_quit), ugtk, NULL));
//	gtk_accel_group_connect (ugtk->accel_group, GDK_KEY_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (uget_gtk_save), ugtk, NULL));
//	gtk_accel_group_connect (ugtk->accel_group, GDK_KEY_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE,
//	                         g_cclosure_new_swap (G_CALLBACK (on_summary_copy_selected), ugtk, NULL));
	uget_gtk_window_init_callback  (&ugtk->window,  ugtk);
	uget_gtk_toolbar_init_callback (&ugtk->toolbar, ugtk);
	uget_gtk_menubar_init_callback (&ugtk->menubar, ugtk);
	uget_gtk_tray_icon_init_callback (&ugtk->tray_icon, ugtk);
}

// ----------------------------------------------------------------------------
// Category
//
static void	on_create_category_response (GtkDialog *dialog, gint response_id, UgCategoryDialog* cdialog)
{
	UgCategory*		category;
	UgetGtk*		ugtk;

	if (response_id == GTK_RESPONSE_OK) {
		ugtk = cdialog->user.app;
		ug_download_form_get_folder_list (&cdialog->download,
				&ugtk->setting.folder_list);
		category = ug_category_new_with_gtk (ugtk->cwidget.primary.category);
		ug_category_dialog_get (cdialog, category);
		ug_category_widget_append (&ugtk->cwidget, category);
		uget_gtk_move_menu_refresh (&ugtk->menubar, ugtk, TRUE);
	}
	ug_category_dialog_free (cdialog);
}

static void	on_create_category (GtkWidget* widget, UgetGtk* ugtk)
{
	UgCategoryDialog*	cdialog;
	gchar*				title;

	title = g_strconcat (UGET_GTK_NAME " - ", _("New Category"), NULL);
	cdialog = ug_category_dialog_new (title, ugtk->window.self);
	g_free (title);
	ug_category_dialog_set (cdialog, &ugtk->setting.category);
	ug_download_form_set_folder_list (&cdialog->download,
			ugtk->setting.folder_list);
	// show category dialog
	cdialog->user.app = ugtk;
	g_signal_connect (cdialog->self, "response",
			G_CALLBACK (on_create_category_response), cdialog);
	gtk_widget_show ((GtkWidget*) cdialog->self);
}

static void	on_delete_category (GtkWidget* widget, UgetGtk* ugtk)
{
	UgCategory*		current;
	GtkWidget*		download_view;

	if (ugtk->cwidget.current.category == ugtk->cwidget.primary.category)
		return;
	// remove current view before removing category
	download_view = ugtk->cwidget.current.widget->self;
	g_object_ref (download_view);
	gtk_container_remove (GTK_CONTAINER (ugtk->window.vpaned), download_view);
	// remove and delete category
	current = ugtk->cwidget.current.category;
	ug_category_widget_remove (&ugtk->cwidget, current);
	// refresh
	gtk_widget_queue_draw ((GtkWidget*) ugtk->cwidget.primary.view);
	uget_gtk_move_menu_refresh (&ugtk->menubar, ugtk, TRUE);
}

static void	on_config_category_response (GtkDialog *dialog, gint response_id, UgCategoryDialog* cdialog)
{
	UgetGtk*		ugtk;

	ugtk = cdialog->user.app;
	if (response_id == GTK_RESPONSE_OK) {
		ug_download_form_get_folder_list (&cdialog->download,
				&ugtk->setting.folder_list);
		ug_category_dialog_get (cdialog, ugtk->cwidget.current.category);
	}
	gtk_widget_set_sensitive ((GtkWidget*)ugtk->window.self, TRUE);
	ug_category_dialog_free (cdialog);
}

static void	on_config_category (GtkWidget* widget, UgetGtk* ugtk)
{
	UgCategoryDialog*	cdialog;
	gchar*				title;

	if (ugtk->cwidget.current.category == ugtk->cwidget.primary.category)
		return;
	title = g_strconcat (UGET_GTK_NAME " - ", _("Category Properties"), NULL);
	cdialog = ug_category_dialog_new (title, ugtk->window.self);
	g_free (title);
	ug_category_dialog_set (cdialog, ugtk->cwidget.current.category);
	ug_download_form_set_folder_list (&cdialog->download,
			ugtk->setting.folder_list);
	// show category dialog
	cdialog->user.app = ugtk;
	g_signal_connect (cdialog->self, "response",
			G_CALLBACK (on_config_category_response), cdialog);
	gtk_widget_set_sensitive ((GtkWidget*)ugtk->window.self, FALSE);
	gtk_widget_show ((GtkWidget*) cdialog->self);
}

static void	on_config_category_default_response (GtkDialog *dialog, gint response_id, UgCategoryDialog* cdialog)
{
	UgetGtk*		ugtk;

	ugtk = cdialog->user.app;
	if (response_id == GTK_RESPONSE_OK) {
		ug_download_form_get_folder_list (&cdialog->download,
				&ugtk->setting.folder_list);
		ug_category_dialog_get (cdialog, &ugtk->setting.category);
	}
	ug_category_dialog_free (cdialog);
}

static void	on_config_category_default (GtkWidget* widget, UgetGtk* ugtk)
{
	UgCategoryDialog*	cdialog;
	gchar*				title;

	title = g_strconcat (UGET_GTK_NAME " - ", _("Default for new Category"), NULL);
	cdialog = ug_category_dialog_new (title, ugtk->window.self);
	g_free (title);
	ug_category_dialog_set (cdialog, &ugtk->setting.category);
	ug_category_form_set_multiple (&cdialog->category, TRUE);
	ug_download_form_set_folder_list (&cdialog->download,
			ugtk->setting.folder_list);
	// show category dialog
	cdialog->user.app = ugtk;
	g_signal_connect (cdialog->self, "response",
			G_CALLBACK (on_config_category_default_response), cdialog);
	gtk_widget_show ((GtkWidget*) cdialog->self);
}

// ----------------------------------------------------------------------------
// Download
//
static void	on_create_download_response (GtkDialog *dialog, gint response_id, UgDownloadDialog* ddialog)
{
	UgCategory*		category;
	UgetGtk*		ugtk;
	GList*			list;
	GList*			link;

	if (response_id == GTK_RESPONSE_OK) {
		ugtk = ddialog->user.app;
		ug_download_form_get_folder_list (&ddialog->download,
				&ugtk->setting.folder_list);
		category = ug_download_dialog_get_category (ddialog);
		if (category) {
			list = ug_download_dialog_get_downloads (ddialog);
			for (link = list;  link;  link = link->next)
				ug_category_gtk_add (category, link->data);
			g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
			g_list_free (list);
			gtk_widget_queue_draw ((GtkWidget*) ugtk->cwidget.self);
		}
	}
	ug_download_dialog_free (ddialog);
}

static void	on_create_download (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadDialog*	ddialog;
	gchar*				title;
	GList*				list;

	title = g_strconcat (UGET_GTK_NAME " - ", _("New Download"), NULL);
	ddialog = ug_download_dialog_new (title, ugtk->window.self);
	g_free (title);
	if (gtk_widget_get_visible ((GtkWidget*) ugtk->window.self) == FALSE)
		gtk_window_set_transient_for ((GtkWindow*) ddialog->self, NULL);
	ug_download_dialog_set_category (ddialog, &ugtk->cwidget);
	ug_download_form_set_folder_list (&ddialog->download,
			ugtk->setting.folder_list);
	list = uget_gtk_clipboard_get_uris (&ugtk->clipboard);
	if (list) {
		gtk_entry_set_text ((GtkEntry*) ddialog->download.url_entry, list->data);
		g_list_foreach (list, (GFunc) g_free, NULL);
		g_list_free (list);
	}
	// connect signal and set data in download dialog
	ddialog->user.app = ugtk;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	on_create_batch (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadDialog*	ddialog;
	gchar*				title;

	title = g_strconcat (UGET_GTK_NAME " - ", _("New Batch Download"), NULL);
	ddialog = ug_download_dialog_new (title, ugtk->window.self);
	g_free (title);
	ug_download_dialog_set_category (ddialog, &ugtk->cwidget);
	ug_download_dialog_use_batch (ddialog);
	ug_download_form_set_folder_list (&ddialog->download,
			ugtk->setting.folder_list);
	// connect signal and set data in download dialog
	ddialog->user.app = ugtk;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show (GTK_WIDGET (ddialog->self));
}

static void	on_create_from_clipboard (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadDialog*	ddialog;
	UgSelectorPage*		page;
	GList*				list;
	gchar*				title;

	list = uget_gtk_clipboard_get_uris (&ugtk->clipboard);
	if (list == NULL) {
		uget_gtk_show_message (ugtk, GTK_MESSAGE_ERROR,
				_("No URLs found in clipboard."));
		return;
	}

	title = g_strconcat (UGET_GTK_NAME " - ", _("New from Clipboard"), NULL);
	ddialog = ug_download_dialog_new (title, ugtk->window.self);
	g_free (title);
	if (gtk_widget_get_visible ((GtkWidget*) ugtk->window.self) == FALSE)
		gtk_window_set_transient_for ((GtkWindow*) ddialog->self, NULL);
	ug_download_dialog_set_category (ddialog, &ugtk->cwidget);
	ug_download_dialog_use_selector (ddialog);
	ug_selector_hide_href (&ddialog->selector);
	page = ug_selector_add_page (&ddialog->selector, _("Clipboard"));
	ug_selector_page_add_uris (page, list);
	g_list_free (list);
	// connect signal and set data in download dialog
	ddialog->user.app = ugtk;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show (GTK_WIDGET (ddialog->self));
}

static void	on_delete_download (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	UgRelation*			relation;
	GList*				list;
	GList*				link;
	// check shift key status
	GdkWindow*			gdk_win;
	GdkModifierType		mask;

	dwidget = ugtk->cwidget.current.widget;
	// check shift key status
	gdk_win = gtk_widget_get_parent_window ((GtkWidget*) dwidget->view);
	gdk_window_get_pointer (gdk_win, NULL, NULL, &mask);

	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		// set status "stop by user"
		ugtk->user_action = TRUE;
		// stop job
		ug_running_remove (ugtk->running, link->data);
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
	// update
	gtk_widget_queue_draw ((GtkWidget*) ugtk->cwidget.self);
	ug_summary_show (&ugtk->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_delete_download_file_response (GtkWidget* widget, gint response_id, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	UgDataCommon*		common;
	gchar*				path;
	GList*				list;
	GList*				link;

	if (response_id != GTK_RESPONSE_YES)
		return;

	dwidget = ugtk->cwidget.current.widget;
	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		// set status "stop by user"
		ugtk->user_action = TRUE;
		// stop job
		ug_running_remove (ugtk->running, link->data);
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
		ug_category_gtk_remove (ugtk->cwidget.current.category, link->data);
	}
	g_list_free (list);
	// update
	gtk_widget_queue_draw ((GtkWidget*) ugtk->cwidget.self);
	ug_summary_show (&ugtk->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_delete_download_file (GtkWidget* widget, UgetGtk* ugtk)
{
	if (ugtk->setting.ui.delete_confirmation == FALSE)
		on_delete_download_file_response (widget, GTK_RESPONSE_YES, ugtk);
	else {
		uget_gtk_confirm_to_delete (ugtk,
				G_CALLBACK (on_delete_download_file_response), ugtk);
	}
}

static void	on_open_download_file (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	UgDataCommon*		common;
	UgDataset*			dataset;
	GtkWidget*			dialog;
	gchar*				string;

	dwidget = ugtk->cwidget.current.widget;
	dataset = ug_download_widget_get_cursor (dwidget);
	if (dataset == NULL)
		return;
	common = ug_dataset_get (dataset, UgDataCommonClass, 0);
	if (common->folder == NULL || common->file == NULL)
		return;

	if (ug_launch_default_app (common->folder, common->file) == FALSE) {
		string = g_strdup_printf (_("Can't launch default application for file '%s'."), common->file);
		dialog = gtk_message_dialog_new (ugtk->window.self,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"%s", string);
		g_free (string);
		string = g_strconcat (UGET_GTK_NAME " - ", _("Error"), NULL);
		gtk_window_set_title ((GtkWindow*) dialog, string);
		g_free (string);
		g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show ((GtkWidget*) dialog);
	}
}

static void	on_open_download_folder (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	UgDataCommon*		common;
	UgDataset*			dataset;
	GtkWidget*			dialog;
	gchar*				string;

	dwidget = ugtk->cwidget.current.widget;
	dataset = ug_download_widget_get_cursor (dwidget);
	if (dataset == NULL)
		return;
	common = ug_dataset_get (dataset, UgDataCommonClass, 0);
	if (common->folder == NULL)
		return;

	string = g_filename_from_utf8 (common->folder, -1, NULL, NULL, NULL);
	if (g_file_test (string, G_FILE_TEST_EXISTS) == FALSE) {
		g_free (string);
		string = g_strdup_printf (_("'%s' - This folder does not exist."), common->folder);
		dialog = gtk_message_dialog_new (ugtk->window.self,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
				"%s", string);
		g_free (string);
		string = g_strconcat (UGET_GTK_NAME " - ", _("Error"), NULL);
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
	UgetGtk*			ugtk;
	GList*				list;
	GList*				link;

	ugtk = ddialog->user.app;
	dwidget = ugtk->cwidget.current.widget;
	if (response_id == GTK_RESPONSE_OK) {
		ug_download_form_get_folder_list (&ddialog->download,
				&ugtk->setting.folder_list);
		list = ug_download_widget_get_selected (dwidget);
		for (link = list;  link;  link = link->next)
			ug_download_dialog_get (ddialog, link->data);
		g_list_free (list);
	}
	ug_download_dialog_free (ddialog);
	// refresh other data & status
	gtk_widget_set_sensitive ((GtkWidget*) ugtk->window.self, TRUE);
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&ugtk->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_config_download (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadDialog*	ddialog;
	gchar*				title;
	GList*				list;

	title = g_strconcat (UGET_GTK_NAME " - ", _("Download Properties"), NULL);
	ddialog = ug_download_dialog_new (title, ugtk->window.self);
	g_free (title);
	// UgDownloadForm
	list = ug_download_widget_get_selected (ugtk->cwidget.current.widget);
	ug_download_form_set_multiple (&ddialog->download,
			(list->next) ? TRUE : FALSE);
	ug_download_form_set_folder_list (&ddialog->download,
			ugtk->setting.folder_list);
	ug_download_form_set_relation (&ddialog->download, FALSE);
	ug_download_dialog_set (ddialog, list->data);
	g_list_free (list);
	// connect signal and set data in download dialog
	ddialog->user.app = ugtk;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_config_download_response), ddialog);
	gtk_widget_set_sensitive ((GtkWidget*) ugtk->window.self, FALSE);
	gtk_widget_show (GTK_WIDGET (ddialog->self));
}

static void	on_set_download_runnable (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	UgRelation*			relation;
	GList*				list;
	GList*				link;

	dwidget = ugtk->cwidget.current.widget;
	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		relation->hints &= ~UG_HINT_UNRUNNABLE;
		// If job is in Finished or Recycled, move it to Queuing.
		ug_category_gtk_changed (relation->category, link->data);
	}
	g_list_free (list);
	// refresh other data & status
	gtk_widget_queue_draw (ugtk->cwidget.self);
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&ugtk->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_set_download_to_pause (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	UgRelation*			relation;
	GList*				list;
	GList*				link;

	dwidget = ugtk->cwidget.current.widget;
	list = ug_download_widget_get_selected (dwidget);
	for (link = list;  link;  link = link->next) {
		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		relation->hints |=  UG_HINT_PAUSED;
		// set status "stop by user"
		ugtk->user_action = TRUE;
		// stop job
		ug_running_remove (ugtk->running, link->data);
	}
	g_list_free (list);
	// refresh other data & status
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&ugtk->summary, ug_download_widget_get_cursor (dwidget));
}

static void	on_move_download_up (GtkWidget* widget, UgetGtk* ugtk)
{
	if (ugtk->cwidget.current.category == NULL)
		return;

	if (ug_category_gtk_move_selected_up (ugtk->cwidget.current.category,
			ugtk->cwidget.current.widget))
	{
		gtk_widget_set_sensitive (ugtk->toolbar.move_down, TRUE);
		gtk_widget_set_sensitive (ugtk->toolbar.move_bottom, TRUE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_down, TRUE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_bottom, TRUE);
	}
	else {
		gtk_widget_set_sensitive (ugtk->toolbar.move_up, FALSE);
		gtk_widget_set_sensitive (ugtk->toolbar.move_top, FALSE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_up, FALSE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_top, FALSE);
	}
}

static void	on_move_download_down (GtkWidget* widget, UgetGtk* ugtk)
{
	if (ugtk->cwidget.current.category == NULL)
		return;

	if (ug_category_gtk_move_selected_down (ugtk->cwidget.current.category,
			ugtk->cwidget.current.widget))
	{
		gtk_widget_set_sensitive (ugtk->toolbar.move_up, TRUE);
		gtk_widget_set_sensitive (ugtk->toolbar.move_top, TRUE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_up, TRUE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_top, TRUE);
	}
	else {
		gtk_widget_set_sensitive (ugtk->toolbar.move_down, FALSE);
		gtk_widget_set_sensitive (ugtk->toolbar.move_bottom, FALSE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_down, FALSE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_bottom, FALSE);
	}
}

static void	on_move_download_to_top (GtkWidget* widget, UgetGtk* ugtk)
{
	if (ugtk->cwidget.current.category == NULL)
		return;

	if (ug_category_gtk_move_selected_to_top (ugtk->cwidget.current.category,
			ugtk->cwidget.current.widget))
	{
		gtk_widget_set_sensitive (ugtk->toolbar.move_down, TRUE);
		gtk_widget_set_sensitive (ugtk->toolbar.move_bottom, TRUE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_down, TRUE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_bottom, TRUE);
	}
	gtk_widget_set_sensitive (ugtk->toolbar.move_up, FALSE);
	gtk_widget_set_sensitive (ugtk->toolbar.move_top, FALSE);
	gtk_widget_set_sensitive (ugtk->menubar.download.move_up, FALSE);
	gtk_widget_set_sensitive (ugtk->menubar.download.move_top, FALSE);
}

static void	on_move_download_to_bottom (GtkWidget* widget, UgetGtk* ugtk)
{
	if (ugtk->cwidget.current.category == NULL)
		return;

	if (ug_category_gtk_move_selected_to_bottom (ugtk->cwidget.current.category,
			ugtk->cwidget.current.widget))
	{
		gtk_widget_set_sensitive (ugtk->toolbar.move_up, TRUE);
		gtk_widget_set_sensitive (ugtk->toolbar.move_top, TRUE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_up, TRUE);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_top, TRUE);
	}
	gtk_widget_set_sensitive (ugtk->toolbar.move_down, FALSE);
	gtk_widget_set_sensitive (ugtk->toolbar.move_bottom, FALSE);
	gtk_widget_set_sensitive (ugtk->menubar.download.move_down, FALSE);
	gtk_widget_set_sensitive (ugtk->menubar.download.move_bottom, FALSE);
}

// ----------------------------------------------------------------------------
// UgetGtkFileMenu
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

static void	on_import_html_file_response (GtkWidget* dialog, gint response, UgetGtk* ugtk)
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
			gtk_window_get_title ((GtkWindow*) dialog), ugtk->window.self);
	ug_download_dialog_set_category (ddialog, &ugtk->cwidget);
	ug_download_dialog_use_selector (ddialog);
	ug_download_form_set_folder_list (&ddialog->download,
			ugtk->setting.folder_list);
	// set <base href>
	gtk_entry_set_text (ddialog->selector.href_entry, string);
	g_free (string);
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
	ddialog->user.app = ugtk;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	on_import_html_file (GtkWidget* widget, UgetGtk* ugtk)
{
	GtkWidget*		dialog;
	gchar*			title;

	title = g_strconcat (UGET_GTK_NAME " - ", _("Import URLs from HTML file"), NULL);
	dialog = create_file_chooser (title, ugtk->window.self,
			"HTML file (*.htm, *.html)", "text/html");
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_import_html_file_response), ugtk);
	gtk_widget_show (dialog);
}

static void	on_import_text_file_response (GtkWidget* dialog, gint response, UgetGtk* ugtk)
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
		uget_gtk_show_message (ugtk, GTK_MESSAGE_ERROR, error->message);
		g_error_free (error);
		return;
	}
	// UgDownloadDialog
	ddialog = ug_download_dialog_new (
			gtk_window_get_title ((GtkWindow*) dialog), ugtk->window.self);
	ug_download_dialog_set_category (ddialog, &ugtk->cwidget);
	ug_download_dialog_use_selector (ddialog);
	ug_download_form_set_folder_list (&ddialog->download,
			ugtk->setting.folder_list);
	page = ug_selector_add_page (&ddialog->selector, _("Text File"));
	ug_selector_hide_href (&ddialog->selector);
	ug_selector_page_add_uris (page, list);
	g_list_free (list);
	// setup & show download dialog
	ddialog->user.app = ugtk;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_create_download_response), ddialog);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	on_import_text_file (GtkWidget* widget, UgetGtk* ugtk)
{
	GtkWidget*		dialog;
	gchar*			title;

	title = g_strconcat (UGET_GTK_NAME " - ", _("Import URLs from text file"), NULL);
	dialog = create_file_chooser (title, ugtk->window.self,
			"Plain text file", "text/plain");
	g_free (title);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_import_text_file_response), ugtk);
	gtk_widget_show (dialog);
}

static void	on_export_text_file_response (GtkWidget* dialog, gint response, UgetGtk* ugtk)
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
	list = ug_category_gtk_get_all (ugtk->cwidget.primary.category);
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

static void	on_export_text_file (GtkWidget* widget, UgetGtk* ugtk)
{
	GtkWidget*		dialog;
	gchar*			title;

	title = g_strconcat (UGET_GTK_NAME " - ", _("Export to"), NULL);
	dialog = gtk_file_chooser_dialog_new (_("Export to"), ugtk->window.self,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK,     GTK_RESPONSE_OK,
			NULL);
	g_free (title);
	gtk_window_set_destroy_with_parent ((GtkWindow*) dialog, TRUE);
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_export_text_file_response), ugtk);
	gtk_widget_show (dialog);
}

static void	on_offline_mode (GtkWidget* widget, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	GtkCheckMenuItem*	item;

	item = GTK_CHECK_MENU_ITEM (widget);
	ugtk->setting.offline_mode = gtk_check_menu_item_get_active (item);
	item = GTK_CHECK_MENU_ITEM (ugtk->menubar.file.offline_mode);
	gtk_check_menu_item_set_active (item, ugtk->setting.offline_mode);
	item = GTK_CHECK_MENU_ITEM (ugtk->tray_icon.menu.offline_mode);
	gtk_check_menu_item_set_active (item, ugtk->setting.offline_mode);

	// into offline mode
	if (ugtk->setting.offline_mode == TRUE) {
		// set status "stop by user"
		ugtk->user_action = TRUE;
		// stop all active jobs
		ug_running_clear (ugtk->running);
		// refresh
		gtk_widget_queue_draw (ugtk->cwidget.self);
		dwidget = ugtk->cwidget.current.widget;
		gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
//		ug_summary_show (&ugtk->summary, ug_download_widget_get_cursor (dwidget));
	}
}

// ----------------------------------------------------------------------------
// UgetGtkEditMenu
//
static void	on_monitor_clipboard (GtkWidget* widget, UgetGtk* ugtk)
{
	gboolean	active;

	active = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	ugtk->setting.clipboard.monitor = active;
}

static void	on_config_settings_response (GtkDialog *dialog, gint response, UgSettingDialog* sdialog)
{
	UgetGtk*	ugtk;

	ugtk = sdialog->user_data;
	ugtk->dialogs.setting = NULL;
	if (response == GTK_RESPONSE_OK) {
		ug_setting_dialog_get (sdialog, &ugtk->setting);
		// clipboard
		uget_gtk_clipboard_set_pattern (&ugtk->clipboard,
				ugtk->setting.clipboard.pattern);
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*) ugtk->menubar.edit.clipboard_monitor,
				ugtk->setting.clipboard.monitor);
		// launch
		g_regex_unref (ugtk->launch_regex);
		ugtk->launch_regex = g_regex_new (ugtk->setting.launch.types,
				G_REGEX_CASELESS, 0, NULL);
	}
	ug_setting_dialog_free (sdialog);
	// refresh
//	gtk_check_menu_item_toggled ((GtkCheckMenuItem*) ugtk->menubar.edit.clipboard_monitor);
	on_monitor_clipboard (ugtk->menubar.edit.clipboard_monitor, ugtk);
}

static void	on_config_shutdown (GtkWidget* widget, UgetGtk* ugtk)
{
	gboolean	active;

	active = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	ugtk->setting.shutdown = active;
}

static void	on_config_settings (GtkWidget* widget, UgetGtk* ugtk)
{
	UgSettingDialog*	sdialog;
	gchar*				title;

	if (ugtk->dialogs.setting) {
		gtk_window_present ((GtkWindow*) ugtk->dialogs.setting);
		return;
	}
	title = g_strconcat (UGET_GTK_NAME " - ", _("Settings"), NULL);
	sdialog = ug_setting_dialog_new (title, ugtk->window.self);
	g_free (title);
	ug_setting_dialog_set (sdialog, &ugtk->setting);
	ugtk->dialogs.setting = (GtkWidget*) sdialog->self;
	// set page
	if (widget == ugtk->menubar.edit.clipboard_option)
		gtk_notebook_set_current_page (sdialog->notebook, UG_SETTING_PAGE_CLIPBOARD);
	else
		gtk_notebook_set_current_page (sdialog->notebook, UG_SETTING_PAGE_UI);
	// show settings dialog
	sdialog->user_data = ugtk;
	g_signal_connect (sdialog->self, "response",
			G_CALLBACK (on_config_settings_response), sdialog);
	gtk_widget_show ((GtkWidget*) sdialog->self);
}

static void	on_config_clipboard (GtkWidget* widget, UgetGtk* ugtk)
{
	on_config_settings (widget, ugtk);
}

// ----------------------------------------------------------------------------
// UgetGtkViewMenu
//
void	on_change_visible_widget (GtkWidget* widget, UgetGtk* ugtk)
{
	struct UgWindowSetting*	setting;
	gboolean				visible;

	setting = &ugtk->setting.window;
	visible = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// Toolbar
	if (widget == ugtk->menubar.view.toolbar) {
		setting->toolbar = visible;
		if (visible)
			gtk_widget_show (ugtk->toolbar.self);
		else
			gtk_widget_hide (ugtk->toolbar.self);
		return;
	}
	// Statusbar
	if (widget == ugtk->menubar.view.statusbar) {
		setting->statusbar = visible;
		if (visible)
			gtk_widget_show ((GtkWidget*) ugtk->statusbar.self);
		else
			gtk_widget_hide ((GtkWidget*) ugtk->statusbar.self);
		return;
	}
	// Category
	if (widget == ugtk->menubar.view.category) {
		setting->category = visible;
		if (visible)
			gtk_widget_show (ugtk->cwidget.self);
		else
			gtk_widget_hide (ugtk->cwidget.self);
		return;
	}
	// Category
	if (widget == ugtk->menubar.view.summary) {
		setting->summary = visible;
		if (visible)
			gtk_widget_show (ugtk->summary.self);
		else
			gtk_widget_hide (ugtk->summary.self);
		return;
	}
}

void	on_change_visible_summary (GtkWidget* widget, UgetGtk* ugtk)
{
	struct UgSummarySetting*	setting;
	gboolean					visible;

	setting = &ugtk->setting.summary;
	visible = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// which widget
	if (widget == ugtk->menubar.view.summary_items.name) {
		setting->name = visible;
		ugtk->summary.visible.name = visible;
	}
	else if (widget == ugtk->menubar.view.summary_items.folder) {
		setting->folder = visible;
		ugtk->summary.visible.folder = visible;
	}
	else if (widget == ugtk->menubar.view.summary_items.category) {
		setting->category = visible;
		ugtk->summary.visible.category = visible;
	}
	else if (widget == ugtk->menubar.view.summary_items.url) {
		setting->url = visible;
		ugtk->summary.visible.url = visible;
	}
	else if (widget == ugtk->menubar.view.summary_items.message) {
		setting->message = visible;
		ugtk->summary.visible.message = visible;
	}

	ug_summary_show (&ugtk->summary,
			ug_download_widget_get_cursor (ugtk->cwidget.current.widget));
}

void	on_change_visible_column (GtkWidget* widget, UgetGtk* ugtk)
{
	struct UgDownloadColumnSetting*	setting;
	UgDownloadWidget*	dwidget;
	GtkTreeViewColumn*	column;
	gboolean			visible;
	gint				column_index;

	setting = &ugtk->setting.download_column;
	dwidget = ugtk->cwidget.current.widget;
	visible = gtk_check_menu_item_get_active ((GtkCheckMenuItem*) widget);
	// which widget
	if (widget == ugtk->menubar.view.columns.completed) {
		column_index = UG_DOWNLOAD_COLUMN_COMPLETE;
		setting->completed = visible;
	}
	else if (widget == ugtk->menubar.view.columns.total) {
		column_index = UG_DOWNLOAD_COLUMN_SIZE;
		setting->total = visible;
	}
	else if (widget == ugtk->menubar.view.columns.percent) {
		column_index = UG_DOWNLOAD_COLUMN_PERCENT;
		setting->percent = visible;
	}
	else if (widget == ugtk->menubar.view.columns.elapsed) {
		column_index = UG_DOWNLOAD_COLUMN_ELAPSED;
		setting->elapsed = visible;
	}
	else if (widget == ugtk->menubar.view.columns.left) {
		column_index = UG_DOWNLOAD_COLUMN_LEFT;
		setting->left = visible;
	}
	else if (widget == ugtk->menubar.view.columns.speed) {
		column_index = UG_DOWNLOAD_COLUMN_SPEED;
		setting->speed = visible;
	}
	else if (widget == ugtk->menubar.view.columns.retry) {
		column_index = UG_DOWNLOAD_COLUMN_RETRY;
		setting->retry = visible;
	}
	else if (widget == ugtk->menubar.view.columns.category) {
		column_index = UG_DOWNLOAD_COLUMN_CATEGORY;
		setting->category = visible;
	}
	else if (widget == ugtk->menubar.view.columns.url) {
		column_index = UG_DOWNLOAD_COLUMN_URL;
		setting->url = visible;
	}
	else if (widget == ugtk->menubar.view.columns.added_on) {
		column_index = UG_DOWNLOAD_COLUMN_ADDED_ON;
		setting->added_on = visible;
	}
	else if (widget == ugtk->menubar.view.columns.completed_on) {
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
// UgetGtkHelpMenu
//
void	on_about (GtkWidget* widget, UgetGtk* ugtk)
{
//	if (gtk_widget_get_visible ((GtkWidget*) ugtk->window.self) == FALSE) {
//		gtk_window_deiconify (ugtk->window.self);
//		gtk_widget_show ((GtkWidget*) ugtk->window.self);
//	}

	gtk_show_about_dialog (ugtk->window.self,
			"logo-icon-name", UGET_GTK_ICON_NAME,
			"program-name", UGET_GTK_NAME,
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
// UgetGtkTrayIcon
//
static void	on_tray_icon_activate (GtkStatusIcon* status_icon, UgetGtk* ugtk)
{
	if (gtk_widget_get_visible ((GtkWidget*) ugtk->window.self) == TRUE) {
		// get position and size
		gtk_window_get_position (ugtk->window.self,
				&ugtk->setting.window.x, &ugtk->setting.window.y);
		gtk_window_get_size (ugtk->window.self,
				&ugtk->setting.window.width, &ugtk->setting.window.height);
		// hide window
		gtk_window_iconify (ugtk->window.self);
		gtk_widget_hide ((GtkWidget*) ugtk->window.self);
	}
	else {
		gtk_widget_show ((GtkWidget*) ugtk->window.self);
		gtk_window_deiconify (ugtk->window.self);
		gtk_window_present (ugtk->window.self);
	}
	// clear error status
	if (ugtk->tray_icon.error_occurred) {
		ugtk->tray_icon.error_occurred = FALSE;
		gtk_status_icon_set_from_icon_name (status_icon, UGET_GTK_ICON_NAME);
	}
}

static void	on_tray_icon_popup_menu (GtkStatusIcon* status_icon, guint button, guint activate_time, UgetGtk* ugtk)
{
	gtk_menu_set_screen ((GtkMenu*) ugtk->tray_icon.menu.self,
			gtk_status_icon_get_screen (status_icon));
#ifdef _WIN32
	gtk_menu_popup ((GtkMenu*) ugtk->tray_icon.menu.self,
			NULL, NULL,
			NULL, NULL,
			button, activate_time);
#else
	gtk_menu_popup ((GtkMenu*) ugtk->tray_icon.menu.self,
			NULL, NULL,
			gtk_status_icon_position_menu, status_icon,
			button, activate_time);
#endif
}

// ----------------------------------------------------------------------------
// UgetGtkWindow

// button-press-event
static gboolean	on_button_press_event (GtkTreeView* treeview, GdkEventButton* event, UgetGtk* ugtk)
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
	if (treeview == ugtk->cwidget.view || treeview == ugtk->cwidget.primary.view)
		menu = (GtkMenu*) ugtk->menubar.category.self;
	else if (treeview == ugtk->summary.view)
		menu = ugtk->summary.menu.self;
	else
		menu = (GtkMenu*) ugtk->menubar.download.self;
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
	gtk_widget_size_request (GTK_WIDGET (menu), &menu_requisition);

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
static gboolean	on_window_key_press_event (GtkWidget *widget, GdkEventKey *event, UgetGtk* ugtk)
{
	GtkTreeView*	focus;
	GtkMenu*		menu;

//	g_print ("key-press : 0x%x\n", event->keyval);
	if (event->keyval != GDK_KEY_Menu)
		return FALSE;

	focus = (GtkTreeView*) gtk_window_get_focus (ugtk->window.self);
	if (focus == ugtk->cwidget.primary.view) {
		widget = (GtkWidget*) ugtk->cwidget.primary.view;
		menu = (GtkMenu*) ugtk->menubar.category.self;
	}
	else if (focus == ugtk->cwidget.view) {
		widget = (GtkWidget*) ugtk->cwidget.view;
		menu = (GtkMenu*) ugtk->menubar.category.self;
	}
	else if (focus == ugtk->summary.view) {
		widget = (GtkWidget*) ugtk->summary.view;
		menu = (GtkMenu*) ugtk->summary.menu.self;
	}
	else if (focus == ugtk->cwidget.current.widget->view) {
		widget = (GtkWidget*) ugtk->cwidget.current.widget->view;
		menu = (GtkMenu*) ugtk->menubar.download.self;
	}
	else
		return FALSE;

	gtk_menu_popup (menu, NULL, NULL,
			menu_position_func, widget,
			0, gtk_get_current_event_time());
	return TRUE;
}

// UgetGtkWindow.self "delete-event"
static gboolean	on_window_delete_event (GtkWidget* widget, GdkEvent* event, UgetGtk* ugtk)
{
	if (ugtk->setting.ui.close_confirmation == FALSE) {
		uget_gtk_close_window (ugtk);
		return TRUE;
	}
	uget_gtk_confirm_to_quit (ugtk);
	return TRUE;
}

// UgDownloadWidget.view "key-press-event"
static gboolean	on_download_key_press_event  (GtkWidget* widget, GdkEventKey* event, UgetGtk* ugtk)
{
	if (event->keyval == GDK_KEY_Delete  &&  ugtk->cwidget.current.category) {
		on_delete_download (widget, ugtk);
		return TRUE;
	}
	return FALSE;
}

// UgDownloadWidget.view selection "changed"
static void	on_download_selection_changed (GtkTreeSelection* selection, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;

	dwidget = ugtk->cwidget.current.widget;
	uget_gtk_statusbar_refresh (&ugtk->statusbar, dwidget);
	uget_gtk_refresh_download_sensitive (ugtk);
}

// UgDownloadWidget.view "cursor-changed"
static void	on_download_cursor_changed (GtkTreeView* view, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	UgDataset*			dataset;

	dwidget = ugtk->cwidget.current.widget;
	dataset = ug_download_widget_get_cursor (dwidget);
	ug_summary_show (&ugtk->summary, dataset);
}

// UgCategoryWidget.view and primary_view "cursor-changed"
static void	on_category_cursor_changed (GtkTreeView* view, UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	GtkTreeSelection*	selection;
	GList*				list;

	// ----------------------------------------------------
	// switch UgDownloadWidget in right side
	dwidget = ugtk->cwidget.current.widget;
	list = gtk_container_get_children (GTK_CONTAINER (ugtk->window.vpaned));
	list = g_list_remove (list, ugtk->summary.self);
	if (list) {
		g_object_ref (list->data);
		gtk_container_remove (GTK_CONTAINER (ugtk->window.vpaned), list->data);
		g_list_free (list);
	}
	gtk_paned_pack1 (ugtk->window.vpaned, dwidget->self, TRUE, TRUE);

	// connect signals
	if (dwidget) {
		if (dwidget->signal_connected == FALSE) {
			dwidget->signal_connected =  TRUE;
			selection = gtk_tree_view_get_selection (dwidget->view);
			g_signal_connect (selection, "changed",
					G_CALLBACK (on_download_selection_changed), ugtk);
			g_signal_connect (dwidget->view, "cursor-changed",
					G_CALLBACK (on_download_cursor_changed), ugtk);
			g_signal_connect (dwidget->view, "key-press-event",
					G_CALLBACK (on_download_key_press_event), ugtk);
			g_signal_connect (dwidget->view, "button-press-event",
					G_CALLBACK (on_button_press_event), ugtk);
		}
		// refresh summary
		ug_summary_show (&ugtk->summary,
				ug_download_widget_get_cursor (dwidget));
	}

	// refresh
	uget_gtk_move_menu_refresh (&ugtk->menubar, ugtk, FALSE);
	uget_gtk_statusbar_refresh (&ugtk->statusbar, dwidget);
	uget_gtk_refresh_download_column (ugtk);
	uget_gtk_refresh_category_sensitive (ugtk);
}

// UgSummary.menu.copy signal handler
static void	on_summary_copy_selected (GtkWidget* widget, UgetGtk* ugtk)
{
	gchar*	text;

	text = ug_summary_get_text_selected (&ugtk->summary);
	uget_gtk_clipboard_set_text (&ugtk->clipboard, text);
}

// UgSummary.menu.copy_all signal handler
static void	on_summary_copy_all (GtkWidget* widget, UgetGtk* ugtk)
{
	gchar*	text;

	text = ug_summary_get_text_all (&ugtk->summary);
	uget_gtk_clipboard_set_text (&ugtk->clipboard, text);
}

// ----------------------------------------------------------------------------
// used by uget_gtk_init_callback()

// UgetGtkWindow
static void uget_gtk_window_init_callback (struct UgetGtkWindow* window, UgetGtk* ugtk)
{
	// UgCategoryWidget
	g_signal_connect_after (ugtk->cwidget.primary.view, "cursor-changed",
			G_CALLBACK (on_category_cursor_changed), ugtk);
	g_signal_connect_after (ugtk->cwidget.view, "cursor-changed",
			G_CALLBACK (on_category_cursor_changed), ugtk);

	// pop-up menu by mouse button
	g_signal_connect (ugtk->cwidget.primary.view, "button-press-event",
			G_CALLBACK (on_button_press_event), ugtk);
	g_signal_connect (ugtk->cwidget.view, "button-press-event",
			G_CALLBACK (on_button_press_event), ugtk);
	g_signal_connect (ugtk->summary.view, "button-press-event",
			G_CALLBACK (on_button_press_event), ugtk);

	// UgSummary.menu signal handlers
	g_signal_connect (ugtk->summary.menu.copy, "activate",
			G_CALLBACK (on_summary_copy_selected), ugtk);
	g_signal_connect (ugtk->summary.menu.copy_all, "activate",
			G_CALLBACK (on_summary_copy_all), ugtk);

	// UgetGtkWindow.self signal handlers
	g_signal_connect (window->self, "key-press-event",
			G_CALLBACK (on_window_key_press_event), ugtk);
	g_signal_connect (window->self, "delete-event",
			G_CALLBACK (on_window_delete_event), ugtk);
	g_signal_connect_swapped (window->self, "destroy",
			G_CALLBACK (uget_gtk_quit), ugtk);
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

// UgetGtkTrayIcon
static void uget_gtk_tray_icon_init_callback (struct UgetGtkTrayIcon* icon, UgetGtk* ugtk)
{
	g_signal_connect (icon->self, "activate",
			G_CALLBACK (on_tray_icon_activate), ugtk);
	g_signal_connect (icon->self, "popup-menu",
			G_CALLBACK (on_tray_icon_popup_menu), ugtk);
#ifdef _WIN32
	g_signal_connect (icon->menu.self, "leave-notify-event",
			G_CALLBACK (tray_menu_leave_enter), NULL);
	g_signal_connect (icon->menu.self, "enter-notify-event",
			G_CALLBACK (tray_menu_leave_enter), NULL);
#endif

	g_signal_connect (icon->menu.create_download, "activate",
			G_CALLBACK (on_create_download), ugtk);
	g_signal_connect (icon->menu.create_clipboard, "activate",
			G_CALLBACK (on_create_from_clipboard), ugtk);
	g_signal_connect (icon->menu.settings, "activate",
			G_CALLBACK (on_config_settings), ugtk);
	g_signal_connect (icon->menu.offline_mode, "toggled",
			G_CALLBACK (on_offline_mode), ugtk);
	g_signal_connect (icon->menu.about, "activate",
			G_CALLBACK (on_about), ugtk);
	g_signal_connect_swapped (icon->menu.quit, "activate",
			G_CALLBACK (uget_gtk_quit), ugtk);
}

// UgetGtkToolbar
static void uget_gtk_toolbar_init_callback (struct UgetGtkToolbar* toolbar, UgetGtk* ugtk)
{
	// create new
	g_signal_connect (toolbar->create, "clicked",
			G_CALLBACK (on_create_download), ugtk);
	g_signal_connect (toolbar->create_download, "activate",
			G_CALLBACK (on_create_download), ugtk);
	g_signal_connect (toolbar->create_category, "activate",
			G_CALLBACK (on_create_category), ugtk);
	g_signal_connect (toolbar->create_batch, "activate",
			G_CALLBACK (on_create_batch), ugtk);
	g_signal_connect (toolbar->create_clipboard, "activate",
			G_CALLBACK (on_create_from_clipboard), ugtk);
	// save
	g_signal_connect_swapped (toolbar->save, "clicked",
			G_CALLBACK (uget_gtk_save), ugtk);
	// change status
	g_signal_connect (toolbar->runnable, "clicked",
			G_CALLBACK (on_set_download_runnable), ugtk);
	g_signal_connect (toolbar->pause, "clicked",
			G_CALLBACK (on_set_download_to_pause), ugtk);
	// change data
	g_signal_connect (toolbar->properties, "clicked",
			G_CALLBACK (on_config_download), ugtk);
	// move
	g_signal_connect (toolbar->move_up, "clicked",
			G_CALLBACK (on_move_download_up), ugtk);
	g_signal_connect (toolbar->move_down, "clicked",
			G_CALLBACK (on_move_download_down), ugtk);
	g_signal_connect (toolbar->move_top, "clicked",
			G_CALLBACK (on_move_download_to_top), ugtk);
	g_signal_connect (toolbar->move_bottom, "clicked",
			G_CALLBACK (on_move_download_to_bottom), ugtk);
}

// UgetGtkMenubar
static void uget_gtk_menubar_init_callback (struct UgetGtkMenubar* menubar, UgetGtk* ugtk)
{
	// ----------------------------------------------------
	// UgetGtkFileMenu
	g_signal_connect (menubar->file.create.download, "activate",
			G_CALLBACK (on_create_download), ugtk);
	g_signal_connect (menubar->file.create.category, "activate",
			G_CALLBACK (on_create_category), ugtk);
	g_signal_connect (menubar->file.create.batch, "activate",
			G_CALLBACK (on_create_batch), ugtk);
	g_signal_connect (menubar->file.create.from_clipboard, "activate",
			G_CALLBACK (on_create_from_clipboard), ugtk);
	g_signal_connect_swapped (menubar->file.save, "activate",
			G_CALLBACK (uget_gtk_save), ugtk);
	g_signal_connect (menubar->file.import_html, "activate",
			G_CALLBACK (on_import_html_file), ugtk);
	g_signal_connect (menubar->file.import_text, "activate",
			G_CALLBACK (on_import_text_file), ugtk);
	g_signal_connect (menubar->file.export_text, "activate",
			G_CALLBACK (on_export_text_file), ugtk);
	g_signal_connect (menubar->file.offline_mode, "toggled",
			G_CALLBACK (on_offline_mode), ugtk);
	g_signal_connect_swapped (menubar->file.quit, "activate",
			G_CALLBACK (uget_gtk_quit), ugtk);

	// ----------------------------------------------------
	// UgetGtkEditMenu
	g_signal_connect (menubar->edit.clipboard_monitor, "activate",
			G_CALLBACK (on_monitor_clipboard), ugtk);
	g_signal_connect (menubar->edit.clipboard_option, "activate",
			G_CALLBACK (on_config_clipboard), ugtk);
	g_signal_connect (menubar->edit.shutdown, "activate",
			G_CALLBACK (on_config_shutdown), ugtk);
	g_signal_connect (menubar->edit.settings, "activate",
			G_CALLBACK (on_config_settings), ugtk);

	// ----------------------------------------------------
	// UgetGtkViewMenu
	g_signal_connect (menubar->view.toolbar, "toggled",
			G_CALLBACK (on_change_visible_widget), ugtk);
	g_signal_connect (menubar->view.statusbar, "toggled",
			G_CALLBACK (on_change_visible_widget), ugtk);
	g_signal_connect (menubar->view.category, "toggled",
			G_CALLBACK (on_change_visible_widget), ugtk);
	g_signal_connect (menubar->view.summary, "toggled",
			G_CALLBACK (on_change_visible_widget), ugtk);
	// summary items
	g_signal_connect (menubar->view.summary_items.name, "toggled",
			G_CALLBACK (on_change_visible_summary), ugtk);
	g_signal_connect (menubar->view.summary_items.folder, "toggled",
			G_CALLBACK (on_change_visible_summary), ugtk);
	g_signal_connect (menubar->view.summary_items.category, "toggled",
			G_CALLBACK (on_change_visible_summary), ugtk);
	g_signal_connect (menubar->view.summary_items.url, "toggled",
			G_CALLBACK (on_change_visible_summary), ugtk);
	g_signal_connect (menubar->view.summary_items.message, "toggled",
			G_CALLBACK (on_change_visible_summary), ugtk);
	// download columns
	g_signal_connect (menubar->view.columns.completed, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.total, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.percent, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.elapsed, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.left, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.speed, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.retry, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.category, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.url, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.added_on, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);
	g_signal_connect (menubar->view.columns.completed_on, "toggled",
			G_CALLBACK (on_change_visible_column), ugtk);

	// ----------------------------------------------------
	// UgetGtkCategoryMenu
	g_signal_connect (menubar->category.create, "activate",
			G_CALLBACK (on_create_category), ugtk);
	g_signal_connect (menubar->category.delete, "activate",
			G_CALLBACK (on_delete_category), ugtk);
	g_signal_connect (menubar->category.properties, "activate",
			G_CALLBACK (on_config_category), ugtk);
	g_signal_connect (menubar->category.default_for_new, "activate",
			G_CALLBACK (on_config_category_default), ugtk);

	// ----------------------------------------------------
	// UgetGtkDownloadMenu
	g_signal_connect (menubar->download.create, "activate",
			G_CALLBACK (on_create_download), ugtk);
	g_signal_connect (menubar->download.delete, "activate",
			G_CALLBACK (on_delete_download), ugtk);
	// file & folder
	g_signal_connect (menubar->download.delete_file, "activate",
			G_CALLBACK (on_delete_download_file), ugtk);
	g_signal_connect (menubar->download.open, "activate",
			G_CALLBACK (on_open_download_file), ugtk);
	g_signal_connect (menubar->download.open_folder, "activate",
			G_CALLBACK (on_open_download_folder), ugtk);
	// change status
	g_signal_connect (menubar->download.runnable, "activate",
			G_CALLBACK (on_set_download_runnable), ugtk);
	g_signal_connect (menubar->download.pause, "activate",
			G_CALLBACK (on_set_download_to_pause), ugtk);
	// move
	g_signal_connect (menubar->download.move_up, "activate",
			G_CALLBACK (on_move_download_up), ugtk);
	g_signal_connect (menubar->download.move_down, "activate",
			G_CALLBACK (on_move_download_down), ugtk);
	g_signal_connect (menubar->download.move_top, "activate",
			G_CALLBACK (on_move_download_to_top), ugtk);
	g_signal_connect (menubar->download.move_bottom, "activate",
			G_CALLBACK (on_move_download_to_bottom), ugtk);
	// change data
	g_signal_connect (menubar->download.properties, "activate",
			G_CALLBACK (on_config_download), ugtk);

	// ----------------------------------------------------
	// UgetGtkHelpMenu
	g_signal_connect (menubar->help.about_uget, "activate",
			G_CALLBACK (on_about), ugtk);
}

