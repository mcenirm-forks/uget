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
#include <UgRegistry.h>
#include <UgetData.h>
#include <UgApp-gtk.h>
#include <curl/curl.h>

#include <glib/gi18n.h>

const char*	ug_get_attachment_dir (void)
{
	static const char*	dir = NULL;

	if (dir == NULL) {
		dir = g_build_filename (g_get_user_config_dir (),
				UG_APP_GTK_DIR, "attachment", NULL);
	}
	return dir;
}

static void ug_app_update_config_dir (void)
{
	gchar*	former_dir;
	gchar*	dir;

	former_dir = g_build_filename (g_get_user_config_dir (),
			"Uget", NULL);
	dir = g_filename_from_utf8 (former_dir, -1, NULL, NULL, NULL);
	if (g_file_test (dir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
		g_free (dir);
		dir = g_build_filename (g_get_user_config_dir (),
				UG_APP_GTK_DIR, NULL);
		ug_rename (former_dir, dir);
	}

	g_free (former_dir);
	g_free (dir);
}

void	ug_app_init (UgAppGtk* app)
{
	UgCategory*		category;
	UgetCommon*	common;

	ug_running_init (&app->running);
	// upgrade from Uget 1.6
	ug_app_update_config_dir ();
	// initialize widgets in UgApp-gtk-gui.c
	ug_app_init_gui (app);
	// initialize settings
	ug_setting_init (&app->setting);
	// load settings & data
	ug_attachment_init (ug_get_attachment_dir ());
	ug_app_load (app);
	ug_attachment_sync ();
	// apply settings
	ug_app_set_setting (app, &app->setting);
	ug_clipboard_init (&app->clipboard, app->setting.clipboard.pattern);
	app->launch_regex = g_regex_new (app->setting.launch.types,
			G_REGEX_CASELESS, 0, NULL);
	// If no category exists, create new one.
	if (ug_category_widget_n_category (&app->cwidget) == 0) {
		category = ug_category_new_with_gtk (app->cwidget.primary.category);
		category->name = g_strdup ("Home");
		common = ug_dataset_alloc_front (category->defaults, UgetCommonInfo);
		common->folder = g_strdup (g_get_home_dir ());
		ug_category_widget_append (&app->cwidget, category);
	}
	// initialize signal handlers in UgApp-gtk-callback.c
	ug_app_init_callback (app);
	// initialize timeout in UgApp-gtk-timeout.c
	ug_app_init_timeout (app);
	// initialize aria2
	ug_app_aria2_init (app);
	ug_app_aria2_setup (app);
	// get update info
	ug_app_get_update_info (app);

	ug_app_menubar_sync_category (app, TRUE);
	ug_category_view_set_cursor (app->cwidget.primary.view, 0, -1);
	gtk_window_set_focus (app->window.self,
			(GtkWidget*) app->cwidget.primary.view);

	// set user interface
	if (app->setting.ui.start_in_offline_mode)
	    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (app->trayicon.menu.offline_mode), TRUE);
	if (app->setting.ui.start_in_tray == FALSE)
		gtk_widget_show ((GtkWidget*) app->window.self);
	ug_app_trayicon_decide_visible (app);
}

void	ug_app_quit (UgAppGtk* app)
{
	// stop all active download
	ug_running_clear (&app->running);
	// get and update setting before program save it
	ug_app_get_setting (app, &app->setting);
	// save data
	ug_app_save (app);
	// aria2
	ug_app_aria2_finalize (app);
	// hide icon in system tray before quit
	ug_trayicon_set_visible (&app->trayicon, FALSE);
	// hide window
	gtk_widget_hide (GTK_WIDGET (app->window.self));
	// clear update info
	ug_app_clear_update_info (app);
	// aria2
	ug_app_aria2_shutdown (app);
	// This will quit gtk_main() in main()
	gtk_main_quit ();
}

void	ug_app_save (UgAppGtk* app)
{
	GList*			list;
	gchar*			file;

	// save setting
	file = g_build_filename (g_get_user_config_dir (),
			UG_APP_GTK_DIR, UG_APP_GTK_SETTING_FILE, NULL);
	ug_setting_save (&app->setting, file);
	g_free (file);
	// get and save all tasks from primary category
	list = ug_category_gtk_get_all (app->cwidget.primary.category);
	file = g_build_filename (g_get_user_config_dir (),
			UG_APP_GTK_DIR, UG_APP_GTK_DOWNLOAD_FILE, NULL);
	ug_download_list_save (list, file);
	g_list_free (list);
	g_free (file);
	// save categories
	file = g_build_filename (g_get_user_config_dir (),
			UG_APP_GTK_DIR, UG_APP_GTK_CATEGORY_FILE, NULL);
	list = ug_category_widget_get_list (&app->cwidget);
	ug_category_list_save (list, file);
	g_list_free (list);
	g_free (file);
}

