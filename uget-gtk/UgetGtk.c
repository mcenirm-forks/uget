/*
 *
 *   Copyright (C) 2005-2011 by Raymond Huang
 *   plushuang at users.sourceforge.net
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
#include <UgRegistry.h>
#include <UgData-download.h>
#include <UgetGtk.h>

#include <glib/gi18n.h>

const char*	ug_get_attachment_dir (void)
{
	static const char*	dir = NULL;

	if (dir == NULL) {
		dir = g_build_filename (g_get_user_config_dir (),
				UGET_GTK_DIR, "attachment", NULL);
	}
	return dir;
}

static void uget_gtk_update_config_dir (void)
{
	gchar*	former_dir;
	gchar*	dir;

	former_dir = g_build_filename (g_get_user_config_dir (),
			"Uget", NULL);
	dir = g_filename_from_utf8 (former_dir, -1, NULL, NULL, NULL);
	if (g_file_test (dir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
		g_free (dir);
		dir = g_build_filename (g_get_user_config_dir (),
				UGET_GTK_DIR, NULL);
		ug_rename (former_dir, dir);
	}

	g_free (former_dir);
	g_free (dir);
}

void	uget_gtk_init (UgetGtk* ugtk)
{
	UgCategory*		category;
	UgDataCommon*	common;

	ugtk->running = ug_running_new ();
	// upgrade from Uget 1.6
	uget_gtk_update_config_dir ();
	// initialize widgets in UgetGtk-gui.c
	uget_gtk_init_gui (ugtk);
	// initialize settings
	uget_gtk_setting_init (&ugtk->setting);
	uget_gtk_setting_reset (&ugtk->setting);	// reset to default
	// load settings & data
	ug_attachment_init (ug_get_attachment_dir ());
	uget_gtk_load (ugtk);
	ug_attachment_sync ();
	// apply settings
	uget_gtk_set_setting (ugtk, &ugtk->setting);
	uget_gtk_clipboard_init (&ugtk->clipboard, ugtk->setting.clipboard.pattern);
	ugtk->launch_regex = g_regex_new (ugtk->setting.launch.types,
			G_REGEX_CASELESS, 0, NULL);
	// If no category exists, create new one.
	if (ug_category_widget_n_category (&ugtk->cwidget) == 0) {
		category = ug_category_new_with_gtk (ugtk->cwidget.primary.category);
		category->name = g_strdup ("Home");
		common = ug_dataset_alloc_front (category->defaults, UgDataCommonClass);
		common->folder = g_strdup (g_get_home_dir ());
		ug_category_widget_append (&ugtk->cwidget, category);
	}
	// initialize signal handlers in UgetGtk-callback.c
	uget_gtk_init_callback (ugtk);
	// initialize timeout in UgetGtk-timeout.c
	uget_gtk_init_timeout (ugtk);

	uget_gtk_move_menu_refresh (&ugtk->menubar, ugtk, TRUE);
	ug_category_view_set_cursor (ugtk->cwidget.primary.view, 0, -1);
	gtk_window_set_focus (ugtk->window.self,
			(GtkWidget*) ugtk->cwidget.primary.view);

	if (ugtk->setting.ui.start_in_offline_mode)
	    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (ugtk->tray_icon.menu.offline_mode), TRUE);
	if (ugtk->setting.ui.start_in_tray == FALSE)
		gtk_widget_show ((GtkWidget*) ugtk->window.self);
}

void	uget_gtk_quit (UgetGtk* ugtk)
{
	// stop all active download
	ug_running_clear (ugtk->running);
	// get and update setting before program save it
	uget_gtk_get_setting (ugtk, &ugtk->setting);
	// save data
	uget_gtk_save (ugtk);
	// hide icon in system tray before quit
	gtk_status_icon_set_visible (ugtk->tray_icon.self, FALSE);
	// hide window
	gtk_widget_hide (GTK_WIDGET (ugtk->window.self));
	// This will quit  gtk_main()  to  main()  in  main-gtk.c
	gtk_main_quit ();
}

void	uget_gtk_save (UgetGtk* ugtk)
{
	GList*			list;
	gchar*			file;

	// save setting
	file = g_build_filename (g_get_user_config_dir (),
			UGET_GTK_DIR, UGET_GTK_SETTING_FILE, NULL);
	uget_gtk_setting_save (&ugtk->setting, file);
	g_free (file);
	// get and save all jobs from primary category
	list = ug_category_gtk_get_all (ugtk->cwidget.primary.category);
	file = g_build_filename (g_get_user_config_dir (),
			UGET_GTK_DIR, UGET_GTK_DOWNLOAD_FILE, NULL);
	ug_download_list_save (list, file);
	g_list_free (list);
	g_free (file);
	// save categories
	file = g_build_filename (g_get_user_config_dir (),
			UGET_GTK_DIR, UGET_GTK_CATEGORY_FILE, NULL);
	list = ug_category_widget_get_list (&ugtk->cwidget);
	ug_category_list_save (list, file);
	g_list_free (list);
	g_free (file);
}

void	uget_gtk_load (UgetGtk* ugtk)
{
	GList*			category_list;
	GList*			download_list;
	GList*			link;
	gchar*			file;

	// load setting
	file = g_build_filename (g_get_user_config_dir (),
			UGET_GTK_DIR, UGET_GTK_SETTING_FILE, NULL);
	uget_gtk_setting_load (&ugtk->setting, file);
	g_free (file);
	// load all jobs from file
	file = g_build_filename (g_get_user_config_dir (),
			UGET_GTK_DIR, UGET_GTK_DOWNLOAD_FILE, NULL);
	download_list = ug_download_list_load (file);
	g_free (file);
	// load all categories
	file = g_build_filename (g_get_user_config_dir (),
			UGET_GTK_DIR, UGET_GTK_CATEGORY_FILE, NULL);
	category_list = ug_category_list_load (file);
	g_free (file);
	// set primary category
	for (link = category_list;  link;  link = link->next)
		ug_category_use_gtk (link->data, ugtk->cwidget.primary.category);
	// add jobs to primary category
	for (link = download_list;  link;  link = link->next)
		ug_category_add (ugtk->cwidget.primary.category, link->data);
	// link and add jobs to categories
	ug_category_list_link (category_list, download_list);
	ug_category_widget_add_list (&ugtk->cwidget, category_list);
	g_list_free (category_list);
	// free unused jobs and list
	g_list_foreach (download_list, (GFunc) ug_dataset_unref, NULL);
	g_list_free (download_list);
}

void	uget_gtk_set_setting (UgetGtk* ugtk, UgetGtkSetting* setting)
{
	// set window position, size, and maximized state
	if (setting->window.width  > 0 &&
	    setting->window.height > 0 &&
	    setting->window.x < gdk_screen_width ()  &&
	    setting->window.y < gdk_screen_height () &&
	    setting->window.x + setting->window.width > 0  &&
	    setting->window.y + setting->window.height > 0)
	{
		gtk_window_move (ugtk->window.self,
				setting->window.x, setting->window.y);
		gtk_window_resize (ugtk->window.self,
				setting->window.width, setting->window.height);
	}
	if (setting->window.maximized)
		gtk_window_maximize (ugtk->window.self);
	// set visible widgets
	gtk_widget_set_visible (ugtk->toolbar.self,
			setting->window.toolbar);
	gtk_widget_set_visible ((GtkWidget*) ugtk->statusbar.self,
			setting->window.statusbar);
	gtk_widget_set_visible (ugtk->cwidget.self,
			setting->window.category);
	gtk_widget_set_visible (ugtk->summary.self,
			setting->window.summary);
	// set user interface
	gtk_status_icon_set_visible (ugtk->tray_icon.self,
			setting->ui.show_tray_icon);
	// ----------------------------------------------------
	// UgetGtkEditMenu
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.edit.clipboard_monitor,
			setting->clipboard.monitor);
	// ----------------------------------------------------
	// UgetGtkViewMenu
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.toolbar,
			setting->window.toolbar);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.statusbar,
			setting->window.statusbar);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.category,
			setting->window.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.summary,
			setting->window.summary);
	// summary items
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.summary_items.name,
			setting->summary.name);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.summary_items.folder,
			setting->summary.folder);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.summary_items.category,
			setting->summary.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.summary_items.url,
			setting->summary.url);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.summary_items.message,
			setting->summary.message);
	// download column
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.completed,
			setting->download_column.completed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.total,
			setting->download_column.total);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.percent,
			setting->download_column.percent);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.elapsed,
			setting->download_column.elapsed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.left,
			setting->download_column.left);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.speed,
			setting->download_column.speed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.retry,
			setting->download_column.retry);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.category,
			setting->download_column.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.url,
			setting->download_column.url);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.added_on,
			setting->download_column.added_on);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) ugtk->menubar.view.columns.completed_on,
			setting->download_column.completed_on);
}

void	uget_gtk_get_setting (UgetGtk* ugtk, UgetGtkSetting* setting)
{
	GdkWindowState	gdk_wstate;
	GdkWindow*		gdk_window;

	// get window position, size, and maximzied state
	if (gtk_widget_get_visible (GTK_WIDGET (ugtk->window.self)) == TRUE) {
		gdk_window = gtk_widget_get_window (GTK_WIDGET (ugtk->window.self));
		gdk_wstate = gdk_window_get_state (gdk_window);

		if (gdk_wstate & GDK_WINDOW_STATE_MAXIMIZED)
			setting->window.maximized = TRUE;
		else
			setting->window.maximized = FALSE;
		// get geometry
		if (setting->window.maximized == FALSE) {
			gtk_window_get_position (ugtk->window.self,
					&setting->window.x, &setting->window.y);
			gtk_window_get_size (ugtk->window.self,
					&setting->window.width, &setting->window.height);
		}
	}
}

// -------------------------------------------------------
// UgetGtkClipboard
void	uget_gtk_clipboard_init (struct UgetGtkClipboard* clipboard, const gchar* pattern)
{
	clipboard->self  = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	clipboard->text  = NULL;
	clipboard->regex = g_regex_new (pattern, G_REGEX_CASELESS, 0, NULL);
}

void	uget_gtk_clipboard_set_pattern (struct UgetGtkClipboard* clipboard, const gchar* pattern)
{
	if (clipboard->regex)
		g_regex_unref (clipboard->regex);
	clipboard->regex = g_regex_new (pattern, G_REGEX_CASELESS, 0, NULL);
}

void	uget_gtk_clipboard_set_text (struct UgetGtkClipboard* clipboard, gchar* text)
{
	g_free (clipboard->text);
	clipboard->text = text;
	gtk_clipboard_set_text (clipboard->self, text, -1);
}

GList*	uget_gtk_clipboard_get_uris (struct UgetGtkClipboard* clipboard)
{
	GList*		list;
	gchar*		text;

	if (gtk_clipboard_wait_is_text_available (clipboard->self) == FALSE)
		return NULL;
	text = gtk_clipboard_wait_for_text (clipboard->self);
	if (text == NULL)
		return NULL;
	// get URIs that scheme is not "file" from text
	list = ug_text_get_uris (text, -1);
	list = ug_uri_list_remove_scheme (list, "file");
	g_free (text);

	return list;
}

GList*	uget_gtk_clipboard_get_matched (struct UgetGtkClipboard* clipboard, const gchar* text)
{
	GList*		link;
	GList*		list;
	gchar*		temp;

	if (text == NULL) {
		g_free (clipboard->text);
		clipboard->text = NULL;
		return NULL;
	}
	// compare
	temp = (clipboard->text) ? clipboard->text : "";
	if (g_ascii_strcasecmp (text, temp) == 0)
		return NULL;
	// replace text
	g_free (clipboard->text);
	clipboard->text = g_strdup (text);
	// get and filter list
	list = ug_text_get_uris (text, -1);
	list = ug_uri_list_remove_scheme (list, "file");
	// filter by filename extension
	for (link = list;  link;  link = link->next) {
		temp = ug_uri_get_filename (link->data);
		// get filename extension
		if (temp)
			text = strrchr (temp, '.');
		else
			text = NULL;
		// free URIs if not matched
		if (text == NULL || g_regex_match (clipboard->regex, text+1, 0, NULL) == FALSE) {
			g_free (link->data);
			link->data = NULL;
		}
		g_free (temp);
	}
	list = g_list_remove_all (list, NULL);
	return list;
}

// -------------------------------------------------------
// utility functions
//
void	uget_gtk_close_window (UgetGtk* ugtk)
{
	switch (ugtk->setting.ui.close_action)
	{
	default:
	case 1:		// Minimize to tray.
		gtk_window_iconify (ugtk->window.self);
		gtk_widget_hide ((GtkWidget*) ugtk->window.self);
		break;

	case 2:		// Exit.
		uget_gtk_quit (ugtk);
		break;
	}
}

// confirmation dialog
struct UgConfirmationDialog {
//	GtkWidget*			self;
	GtkToggleButton*	confirmation;
	GtkToggleButton*	setting;
	UgetGtk*			ugtk;
};

static void	on_confirm_to_quit_response (GtkWidget* dialog, gint response, struct UgConfirmationDialog* ucd)
{
	UgetGtk*	ugtk;

	ugtk = ucd->ugtk;
	ugtk->dialogs.close_confirmation = NULL;
	if (response == GTK_RESPONSE_YES) {
		// window close setting
		if (gtk_toggle_button_get_active (ucd->setting) == FALSE)
			ugtk->setting.ui.close_action = 1;		// Minimize to tray.
		else
			ugtk->setting.ui.close_action = 2;		// Exit.
		// window close confirmation
		if (gtk_toggle_button_get_active (ucd->confirmation) == FALSE)
			ugtk->setting.ui.close_confirmation = TRUE;
		else
			ugtk->setting.ui.close_confirmation = FALSE;
		// minimize to tray or exit when user close window.
		uget_gtk_close_window (ugtk);
	}
	gtk_widget_destroy (dialog);
	g_free (ucd);
}

void	uget_gtk_confirm_to_quit (UgetGtk* ugtk)
{
	struct UgConfirmationDialog*	ucd;
	GtkWidget*	dialog;
	GtkWidget*	button;
	GtkBox*		hbox;
	GtkBox*		vbox;
	gchar*		title;

	// show previous message dialog
	if (ugtk->dialogs.close_confirmation) {
		gtk_window_present ((GtkWindow*) ugtk->dialogs.close_confirmation);
		return;
	}
	// create confirmation dialog
	title = g_strconcat (UGET_GTK_NAME " - ", _("Really Quit?"), NULL);
	dialog = gtk_dialog_new_with_buttons (title, ugtk->window.self,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_NO,  GTK_RESPONSE_NO,
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			NULL);
	g_free (title);
	ugtk->dialogs.close_confirmation = dialog;
	ucd = g_malloc (sizeof (struct UgConfirmationDialog));
//	ucd->self = dialog;
	ucd->ugtk = ugtk;
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 4);
	vbox = (GtkBox*) gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	// image and label
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (hbox, gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG),
	                    FALSE, FALSE, 8);
	gtk_box_pack_start (hbox, gtk_label_new (_("Are you sure you want to quit?")),
	                    FALSE, FALSE, 4);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 6);
	// check button
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	button = gtk_check_button_new_with_label (_("Remember this action"));
	gtk_box_pack_end (hbox, button, TRUE, TRUE, 20);
	gtk_box_pack_end (vbox, (GtkWidget*) hbox, FALSE, FALSE, 10);
	ucd->confirmation = (GtkToggleButton*) button;
	// radio button
	button = gtk_radio_button_new_with_label_from_widget (NULL, _("Minimize to tray"));
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 1);
	button = gtk_radio_button_new_with_label_from_widget ((GtkRadioButton*) button, _("Exit uGet"));
	gtk_box_pack_start (vbox, button, FALSE, FALSE, 1);
	ucd->setting = (GtkToggleButton*) button;
	// config
	if (ugtk->setting.ui.close_action == 0)
		gtk_toggle_button_set_active (ucd->confirmation, TRUE);
	else if (ugtk->setting.ui.close_confirmation == FALSE)
		gtk_toggle_button_set_active (ucd->confirmation, TRUE);
	else
		gtk_toggle_button_set_active (ucd->confirmation, FALSE);
//	if (ugtk->setting.ui.close_action == 1)
//		gtk_toggle_button_set_active (ucd->setting, TRUE);
	if (ugtk->setting.ui.close_action == 2)
		gtk_toggle_button_set_active ((GtkToggleButton*) button, TRUE);

	g_signal_connect (dialog, "response",
			G_CALLBACK (on_confirm_to_quit_response), ucd);
	gtk_widget_show_all ((GtkWidget*) dialog);
}

// delete confirmation dialog
static void	on_confirm_to_delete_response (GtkWidget* dialog, gint response, struct UgConfirmationDialog* ucd)
{
	UgetGtk*	ugtk;

	ugtk = ucd->ugtk;
	ugtk->dialogs.delete_confirmation = NULL;
	if (response == GTK_RESPONSE_YES) {
		// delete confirmation
		if (gtk_toggle_button_get_active (ucd->confirmation) == FALSE)
			ugtk->setting.ui.delete_confirmation = TRUE;
		else
			ugtk->setting.ui.delete_confirmation = FALSE;
	}
	// refresh
	gtk_widget_set_sensitive ((GtkWidget*) ugtk->window.self, TRUE);
	gtk_widget_destroy (dialog);
	g_free (ucd);
}

void	uget_gtk_confirm_to_delete (UgetGtk* ugtk, GCallback response, gpointer response_data)
{
	struct UgConfirmationDialog*	ucd;
	GtkWidget*	dialog;
	GtkWidget*	button;
	GtkBox*		hbox;
	GtkBox*		vbox;
	gchar*		title;

	// show previous message dialog
	if (ugtk->dialogs.delete_confirmation) {
		gtk_window_present ((GtkWindow*) ugtk->dialogs.delete_confirmation);
		return;
	}
	// create confirmation dialog
	title = g_strconcat (UGET_GTK_NAME " - ", _("Really delete files?"), NULL);
	dialog = gtk_dialog_new_with_buttons (title, ugtk->window.self,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_NO,  GTK_RESPONSE_NO,
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			NULL);
	g_free (title);
	ugtk->dialogs.delete_confirmation = dialog;
	ucd = g_malloc (sizeof (struct UgConfirmationDialog));
//	ucd->self = dialog;
	ucd->ugtk = ugtk;

	gtk_container_set_border_width (GTK_CONTAINER (dialog), 4);
	vbox = (GtkBox*) gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	// image and label
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (hbox, gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG),
	                    FALSE, FALSE, 8);
	gtk_box_pack_start (hbox, gtk_label_new (_("Are you sure you want to delete files?")),
	                    FALSE, FALSE, 4);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 6);
	// check button
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	button = gtk_check_button_new_with_label (_("Do not ask me again"));
	gtk_box_pack_end (hbox, button, TRUE, TRUE, 20);
	gtk_box_pack_end (vbox, (GtkWidget*) hbox, FALSE, FALSE, 10);
	ucd->confirmation = (GtkToggleButton*) button;
	// config
	if (ugtk->setting.ui.delete_confirmation == FALSE)
		gtk_toggle_button_set_active ((GtkToggleButton*) button, TRUE);
	else
		gtk_toggle_button_set_active ((GtkToggleButton*) button, FALSE);

	g_signal_connect (dialog, "response",
			G_CALLBACK (response), response_data);
	g_signal_connect_after (dialog, "response",
			G_CALLBACK (on_confirm_to_delete_response), ucd);
	gtk_widget_set_sensitive ((GtkWidget*) ugtk->window.self, FALSE);
	gtk_widget_show_all ((GtkWidget*) dialog);
}

// message dialog
static void	on_message_response (GtkWidget* dialog, gint response, GtkWidget** value)
{
	gtk_widget_destroy (dialog);
	*value = NULL;
}

void	uget_gtk_show_message (UgetGtk* ugtk, GtkMessageType type, const gchar* message)
{
	GtkWidget*		dialog;
	GtkWidget**		value;
	gchar*			title;

	dialog = gtk_message_dialog_new (ugtk->window.self,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			type, GTK_BUTTONS_OK,
			"%s", message);
	// set title
	switch (type) {
	case GTK_MESSAGE_ERROR:
		if (ugtk->dialogs.error)
			gtk_widget_destroy (ugtk->dialogs.error);
		ugtk->dialogs.error = dialog;
		value = &ugtk->dialogs.error;
		title = g_strconcat (UGET_GTK_NAME " - ", _("Error"), NULL);
		break;
	default:
		if (ugtk->dialogs.message)
			gtk_widget_destroy (ugtk->dialogs.message);
		ugtk->dialogs.message = dialog;
		value = &ugtk->dialogs.message;
		title = g_strconcat (UGET_GTK_NAME " - ", _("Message"), NULL);
	}
	gtk_window_set_title ((GtkWindow*) dialog, title);
	g_free (title);
	// signal handler
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_message_response), value);
	gtk_widget_show (dialog);
}

// -------------------------------------------------------
// Functions are used to refresh status and data.
//
void	on_move_download (GtkWidget* widget, UgetGtk* ugtk)
{
	GPtrArray*			array;
	UgDownloadWidget*	dwidget;
	UgCategory*			category;
	GList*				list;
	GList*				link;
	guint				index;

	array = ugtk->menubar.download.move_to.array;
	category = NULL;
	for (index = 0;  index < array->len;  index += 2) {
		if (widget == g_ptr_array_index (array, index)) {
			category = g_ptr_array_index (array, index + 1);
			break;
		}
	}

	if (category == NULL || category == ugtk->cwidget.current.category)
		return;
	list = ug_download_widget_get_selected (ugtk->cwidget.current.widget);
	for (link = list;  link;  link = link->next)
		ug_category_gtk_move_to (ugtk->cwidget.current.category, link->data, category);
	g_list_free (list);
	// refresh
	dwidget = ugtk->cwidget.current.widget;
	gtk_widget_queue_draw (ugtk->cwidget.self);
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&ugtk->summary, ug_download_widget_get_cursor (dwidget));
}

void	uget_gtk_move_menu_refresh (struct UgetGtkMenubar* menubar, UgetGtk* ugtk, gboolean reset)
{
	UgCategory*		category;
	GtkWidget*		menu_item;
	GtkWidget*		image;
	GPtrArray*		array;
	GtkTreeModel*	model;
	GtkTreeIter		iter;
	gboolean		valid;
	guint			index;

	array = menubar->download.move_to.array;
	model = GTK_TREE_MODEL (ugtk->cwidget.store);

	if (reset) {
		// remove all item
		for (index = 0;  index < array->len;  index += 2) {
			menu_item = g_ptr_array_index (array, index);
			gtk_container_remove ((GtkContainer*) menubar->download.move_to.self, menu_item);
		}
		g_ptr_array_set_size (array, 0);
		// add new item
		valid = gtk_tree_model_get_iter_first (model, &iter);
		for (index = 0;  valid;  index += 2) {
			gtk_tree_model_get (model, &iter, 0, &category, -1);
			valid = gtk_tree_model_iter_next (model, &iter);
			// create menu item
			menu_item = gtk_image_menu_item_new_with_label (category->name);
			image = gtk_image_new_from_stock (UGET_GTK_CATEGORY_STOCK, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image ((GtkImageMenuItem*) menu_item, image);
			gtk_menu_shell_append (GTK_MENU_SHELL (menubar->download.move_to.self), menu_item);
			g_signal_connect (menu_item, "activate",
					G_CALLBACK (on_move_download), ugtk);
			gtk_widget_show (menu_item);
			g_ptr_array_add (array, menu_item);
			g_ptr_array_add (array, category);
		}
	}

	// set sensitive
	for (index = 0;  index < array->len;  index += 2) {
		menu_item = g_ptr_array_index (array, index);
		category  = g_ptr_array_index (array, index +1);
		if (category == ugtk->cwidget.current.category)
			gtk_widget_set_sensitive (menu_item, FALSE);
		else
			gtk_widget_set_sensitive (menu_item, TRUE);
	}
}

void	uget_gtk_tray_icon_refresh (struct UgetGtkTrayIcon* icon, guint n_active, gdouble speed)
{
	gchar*	string;
	gchar*	string_speed;
	guint	current_status;

	// change tray icon
	if (icon->error_occurred) {
		string = UGET_GTK_ICON_ERROR_NAME;
		current_status = 2;
	}
	else if (n_active > 0) {
		string = UGET_GTK_ICON_ACTIVE_NAME;
		current_status = 1;
	}
	else {
		string = UGET_GTK_ICON_NAME;
		current_status = 0;
	}

	if (icon->last_status != current_status) {
		icon->last_status  = current_status;
		gtk_status_icon_set_from_icon_name (icon->self, string);
	}
	// change tooltip
	string_speed = ug_str_dtoa_unit (speed, 1, "/s");
	string = g_strdup_printf (
			UGET_GTK_NAME " " UGET_GTK_VERSION "\n"
			"%u %s" "\n"
			"%s",
			n_active, _("downloading"),
			string_speed);
	gtk_status_icon_set_tooltip_text (icon->self, string);
	g_free (string_speed);
	g_free (string);
}

void	uget_gtk_statusbar_refresh (struct UgetGtkStatusbar* statusbar, UgDownloadWidget* dwidget)
{
	static guint	context_id = 0;
	gint			n_selected;
	gchar*			string;

	if (context_id == 0)
		context_id = gtk_statusbar_get_context_id (statusbar->self, "selected");
	gtk_statusbar_pop  (statusbar->self, context_id);

	if (dwidget) {
		n_selected = ug_download_widget_count_selected (dwidget);
		string = g_strdup_printf (_("Selected %d items"), n_selected);
		gtk_statusbar_push (statusbar->self, context_id, string);
		g_free (string);
	}
}

void	uget_gtk_statusbar_refresh_speed (struct UgetGtkStatusbar* statusbar, gdouble speed)
{
	gchar*			string;

	string = ug_str_dtoa_unit (speed, 1, "/s");
	gtk_label_set_text (statusbar->speed, string);
	g_free (string);
}

void	uget_gtk_refresh_download_column (UgetGtk* ugtk)
{
	struct UgDownloadColumnSetting*	setting;
	GtkTreeViewColumn*	column;
	UgDownloadWidget*	dwidget;
	UgCategoryGtk*		cgtk;
	gboolean			sensitive;

	// ----------------------------------------------------
	// set UgetGtkViewMenu sensitive
	cgtk = ugtk->cwidget.current.cgtk;
	dwidget = ugtk->cwidget.current.widget;
	// Finished
	if (dwidget == &cgtk->finished)
		sensitive = FALSE;
	else
		sensitive = TRUE;
	gtk_widget_set_sensitive (ugtk->menubar.view.columns.completed, sensitive);
	gtk_widget_set_sensitive (ugtk->menubar.view.columns.percent, sensitive);
	// Recycled
	if (dwidget == &cgtk->recycled)
		sensitive = FALSE;
	else
		sensitive = TRUE;
	gtk_widget_set_sensitive (ugtk->menubar.view.columns.elapsed, sensitive);
	// Finished & Recycled
	if (dwidget == &cgtk->finished  ||  dwidget == &cgtk->recycled)
		sensitive = FALSE;
	else
		sensitive = TRUE;
	gtk_widget_set_sensitive (ugtk->menubar.view.columns.left, sensitive);
	gtk_widget_set_sensitive (ugtk->menubar.view.columns.speed, sensitive);

	// ----------------------------------------------------
	// set download column visible
	setting = &ugtk->setting.download_column;
	// changed_count set in  on_change_visible_column()
	if (dwidget->changed_count != setting->changed_count) {
		dwidget->changed_count =  setting->changed_count;
		if (dwidget != &cgtk->finished) {
			column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_COMPLETE);
			gtk_tree_view_column_set_visible (column, setting->completed);
			column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_PERCENT);
			gtk_tree_view_column_set_visible (column, setting->percent);
		}
		if (dwidget != &cgtk->recycled) {
			column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_ELAPSED);
			gtk_tree_view_column_set_visible (column, setting->elapsed);
		}
		if (dwidget != &cgtk->finished  &&  dwidget != &cgtk->recycled) {
			column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_LEFT);
			gtk_tree_view_column_set_visible (column, setting->left);
			column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_SPEED);
			gtk_tree_view_column_set_visible (column, setting->speed);
		}
		column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_SIZE);
		gtk_tree_view_column_set_visible (column, setting->total);
		column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_RETRY);
		gtk_tree_view_column_set_visible (column, setting->retry);
		column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_CATEGORY);
		gtk_tree_view_column_set_visible (column, setting->category);
		column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_URL);
		gtk_tree_view_column_set_visible (column, setting->url);
		column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_ADDED_ON);
		gtk_tree_view_column_set_visible (column, setting->added_on);
		column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_COMPLETED_ON);
		gtk_tree_view_column_set_visible (column, setting->completed_on);
	}
}

void	uget_gtk_refresh_download_sensitive (UgetGtk* ugtk)
{
	UgDownloadWidget*	dwidget;
	UgCategoryGtk*		cgtk;
	gboolean			sensitive = FALSE;
	static gboolean		last_sensitive = TRUE;

	cgtk = ugtk->cwidget.current.cgtk;
	dwidget = ugtk->cwidget.current.widget;
	if (ug_download_widget_count_selected (dwidget) > 0)
		sensitive = TRUE;

	// change sensitive after select/unselect
	if (last_sensitive != sensitive) {
		last_sensitive =  sensitive;
		gtk_widget_set_sensitive (ugtk->toolbar.runnable, sensitive);
		gtk_widget_set_sensitive (ugtk->toolbar.pause, sensitive);
		gtk_widget_set_sensitive (ugtk->toolbar.properties, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.download.open, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.download.open_folder, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.download.delete, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.download.delete_file, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.download.runnable, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.download.pause, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.download.move_to.item, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.download.properties, sensitive);
	}

	// Move Up/Down/Top/Bottom functions need reset sensitive when selection changed.
	// These need by  on_move_download_xxx()  series.
	if (dwidget == &cgtk->all || dwidget == &cgtk->active)
		sensitive = FALSE;
	gtk_widget_set_sensitive (ugtk->toolbar.move_up, sensitive);
	gtk_widget_set_sensitive (ugtk->toolbar.move_down, sensitive);
	gtk_widget_set_sensitive (ugtk->toolbar.move_top, sensitive);
	gtk_widget_set_sensitive (ugtk->toolbar.move_bottom, sensitive);
	gtk_widget_set_sensitive (ugtk->menubar.download.move_up, sensitive);
	gtk_widget_set_sensitive (ugtk->menubar.download.move_down, sensitive);
	gtk_widget_set_sensitive (ugtk->menubar.download.move_top, sensitive);
	gtk_widget_set_sensitive (ugtk->menubar.download.move_bottom, sensitive);
}

void	uget_gtk_refresh_category_sensitive (UgetGtk* ugtk)
{
	static gboolean		last_sensitive = TRUE;
	gboolean			sensitive;

	if (ugtk->cwidget.current.category)
		sensitive = TRUE;
	else
		sensitive = FALSE;

	if (last_sensitive != sensitive) {
		last_sensitive = sensitive;
		gtk_widget_set_sensitive (ugtk->menubar.category.properties, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.view.columns.self, sensitive);
	}
	if (ugtk->cwidget.current.category == ugtk->cwidget.primary.category) {
		gtk_widget_set_sensitive (ugtk->menubar.category.delete, FALSE);
		gtk_widget_set_sensitive (ugtk->menubar.category.properties, FALSE);
	}
	else {
		gtk_widget_set_sensitive (ugtk->menubar.category.delete, sensitive);
		gtk_widget_set_sensitive (ugtk->menubar.category.properties, sensitive);
	}

	uget_gtk_refresh_download_sensitive (ugtk);
}