void	ug_app_load (UgAppGtk* app)
{
	GList*			category_list;
	GList*			download_list;
	GList*			link;
	gchar*			file;

	// load setting
	file = g_build_filename (g_get_user_config_dir (),
			UG_APP_GTK_DIR, UG_APP_GTK_SETTING_FILE, NULL);
	ug_setting_load (&app->setting, file);
	g_free (file);
	// load all tasks from file
	file = g_build_filename (g_get_user_config_dir (),
			UG_APP_GTK_DIR, UG_APP_GTK_DOWNLOAD_FILE, NULL);
	download_list = ug_download_list_load (file);
	g_free (file);
	// load all categories
	file = g_build_filename (g_get_user_config_dir (),
			UG_APP_GTK_DIR, UG_APP_GTK_CATEGORY_FILE, NULL);
	category_list = ug_category_list_load (file);
	g_free (file);
	// set primary category
	for (link = category_list;  link;  link = link->next)
		ug_category_use_gtk (link->data, app->cwidget.primary.category);
	// add tasks to primary category
	for (link = download_list;  link;  link = link->next)
		ug_category_add (app->cwidget.primary.category, link->data);
	// link and add tasks to categories
	ug_category_list_link (category_list, download_list);
	ug_category_widget_add_list (&app->cwidget, category_list);
	g_list_free (category_list);
	// free unused tasks and list
	g_list_foreach (download_list, (GFunc) ug_dataset_unref, NULL);
	g_list_free (download_list);
}

void	ug_app_set_setting (UgAppGtk* app, UgSetting* setting)
{
	// set window position, size, and maximized state
	if (setting->window.width  > 0 &&
	    setting->window.height > 0 &&
	    setting->window.x < gdk_screen_width ()  &&
	    setting->window.y < gdk_screen_height () &&
	    setting->window.x + setting->window.width > 0  &&
	    setting->window.y + setting->window.height > 0)
	{
		gtk_window_move (app->window.self,
				setting->window.x, setting->window.y);
		gtk_window_resize (app->window.self,
				setting->window.width, setting->window.height);
	}
	if (setting->window.maximized)
		gtk_window_maximize (app->window.self);
	// set visible widgets
	gtk_widget_set_visible (app->toolbar.self,
			setting->window.toolbar);
	gtk_widget_set_visible ((GtkWidget*) app->statusbar.self,
			setting->window.statusbar);
	gtk_widget_set_visible (app->cwidget.self,
			setting->window.category);
	gtk_widget_set_visible (app->summary.self,
			setting->window.summary);
	gtk_widget_set_visible (app->banner.self,
			setting->window.banner);
	// Summary
	app->summary.visible.name     = setting->summary.name;
	app->summary.visible.folder   = setting->summary.folder;
	app->summary.visible.category = setting->summary.category;
	app->summary.visible.url      = setting->summary.url;
	app->summary.visible.message  = setting->summary.message;
	// ----------------------------------------------------
	// UgEditMenu
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.edit.clipboard_monitor,
			setting->clipboard.monitor);
	// ----------------------------------------------------
	// UgViewMenu
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.toolbar,
			setting->window.toolbar);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.statusbar,
			setting->window.statusbar);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.category,
			setting->window.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary,
			setting->window.summary);
	// summary items
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.name,
			setting->summary.name);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.folder,
			setting->summary.folder);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.category,
			setting->summary.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.url,
			setting->summary.url);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.summary_items.message,
			setting->summary.message);
	// download column
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.completed,
			setting->download_column.completed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.total,
			setting->download_column.total);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.percent,
			setting->download_column.percent);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.elapsed,
			setting->download_column.elapsed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.left,
			setting->download_column.left);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.speed,
			setting->download_column.speed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.upload_speed,
			setting->download_column.upload_speed);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.uploaded,
			setting->download_column.uploaded);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.ratio,
			setting->download_column.ratio);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.retry,
			setting->download_column.retry);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.category,
			setting->download_column.category);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.url,
			setting->download_column.url);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.added_on,
			setting->download_column.added_on);
	gtk_check_menu_item_set_active (
			(GtkCheckMenuItem*) app->menubar.view.columns.completed_on,
			setting->download_column.completed_on);

#ifdef HAVE_APP_INDICATOR
	// AppIndicator
	if (setting->ui.app_indicator == FALSE)
		app->trayicon.indicator = NULL;
#endif	// HAVE_APP_INDICATOR

	// get setting of download column
	app->cwidget.sort.nth   = app->setting.download_column.sort.nth;
	app->cwidget.sort.order = app->setting.download_column.sort.order;

	// aria2
	ug_app_decide_bt_meta_sensitive (app);
}

void	ug_app_get_setting (UgAppGtk* app, UgSetting* setting)
{
	GdkWindowState	gdk_wstate;
	GdkWindow*		gdk_window;

	// get window position, size, and maximzied state
	if (gtk_widget_get_visible (GTK_WIDGET (app->window.self)) == TRUE) {
		gdk_window = gtk_widget_get_window (GTK_WIDGET (app->window.self));
		gdk_wstate = gdk_window_get_state (gdk_window);

		if (gdk_wstate & GDK_WINDOW_STATE_MAXIMIZED)
			setting->window.maximized = TRUE;
		else
			setting->window.maximized = FALSE;
		// get geometry
		if (setting->window.maximized == FALSE) {
			gtk_window_get_position (app->window.self,
					&setting->window.x, &setting->window.y);
			gtk_window_get_size (app->window.self,
					&setting->window.width, &setting->window.height);
		}
	}

	// get setting of download column
	if (app->cwidget.current.widget == &app->cwidget.current.cgtk->all) {
		app->setting.download_column.sort.nth   = app->cwidget.current.widget->sort.nth;
		app->setting.download_column.sort.order = app->cwidget.current.widget->sort.order;
	}
	else {
		app->setting.download_column.sort.nth   = app->cwidget.sort.nth;
		app->setting.download_column.sort.order = app->cwidget.sort.order;
	}

	// banner
	setting->window.banner = gtk_widget_get_visible (app->banner.self);
}

// ------------------------------------

static size_t  write_update_info (void *ptr, size_t size, size_t nmemb, GString* buf)
{
	if (buf->len > 4096)
		return 0;
	g_string_append_len (buf, ptr, size * nmemb);
	return nmemb;
}

static void  get_update_info_thread (UgAppGtk* app)
{
	CURL*		curl;
	CURLcode	res;
	long		response_code = 0;

	curl = curl_easy_init();
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION,
			(curl_write_callback) write_update_info);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, app->update_info.text);
	curl_easy_setopt (curl, CURLOPT_URL, "http://ugetdm.com/versioncheck?v=" PACKAGE_VERSION);
	curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1);
	res = curl_easy_perform (curl);
	curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &response_code);
	curl_easy_cleanup (curl);

	if (res == CURLE_OK && response_code < 400)
		app->update_info.ready = TRUE;
	app->update_info.thread = NULL;
}

void ug_app_get_update_info (UgAppGtk* app)
{
	if (app->update_info.thread == NULL) {
		if (app->update_info.text == NULL)
			app->update_info.text = g_string_sized_new (2048);
		g_string_set_size (app->update_info.text, 0);
		app->update_info.ready = FALSE;
		app->update_info.thread = g_thread_new ("get update info",
				(GThreadFunc) get_update_info_thread, app);
		g_thread_unref (app->update_info.thread);
	}
}

void  ug_app_clear_update_info (UgAppGtk* app)
{
	if (app->update_info.thread == NULL) {
		if (app->update_info.text)
			g_string_free (app->update_info.text, TRUE);
		app->update_info.text = NULL;
		app->update_info.ready = FALSE;
	}
}

// -------------------------------------------------------
// UgClipboard

void	ug_clipboard_init (struct UgClipboard* clipboard, const gchar* pattern)
{
	clipboard->self  = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
	clipboard->text  = NULL;
	clipboard->regex = g_regex_new (pattern, G_REGEX_CASELESS, 0, NULL);
}

void	ug_clipboard_set_pattern (struct UgClipboard* clipboard, const gchar* pattern)
{
	if (clipboard->regex)
		g_regex_unref (clipboard->regex);
	clipboard->regex = g_regex_new (pattern, G_REGEX_CASELESS, 0, NULL);
}

void	ug_clipboard_set_text (struct UgClipboard* clipboard, gchar* text)
{
	g_free (clipboard->text);
	clipboard->text = text;
	gtk_clipboard_set_text (clipboard->self, text, -1);
}

GList*	ug_clipboard_get_uris (struct UgClipboard* clipboard)
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

GList*	ug_clipboard_get_matched (struct UgClipboard* clipboard, const gchar* text)
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
// UgTrayIcon and UgStatusbar

void	ug_trayicon_set_info (struct UgTrayIcon* trayicon, guint n_active, gint64 down_speed, gint64 up_speed)
{
	gchar*	string;
	gchar*	string_down_speed;
	gchar*	string_up_speed;
	guint	current_status;

	// change tray icon
	if (trayicon->error_occurred) {
		string = UG_APP_GTK_TRAY_ICON_ERROR_NAME;
		current_status = 2;
	}
	else if (n_active > 0) {
		string = UG_APP_GTK_TRAY_ICON_ACTIVE_NAME;
		current_status = 1;
	}
	else {
		string = UG_APP_GTK_TRAY_ICON_NAME;
		current_status = 0;
	}

	if (trayicon->last_status != current_status) {
		trayicon->last_status  = current_status;
#ifdef HAVE_APP_INDICATOR
		if (trayicon->indicator) {
			trayicon->error_occurred = FALSE;
			if (app_indicator_get_status (trayicon->indicator) != APP_INDICATOR_STATUS_PASSIVE) {
				if (current_status == 0) {
					app_indicator_set_status (trayicon->indicator,
							APP_INDICATOR_STATUS_ACTIVE);
				}
				else {
					app_indicator_set_attention_icon (trayicon->indicator, string);
	//				app_indicator_set_attention_icon_full (trayicon->indicator,
	//						string, "attention");
					app_indicator_set_status (trayicon->indicator,
							APP_INDICATOR_STATUS_ATTENTION);
				}
			}
		}
		else
#endif	// HAVE_APP_INDICATOR
		gtk_status_icon_set_from_icon_name (trayicon->self, string);
	}

	// change tooltip
	string_down_speed = ug_str_dtoa_unit ((gdouble) down_speed, 1, "/s");
	string_up_speed   = ug_str_dtoa_unit ((gdouble) up_speed, 1, "/s");
	string = g_strdup_printf (
			UG_APP_GTK_NAME " " UG_APP_GTK_VERSION "\n"
			"%u %s" "\n"
			"D: %s" "\n"
			"U: %s",
			n_active, _("tasks"),
			string_down_speed,
			string_up_speed);
#ifdef HAVE_APP_INDICATOR
	if (trayicon->indicator) {
//		g_object_set (trayicon->indicator, "icon-desc", string, NULL);
//		g_object_set (trayicon->indicator, "attention-icon-desc", string, NULL);
	}
	else
#endif	// HAVE_APP_INDICATOR
	gtk_status_icon_set_tooltip_text (trayicon->self, string);

	g_free (string_down_speed);
	g_free (string_up_speed);
	g_free (string);
}

void	ug_trayicon_set_visible (struct UgTrayIcon* trayicon, gboolean visible)
{
#ifdef HAVE_APP_INDICATOR
	if (trayicon->indicator) {
		if (visible)
			app_indicator_set_status (trayicon->indicator,
					APP_INDICATOR_STATUS_ACTIVE);
		else
			app_indicator_set_status (trayicon->indicator,
					APP_INDICATOR_STATUS_PASSIVE);
	}
	else
#endif	// HAVE_APP_INDICATOR
	gtk_status_icon_set_visible (trayicon->self, visible);
}

void	ug_statusbar_set_info (struct UgStatusbar* statusbar, UgDownloadWidget* dwidget)
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

void	ug_statusbar_set_speed (struct UgStatusbar* statusbar, gint64 down_speed, gint64 up_speed)
{
	gchar*		string;

	string = ug_str_dtoa_unit ((gdouble) down_speed, 1, "/s");
	gtk_label_set_text (statusbar->down_speed, string);
	g_free (string);

	string = ug_str_dtoa_unit ((gdouble) up_speed, 1, "/s");
	gtk_label_set_text (statusbar->up_speed, string);
	g_free (string);
}


// -------------------------------------------------------
// utility or integrate functions

// confirmation dialog
struct UgConfirmationDialog {
//	GtkWidget*			self;
	GtkToggleButton*	confirmation;
	GtkToggleButton*	setting;
	UgAppGtk*			app;
};

static void	on_confirm_to_quit_response (GtkWidget* dialog, gint response, struct UgConfirmationDialog* ucd)
{
	UgAppGtk*	app;

	app = ucd->app;
	app->dialogs.close_confirmation = NULL;
	if (response == GTK_RESPONSE_YES) {
		// window close setting
		if (gtk_toggle_button_get_active (ucd->setting) == FALSE)
			app->setting.ui.close_action = 1;		// Minimize to tray.
		else
			app->setting.ui.close_action = 2;		// Exit.
		// window close confirmation
		if (gtk_toggle_button_get_active (ucd->confirmation) == FALSE)
			app->setting.ui.close_confirmation = TRUE;
		else
			app->setting.ui.close_confirmation = FALSE;
		// minimize to tray or exit when user close window.
		ug_app_window_close (app);
	}
	gtk_widget_destroy (dialog);
	g_free (ucd);
}

void	ug_app_confirm_to_quit (UgAppGtk* app)
{
	struct UgConfirmationDialog*	ucd;
	GtkWidget*	dialog;
	GtkWidget*	button;
	GtkBox*		hbox;
	GtkBox*		vbox;
	gchar*		title;

	// show previous message dialog
	if (app->dialogs.close_confirmation) {
		gtk_window_present ((GtkWindow*) app->dialogs.close_confirmation);
		return;
	}
	// create confirmation dialog
	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Really Quit?"), NULL);
	dialog = gtk_dialog_new_with_buttons (title, app->window.self,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_NO,  GTK_RESPONSE_NO,
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			NULL);
	g_free (title);
	app->dialogs.close_confirmation = dialog;
	ucd = g_malloc (sizeof (struct UgConfirmationDialog));
//	ucd->self = dialog;
	ucd->app = app;
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 4);
	vbox = (GtkBox*) gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	// image and label
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (hbox, gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG),
	                    FALSE, FALSE, 8);
	gtk_box_pack_start (hbox, gtk_label_new (_("Are you sure you want to quit?")),
	                    FALSE, FALSE, 4);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 6);
	// check button
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
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
	if (app->setting.ui.close_action == 0)
		gtk_toggle_button_set_active (ucd->confirmation, TRUE);
	else if (app->setting.ui.close_confirmation == FALSE)
		gtk_toggle_button_set_active (ucd->confirmation, TRUE);
	else
		gtk_toggle_button_set_active (ucd->confirmation, FALSE);
//	if (app->setting.ui.close_action == 1)
//		gtk_toggle_button_set_active (ucd->setting, TRUE);
	if (app->setting.ui.close_action == 2)
		gtk_toggle_button_set_active ((GtkToggleButton*) button, TRUE);

	g_signal_connect (dialog, "response",
			G_CALLBACK (on_confirm_to_quit_response), ucd);
	gtk_widget_show_all ((GtkWidget*) dialog);
}

// delete confirmation dialog
static void	on_confirm_to_delete_response (GtkWidget* dialog, gint response, struct UgConfirmationDialog* ucd)
{
	UgAppGtk*	app;

	app = ucd->app;
	app->dialogs.delete_confirmation = NULL;
	if (response == GTK_RESPONSE_YES) {
		// delete confirmation
		if (gtk_toggle_button_get_active (ucd->confirmation) == FALSE)
			app->setting.ui.delete_confirmation = TRUE;
		else
			app->setting.ui.delete_confirmation = FALSE;
	}
	// refresh
	gtk_widget_set_sensitive ((GtkWidget*) app->window.self, TRUE);
	gtk_widget_destroy (dialog);
	g_free (ucd);
}

void	ug_app_confirm_to_delete (UgAppGtk* app, GCallback response, gpointer response_data)
{
	struct UgConfirmationDialog*	ucd;
	GtkWidget*	dialog;
	GtkWidget*	button;
	GtkBox*		hbox;
	GtkBox*		vbox;
	gchar*		title;

	// show previous message dialog
	if (app->dialogs.delete_confirmation) {
		gtk_window_present ((GtkWindow*) app->dialogs.delete_confirmation);
		return;
	}
	// create confirmation dialog
	title = g_strconcat (UG_APP_GTK_NAME " - ", _("Really delete files?"), NULL);
	dialog = gtk_dialog_new_with_buttons (title, app->window.self,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_NO,  GTK_RESPONSE_NO,
			GTK_STOCK_YES, GTK_RESPONSE_YES,
			NULL);
	g_free (title);
	app->dialogs.delete_confirmation = dialog;
	ucd = g_malloc (sizeof (struct UgConfirmationDialog));
//	ucd->self = dialog;
	ucd->app = app;

	gtk_container_set_border_width (GTK_CONTAINER (dialog), 4);
	vbox = (GtkBox*) gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	// image and label
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start (hbox, gtk_image_new_from_stock (GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG),
	                    FALSE, FALSE, 8);
	gtk_box_pack_start (hbox, gtk_label_new (_("Are you sure you want to delete files?")),
	                    FALSE, FALSE, 4);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 6);
	// check button
	hbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	button = gtk_check_button_new_with_label (_("Do not ask me again"));
	gtk_box_pack_end (hbox, button, TRUE, TRUE, 20);
	gtk_box_pack_end (vbox, (GtkWidget*) hbox, FALSE, FALSE, 10);
	ucd->confirmation = (GtkToggleButton*) button;
	// config
	if (app->setting.ui.delete_confirmation == FALSE)
		gtk_toggle_button_set_active ((GtkToggleButton*) button, TRUE);
	else
		gtk_toggle_button_set_active ((GtkToggleButton*) button, FALSE);

	g_signal_connect (dialog, "response",
			G_CALLBACK (response), response_data);
	g_signal_connect_after (dialog, "response",
			G_CALLBACK (on_confirm_to_delete_response), ucd);
	gtk_widget_set_sensitive ((GtkWidget*) app->window.self, FALSE);
	gtk_widget_show_all ((GtkWidget*) dialog);
}

// message dialog
static void	on_message_response (GtkWidget* dialog, gint response, GtkWidget** value)
{
	gtk_widget_destroy (dialog);
	*value = NULL;
}

void	ug_app_show_message (UgAppGtk* app, GtkMessageType type, const gchar* message)
{
	GtkWidget*		dialog;
	GtkWidget**		value;
	gchar*			title;

	dialog = gtk_message_dialog_new (app->window.self,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			type, GTK_BUTTONS_OK,
			"%s", message);
	// set title
	switch (type) {
	case GTK_MESSAGE_ERROR:
		if (app->dialogs.error)
			gtk_widget_destroy (app->dialogs.error);
		app->dialogs.error = dialog;
		value = &app->dialogs.error;
		title = g_strconcat (UG_APP_GTK_NAME " - ", _("Error"), NULL);
		break;

	default:
		if (app->dialogs.message)
			gtk_widget_destroy (app->dialogs.message);
		app->dialogs.message = dialog;
		value = &app->dialogs.message;
		title = g_strconcat (UG_APP_GTK_NAME " - ", _("Message"), NULL);
		break;
	}
	gtk_window_set_title ((GtkWindow*) dialog, title);
	g_free (title);
	// signal handler
	g_signal_connect (dialog, "response",
			G_CALLBACK (on_message_response), value);
	gtk_widget_show (dialog);
}

void	ug_app_window_close (UgAppGtk* app)
{
	switch (app->setting.ui.close_action)
	{
	default:
	case 1:		// Minimize to tray.
#ifndef HAVE_APP_INDICATOR
		// This may cause Ubuntu Unity crash
		gtk_window_iconify (app->window.self);
#endif
		gtk_widget_hide ((GtkWidget*) app->window.self);
		ug_app_trayicon_decide_visible (app);
		break;

	case 2:		// Exit.
		ug_app_quit (app);
		break;
	}
}

void	ug_app_trayicon_decide_visible (UgAppGtk* app)
{
	gboolean	visible;

	if (app->setting.ui.show_trayicon)
		visible = TRUE;
	else {
		if (gtk_widget_get_visible ((GtkWidget*) app->window.self))
			visible = FALSE;
		else
			visible = TRUE;
	}
	ug_trayicon_set_visible (&app->trayicon, visible);
}

// this function used by ug_app_menubar_sync_category()
static void	on_move_download (GtkWidget* widget, UgAppGtk* app)
{
	GPtrArray*			array;
	UgDownloadWidget*	dwidget;
	UgCategory*			category;
	GList*				list;
	GList*				link;
	guint				index;

	array = app->menubar.download.move_to.array;
	category = NULL;
	for (index = 0;  index < array->len;  index += 2) {
		if (widget == g_ptr_array_index (array, index)) {
			category = g_ptr_array_index (array, index + 1);
			break;
		}
	}

	if (category == NULL || category == app->cwidget.current.category)
		return;
	list = ug_download_widget_get_selected (app->cwidget.current.widget);
	for (link = list;  link;  link = link->next)
		ug_category_gtk_move_to (app->cwidget.current.category, link->data, category);
	g_list_free (list);
	// refresh
	dwidget = app->cwidget.current.widget;
	gtk_widget_queue_draw (app->cwidget.self);
	gtk_widget_queue_draw (GTK_WIDGET (dwidget->view));
	ug_summary_show (&app->summary, ug_download_widget_get_cursor (dwidget));
}

void	ug_app_menubar_sync_category (UgAppGtk* app, gboolean reset)
{
	UgCategory*		category;
	GtkWidget*		menu_item;
	GtkWidget*		image;
	GPtrArray*		array;
	GtkTreeModel*	model;
	GtkTreeIter		iter;
	gboolean		valid;
	guint			index;

	array = app->menubar.download.move_to.array;
	model = GTK_TREE_MODEL (app->cwidget.store);

	if (reset) {
		// remove all item
		for (index = 0;  index < array->len;  index += 2) {
			menu_item = g_ptr_array_index (array, index);
			gtk_container_remove ((GtkContainer*) app->menubar.download.move_to.self, menu_item);
		}
		g_ptr_array_set_size (array, 0);
		// add new item
		valid = gtk_tree_model_get_iter_first (model, &iter);
		for (index = 0;  valid;  index += 2) {
			gtk_tree_model_get (model, &iter, 0, &category, -1);
			valid = gtk_tree_model_iter_next (model, &iter);
			// create menu item
			menu_item = gtk_image_menu_item_new_with_label (category->name);
			image = gtk_image_new_from_stock (UG_APP_GTK_CATEGORY_STOCK, GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image ((GtkImageMenuItem*) menu_item, image);
			gtk_menu_shell_append (GTK_MENU_SHELL (app->menubar.download.move_to.self), menu_item);
			g_signal_connect (menu_item, "activate",
					G_CALLBACK (on_move_download), app);
			gtk_widget_show (menu_item);
			g_ptr_array_add (array, menu_item);
			g_ptr_array_add (array, category);
		}
	}

	// set sensitive
	for (index = 0;  index < array->len;  index += 2) {
		menu_item = g_ptr_array_index (array, index);
		category  = g_ptr_array_index (array, index +1);
		if (category == app->cwidget.current.category)
			gtk_widget_set_sensitive (menu_item, FALSE);
		else
			gtk_widget_set_sensitive (menu_item, TRUE);
	}
}

void	ug_app_reset_download_column (UgAppGtk* app)
{
	struct UgDownloadColumnSetting*	setting;
	GtkTreeViewColumn*	column;
	UgDownloadWidget*	dwidget;
	UgCategoryGtk*		cgtk;
	gboolean			sensitive;

	// ----------------------------------------------------
	// set UgViewMenu sensitive
	cgtk = app->cwidget.current.cgtk;
	dwidget = app->cwidget.current.widget;
	// Finished
	if (dwidget == &cgtk->finished)
		sensitive = FALSE;
	else
		sensitive = TRUE;
	gtk_widget_set_sensitive (app->menubar.view.columns.completed, sensitive);
	gtk_widget_set_sensitive (app->menubar.view.columns.percent, sensitive);
	// Recycled
	if (dwidget == &cgtk->recycled)
		sensitive = FALSE;
	else
		sensitive = TRUE;
	gtk_widget_set_sensitive (app->menubar.view.columns.elapsed, sensitive);
	// Finished & Recycled
	if (dwidget == &cgtk->finished  ||  dwidget == &cgtk->recycled)
		sensitive = FALSE;
	else
		sensitive = TRUE;
	gtk_widget_set_sensitive (app->menubar.view.columns.left, sensitive);
	gtk_widget_set_sensitive (app->menubar.view.columns.speed, sensitive);
	gtk_widget_set_sensitive (app->menubar.view.columns.upload_speed, sensitive);
	gtk_widget_set_sensitive (app->menubar.view.columns.uploaded, sensitive);
	gtk_widget_set_sensitive (app->menubar.view.columns.ratio, sensitive);

	// ----------------------------------------------------
	// set download column visible
	setting = &app->setting.download_column;
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
			column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_UPLOAD_SPEED);
			gtk_tree_view_column_set_visible (column, setting->upload_speed);
			column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_UPLOADED);
			gtk_tree_view_column_set_visible (column, setting->uploaded);
			column = gtk_tree_view_get_column (dwidget->view, UG_DOWNLOAD_COLUMN_RATIO);
			gtk_tree_view_column_set_visible (column, setting->ratio);
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

void	ug_app_decide_download_sensitive (UgAppGtk* app)
{
	UgDownloadWidget*	dwidget;
	UgCategoryGtk*		cgtk;
	gboolean			sensitive = FALSE;
	static gboolean		last_sensitive = TRUE;

	cgtk = app->cwidget.current.cgtk;
	dwidget = app->cwidget.current.widget;
	if (ug_download_widget_count_selected (dwidget) > 0)
		sensitive = TRUE;

	// change sensitive after select/unselect
	if (last_sensitive != sensitive) {
		last_sensitive =  sensitive;
		gtk_widget_set_sensitive (app->toolbar.runnable, sensitive);
		gtk_widget_set_sensitive (app->toolbar.pause, sensitive);
		gtk_widget_set_sensitive (app->toolbar.properties, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.open, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.open_folder, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.delete, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.delete_file, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.force_start, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.runnable, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.pause, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.move_to.item, sensitive);
		gtk_widget_set_sensitive (app->menubar.download.properties, sensitive);
	}

	// Move Up/Down/Top/Bottom functions need reset sensitive when selection changed.
	// These need by  on_move_download_xxx()  series.
	if (dwidget == &cgtk->all || dwidget == &cgtk->active)
		sensitive = FALSE;
	gtk_widget_set_sensitive (app->toolbar.move_up, sensitive);
	gtk_widget_set_sensitive (app->toolbar.move_down, sensitive);
	gtk_widget_set_sensitive (app->toolbar.move_top, sensitive);
	gtk_widget_set_sensitive (app->toolbar.move_bottom, sensitive);
	gtk_widget_set_sensitive (app->menubar.download.move_up, sensitive);
	gtk_widget_set_sensitive (app->menubar.download.move_down, sensitive);
	gtk_widget_set_sensitive (app->menubar.download.move_top, sensitive);
	gtk_widget_set_sensitive (app->menubar.download.move_bottom, sensitive);
}

void	ug_app_decide_category_sensitive (UgAppGtk* app)
{
	static gboolean		last_sensitive = TRUE;
	gboolean			sensitive;

	if (app->cwidget.current.category)
		sensitive = TRUE;
	else
		sensitive = FALSE;

	if (last_sensitive != sensitive) {
		last_sensitive = sensitive;
		gtk_widget_set_sensitive (app->menubar.category.properties, sensitive);
		gtk_widget_set_sensitive (app->menubar.view.columns.self, sensitive);
	}
	if (app->cwidget.current.category == app->cwidget.primary.category) {
		gtk_widget_set_sensitive (app->menubar.category.delete, FALSE);
		gtk_widget_set_sensitive (app->menubar.category.properties, FALSE);
	}
	else {
		gtk_widget_set_sensitive (app->menubar.category.delete, sensitive);
		gtk_widget_set_sensitive (app->menubar.category.properties, sensitive);
	}

	ug_app_decide_download_sensitive (app);
}

void	ug_app_decide_bt_meta_sensitive (UgAppGtk* app)
{
	gboolean	sensitive;

	sensitive = app->setting.plugin.aria2.enable;
	gtk_widget_set_sensitive (app->trayicon.menu.create_metalink, sensitive);
	gtk_widget_set_sensitive (app->trayicon.menu.create_torrent, sensitive);
	gtk_widget_set_sensitive (app->toolbar.create_metalink, sensitive);
	gtk_widget_set_sensitive (app->toolbar.create_torrent, sensitive);
	gtk_widget_set_sensitive (app->menubar.file.create.metalink, sensitive);
	gtk_widget_set_sensitive (app->menubar.file.create.torrent, sensitive);
}

