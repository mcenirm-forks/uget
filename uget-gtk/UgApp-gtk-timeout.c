/*
 *
 *   Copyright (C) 2005-2011 by plushuang
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

#ifdef	HAVE_CONFIG_H
#include <config.h>
#  if HAVE_LIBNOTIFY
#    include <libnotify/notify.h>
#  endif
#  if HAVE_GSTREAMER
#    include <gst/gst.h>
#  endif
#endif

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>

#ifdef	_WIN32
#ifndef	_WIN32_IE
#define	_WIN32_IE	0x0600
#endif
#include <windows.h>
#include <mmsystem.h>
#endif	// _End of _WIN32

#include <UgUri.h>
#include <UgUtils.h>
#include <UgString.h>
#include <UgData-download.h>
#include <UgApp-gtk.h>
#include <UgDownloadDialog.h>

#include <glib/gi18n.h>


static void	ug_app_notify_starting (UgAppGtk* app);
static void	ug_app_notify_completed (UgAppGtk* app);
// GSourceFunc
static gboolean	ug_app_timeout_ipc (UgAppGtk* app);
static gboolean	ug_app_timeout_queuing (UgAppGtk* app);
static gboolean	ug_app_timeout_clipboard (UgAppGtk* app);
static gboolean	ug_app_timeout_autosave (UgAppGtk* app);

void	ug_app_init_timeout (UgAppGtk* app)
{
	// 0.5 seconds
	g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 500,
			(GSourceFunc) ug_app_timeout_ipc, app, NULL);
	// 0.5 seconds
	g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 500,
			(GSourceFunc) ug_running_dispatch, &app->running, NULL);
	// 1 seconds
	g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE, 1,
			(GSourceFunc) ug_app_timeout_queuing, app, NULL);
	// 2 seconds
	g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE, 2,
			(GSourceFunc) ug_app_timeout_clipboard, app, NULL);
	// 1 minutes
	g_timeout_add_seconds_full (G_PRIORITY_DEFAULT_IDLE, 60,
			(GSourceFunc) ug_app_timeout_autosave, app, NULL);
}

static gboolean	ug_app_timeout_autosave (UgAppGtk* app)
{
	static guint	counts = 0;

	counts++;
	// "app->setting.auto_save.interval" may changed by user
	if (counts >= app->setting.auto_save.interval) {
		counts = 0;
		if (app->setting.auto_save.active)
			ug_app_save (app);
	}
	// return FALSE if the source should be removed.
	return TRUE;
}

// ----------------------------------------------------------------------------
// Clipboard
//
static gboolean	clipboard_processing = FALSE;

static void	on_keep_above_window_show (GtkWindow *window, gpointer  user_data)
{
	gtk_window_present (window);
	gtk_window_set_keep_above (window, FALSE);
}

static void	on_add_download_response (GtkDialog *dialog, gint response, UgDownloadDialog* ddialog)
{
	UgCategory*		category;
	UgAppGtk*		app;
	GList*			list;
	GList*			link;

	if (response == GTK_RESPONSE_OK) {
		app = ddialog->user.app;
		ug_download_form_get_folder_list (&ddialog->download,
				&app->setting.folder_list);
		category = ug_download_dialog_get_category (ddialog);
		if (category) {
			list = ug_download_dialog_get_downloads (ddialog);
			for (link = list;  link;  link = link->next)
				ug_category_add (category, link->data);
			g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
			g_list_free (list);
			gtk_widget_queue_draw ((GtkWidget*) app->cwidget.self);
		}
	}
	ug_download_dialog_free (ddialog);
}

static void	uget_add_uris_selected (UgAppGtk* app, GList* list)
{
	UgDownloadDialog*	ddialog;
	UgSelectorPage*		page;
	gchar*				string;

	if (list->next == NULL) {
		// only 1 url matched
		string = g_strconcat (UG_APP_GTK_NAME " - ", _("New from Clipboard"),
				" (", _("only one matched"), ")", NULL);
//		ddialog = ug_download_dialog_new (string, app->window.self);
		ddialog = ug_download_dialog_new (string, NULL);
		g_free (string);
		gtk_entry_set_text ((GtkEntry*) ddialog->download.url_entry, list->data);
		g_free (list->data);
	}
	else {
		string = g_strconcat (UG_APP_GTK_NAME " - ", _("New from Clipboard"), NULL);
//		ddialog = ug_download_dialog_new (string, app->window.self);
		ddialog = ug_download_dialog_new (string, NULL);
		g_free (string);
		ug_download_dialog_use_selector (ddialog);
		ug_selector_hide_href (&ddialog->selector);
		page = ug_selector_add_page (&ddialog->selector, _("Clipboard"));
		ug_selector_page_add_uris (page, list);
	}
	g_list_free (list);

	ug_download_form_set_folder_list (&ddialog->download,
			app->setting.folder_list);
	ug_download_dialog_set_category (ddialog, &app->cwidget);
	// connect signal and set data in download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_add_download_response), ddialog);
	g_signal_connect_after (ddialog->self, "show",
			G_CALLBACK (on_keep_above_window_show), NULL);
	// Make sure dilaog will show on top first time.
	// uget_on_keep_above_window_show ()  will set keep_above = FALSE
	gtk_window_set_keep_above ((GtkWindow*) ddialog->self, TRUE);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	uget_add_uris_quietly (UgAppGtk* app, GList* list)
{
	UgCategory*		category;
	UgDataset*		dataset;
	UgDataCommon*	common;
	GList*			link;

	// get category
	if (app->setting.clipboard.nth_category == -1)
		category = ug_category_view_get_cursor (app->cwidget.view);
	else {
		category = ug_category_view_get_nth (app->cwidget.view,
				app->setting.clipboard.nth_category);
	}
	if (category == NULL)
		category = ug_category_view_get_nth (app->cwidget.view, 0);
	if (category == NULL)
		return;
	// add list to category
	for (link = list;  link;  link = link->next) {
		dataset = ug_dataset_new ();
		ug_data_assign (dataset, category->defaults);
		common = ug_dataset_realloc (dataset, UG_DATA_COMMON_I, 0);
		g_free (common->url);
		common->url = link->data;
		ug_download_complete_data (dataset);
		ug_category_add (category, dataset);
	}
	g_list_free (list);
}

static void on_clipboard_text_received (GtkClipboard*	clipboard,
                                        const gchar*	text,
                                        gpointer		user_data)
{
	UgAppGtk*		app;
	GList*			list;

	app = (UgAppGtk*) user_data;
	list = ug_clipboard_get_matched (&app->clipboard, text);
	if (list) {
		if (app->setting.clipboard.quiet)
			uget_add_uris_quietly (app, list);
		else
			uget_add_uris_selected (app, list);
	}
	clipboard_processing = FALSE;
}

static gboolean	ug_app_timeout_clipboard (UgAppGtk* app)
{
	if (app->setting.clipboard.monitor && clipboard_processing == FALSE) {
		// set FALSE in on_clipboard_text_received()
		clipboard_processing = TRUE;
		gtk_clipboard_request_text (app->clipboard.self,
				on_clipboard_text_received, app);
	}
	// return FALSE if the source should be removed.
	return TRUE;
}

// ----------------------------------------------------------------------------
// IPC
//
static void	uget_add_download_selected (UgAppGtk* app, GList* list, gint category_index)
{
	UgDownloadDialog*	ddialog;
	UgSelectorPage*		page;
	gchar*				string;

	string = g_strconcat (UG_APP_GTK_NAME " - ", _("New Download"), NULL);
//	ddialog = ug_download_dialog_new (string, app->window.self);
	ddialog = ug_download_dialog_new (string, NULL);
	g_free (string);
	if (list->next) {
		ug_download_dialog_set (ddialog, list->data);
		ug_download_dialog_use_selector (ddialog);
		ug_selector_hide_href (&ddialog->selector);
		page = ug_selector_add_page (&ddialog->selector, _("Command line"));
		ug_selector_page_add_downloads (page, list);
	}
	else {
		ug_download_dialog_set (ddialog, list->data);
		ddialog->download.changed.url = TRUE;
		// set external data to UgDownloadDialog
		ddialog->dataset = list->data;
		ug_dataset_ref (list->data);
	}

	ug_download_form_set_folder_list (&ddialog->download,
			app->setting.folder_list);
	ug_download_dialog_set_category (ddialog, &app->cwidget);
	ug_category_view_set_cursor (ddialog->category_view, category_index, -1);
	// connect signal and set data in download dialog
	ddialog->user.app = app;
	g_signal_connect (ddialog->self, "response",
			G_CALLBACK (on_add_download_response), ddialog);
	g_signal_connect_after (ddialog->self, "show",
			G_CALLBACK (on_keep_above_window_show), NULL);
	// Make sure dilaog will show on top first time.
	// uget_on_keep_above_window_show ()  will set keep_above = FALSE
	gtk_window_set_keep_above ((GtkWindow*) ddialog->self, TRUE);
	gtk_widget_show ((GtkWidget*) ddialog->self);
}

static void	uget_add_download_quietly (UgAppGtk* app, GList* list, gint category_index)
{
	UgCategory*		category;
	UgDataset*		dataset;
	GList*			link;

	// get category
	category = ug_category_view_get_nth (app->cwidget.view, category_index);
	if (category == NULL)
		category = ug_category_view_get_cursor (app->cwidget.view);
	if (category == NULL)
		return;
	// add list to category
	for (link = list;  link;  link = link->next) {
		dataset = link->data;
		ug_data_assign (dataset, category->defaults);
		ug_download_complete_data (dataset);
		ug_category_add (category, dataset);
	}
}

static gboolean	ug_app_timeout_ipc (UgAppGtk* app)
{
	GPtrArray*		args;
	GList*			list;
	gint			category_index;

	args = ug_ipc_pop (&app->ipc);
	if (args == NULL)
		return TRUE;
	// If no argument, program presents main window to the user.
	if (args->len == 1) {
		ug_arg_free (args, TRUE);
		if (gtk_widget_get_visible ((GtkWidget*) app->window.self) == FALSE)
			gtk_window_deiconify (app->window.self);
		gtk_window_present (app->window.self);
		return TRUE;
	}
	// get and parse downloads
	list = ug_option_parse (&app->option, args);
	ug_arg_free (args, TRUE);
	// set-offline
	switch (app->option.data->offline) {
	case 0:
		app->setting.offline_mode = FALSE;
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*) app->menubar.file.offline_mode, FALSE);
		break;

	case 1:
		app->setting.offline_mode = TRUE;
		gtk_check_menu_item_set_active (
				(GtkCheckMenuItem*) app->menubar.file.offline_mode, TRUE);
		break;

	default:
		break;
	}
	// if no data
	if (list == NULL)
		return TRUE;
	// create attachment (backup cookie & post file)
	if (ug_download_create_attachment (list->data, FALSE) == TRUE)
		g_list_foreach (list->next, (GFunc) ug_download_assign_attachment, list->data);
	// add downloads
	category_index = app->option.data->category_index;
	if (app->option.data->quiet)
		uget_add_download_quietly (app, list, category_index);
	else
		uget_add_download_selected (app, list, category_index);
	// free unused downloads
	g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
	g_list_free (list);
	// return FALSE if the source should be removed.
	return TRUE;
}


// ----------------------------------------------------------------------------
// queuing
//
static void	ug_app_launch_default_app (UgDataset* dataset, GRegex* regex)
{
	UgDataCommon*	common;
	const gchar*	file_ext;

	common = UG_DATASET_COMMON (dataset);
	if (common == NULL  ||  common->file == NULL)
		return;
	file_ext = strrchr (common->file, G_DIR_SEPARATOR);
	if (file_ext == NULL)
		file_ext = common->file;
	file_ext = strrchr (common->file, '.');
	if (file_ext == NULL)
		return;
	if (g_regex_match (regex, file_ext + 1, 0, NULL))
		ug_launch_default_app (common->folder, common->file);
}

// scheduler
static gboolean	ug_app_decide_schedule_state (UgAppGtk* app)
{
	struct tm*	timem;
	time_t		timet;
	guint		weekdays, dayhours;
	gboolean	changed;
	UgScheduleState	state;

	if (app->setting.scheduler.enable == FALSE) {
		app->schedule_state = UG_SCHEDULE_NORMAL;
		return FALSE;
	}

	// get current time
	timet = time (NULL);
	timem = localtime (&timet);
	dayhours = timem->tm_hour;
	if (timem->tm_wday == 0)
		weekdays = 6;
	else
		weekdays = timem->tm_wday - 1;
	// get current schedule state
	state = app->setting.scheduler.state [weekdays][dayhours];
	if (app->schedule_state == state)
		changed = FALSE;
	else {
		app->schedule_state  = state;
		changed = TRUE;
		// switch mode
		switch (state)
		{
		case UG_SCHEDULE_TURN_OFF:
			ug_running_clear (&app->running);
			break;

		case UG_SCHEDULE_LIMITED_SPEED:
			ug_running_set_speed (&app->running,
					app->setting.scheduler.speed_limit);
			break;

		default:
			// no speed limit
			ug_running_set_speed (&app->running, 0);
			break;
		}
	}

	return changed;
}

// start, stop jobs and refresh information.
static gboolean	ug_app_timeout_queuing (UgAppGtk* app)
{
	GList*		jobs;
	GList*		list;
	GList*		link;
	gboolean		changed;
	static guint	n_before = 0;
	guint			n_after;
	union {
		UgRelation*	relation;
		gchar*		string;
		gdouble		speed;
	} temp;


	// If changed is TRUE, it will refresh all category-related data.
	changed = ug_app_decide_schedule_state (app);
	// do something for inactive jobs
	jobs = ug_running_get_inactive (&app->running);
	for (link = jobs;  link;  link = link->next) {
		temp.relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		// This will change tray icon.
		if (temp.relation->hints & UG_HINT_ERROR)
			app->trayicon.error_occurred = TRUE;
		// launch default application
		if ((temp.relation->hints & UG_HINT_COMPLETED) && app->setting.launch.active)
			ug_app_launch_default_app (link->data, app->launch_regex);
		// remove inactive jobs from group
		ug_running_remove (&app->running, link->data);
		changed = TRUE;
	}
	g_list_free (jobs);

	// category list
	list = ug_category_widget_get_list (&app->cwidget);
	for (link = list;  link;  link = link->next) {
		// clear excess downloads
		if (ug_category_gtk_clear_excess (link->data))
			changed = TRUE;
		// Don't activate jobs if offline mode was enabled or schedule turns off jobs.
		if (app->setting.offline_mode || app->schedule_state == UG_SCHEDULE_TURN_OFF)
			continue;
		// get queuing jobs from categories and activate them
		jobs = ug_category_gtk_get_jobs (link->data);
		if (ug_running_add_jobs (&app->running, jobs))
			changed = TRUE;
		g_list_free (jobs);
	}
	g_list_free (list);

	// get number of jobs after queuing
	n_after = ug_running_get_n_jobs (&app->running);
	// some jobs was start or stop
	if (n_before != n_after) {
		// downloading start
		if (n_before == 0  &&  n_after > 0) {
			// starting notification
			if (app->setting.ui.start_notification)
				ug_app_notify_starting (app);
		}
		// downloading completed
		else if (n_before > 0  &&  n_after == 0) {
			if (app->user_action == FALSE) {
				// completed notification
				ug_app_notify_completed (app);
				// shutdown
				if (app->setting.shutdown) {
					ug_app_save (app);
					ug_shutdown ();
				}
			}
		}
		// window title
		if (n_after > 0) {
			temp.string = g_strdup_printf (UG_APP_GTK_NAME " - %u %s",
					n_after, _("downloading"));
			gtk_window_set_title (app->window.self, temp.string);
			g_free (temp.string);
		}
		else
			gtk_window_set_title (app->window.self, UG_APP_GTK_NAME);
		// update
		n_before = n_after;
		changed = TRUE;
	}

	// category or download status changed
	if (changed || n_after) {
		temp.speed = ug_running_get_speed (&app->running);
		gtk_widget_queue_draw (app->cwidget.current.widget->self);
		// summary
		ug_summary_show (&app->summary,
				ug_download_widget_get_cursor (app->cwidget.current.widget));
		// status bar
		ug_statusbar_set_speed (&app->statusbar, temp.speed);
		// tray icon
		ug_trayicon_set_info (&app->trayicon, n_after, temp.speed);
	}
	// category status changed
	if (changed)
		gtk_widget_queue_draw (app->cwidget.self);

	// return FALSE if the source should be removed.
	return TRUE;
}

// ----------------------------------------------------------------------------
// sound

// static void uget_play_sound (const gchar* sound_file);
// GStreamer
#ifdef HAVE_GSTREAMER
static gboolean ugst_bus_func (GstBus* bus, GstMessage* msg, gpointer data)
{
	GstElement*	playbin = data;
	GError*		error   = NULL;

	switch (GST_MESSAGE_TYPE (msg)) {
	case GST_MESSAGE_WARNING:
		gst_message_parse_warning (msg, &error, NULL);
//		g_print ("uget-gtk: gstreamer: %s\n", error->message);
		g_error_free (error);
		break;

	case GST_MESSAGE_ERROR:
		gst_message_parse_error (msg, &error, NULL);
//		g_print ("uget-gtk: gstreamer: %s\n", error->message);
		g_error_free (error);
		// clean up
	case GST_MESSAGE_EOS:
		gst_element_set_state (playbin, GST_STATE_NULL);
		gst_object_unref (GST_OBJECT (playbin));
		break;

	default:
		break;
	}
	return TRUE;
}

static void uget_play_sound (const gchar* sound_file)
{
	GstElement*	playbin = NULL;
	GstBus*		bus     = NULL;
	char*		uri;
	gchar*		file_os;
	extern gboolean	gst_inited;		// uget-gtk/main.c

	if (gst_inited == FALSE)
		return;

	file_os = g_filename_from_utf8 (sound_file, -1, NULL, NULL, NULL);
	if (g_file_test (file_os, G_FILE_TEST_EXISTS) == FALSE) {
		g_free (file_os);
		return;
	}

	playbin = gst_element_factory_make ("playbin", "play");
	if (playbin == NULL) {
		g_free (file_os);
		return;
	}

	uri = g_filename_to_uri (file_os, NULL, NULL);
	g_free (file_os);

	g_object_set (G_OBJECT (playbin), "uri", uri, NULL);

	bus = gst_pipeline_get_bus (GST_PIPELINE (playbin));
	gst_bus_add_watch (bus, ugst_bus_func, playbin);

	gst_element_set_state (playbin, GST_STATE_PLAYING);

	gst_object_unref (bus);
	g_free (uri);
}

#elif defined (_WIN32)
static void uget_play_sound (const gchar* sound_file)
{
	gunichar2*	file_wcs;

	if (g_file_test (sound_file, G_FILE_TEST_EXISTS) == FALSE)
		return;

	file_wcs = g_utf8_to_utf16 (sound_file, -1, NULL, NULL, NULL);
	PlaySoundW (file_wcs, NULL, SND_ASYNC | SND_FILENAME);
	g_free (file_wcs);
}

#else
// --disable-gstreamer
static void uget_play_sound (const gchar* sound_file)
{
}
#endif	// HAVE_GSTREAMER

// ----------------------------------------------------------------------------
// notification
//
#ifdef HAVE_LIBNOTIFY
static void ug_app_notify (UgAppGtk* app, const gchar* title, const gchar* body)
{
	static	NotifyNotification*	notification = NULL;
	gchar*	string;

	if (notify_is_initted () == FALSE)
		return;
	// set title and body
	string = g_strconcat (UG_APP_GTK_NAME " - ", title, NULL);
	if (notification == NULL) {

#if defined (NOTIFY_VERSION_MINOR) && NOTIFY_VERSION_MAJOR >= 0 && NOTIFY_VERSION_MINOR >= 7
		notification = notify_notification_new (string,
				body, UG_APP_GTK_ICON_NAME);
#else
		notification = notify_notification_new (string,
				body, UG_APP_GTK_ICON_NAME, NULL);
#endif
		notify_notification_set_timeout (notification, 7000);	// milliseconds
	}
	else {
		notify_notification_update (notification, string,
				body, UG_APP_GTK_ICON_NAME);
	}
	g_free (string);

	notify_notification_show (notification, NULL);
}
#elif defined (_WIN32)
static void ug_app_notify (UgAppGtk* app, const gchar* title, const gchar* body)
{
	static	NOTIFYICONDATAW*	pNotifyData = NULL;
	gchar*		string;
	gunichar2*	string_wcs;

	if (pNotifyData == NULL) {
		pNotifyData = g_malloc0 (sizeof (NOTIFYICONDATAW));
		pNotifyData->cbSize = sizeof (NOTIFYICONDATAW);
		pNotifyData->uFlags = NIF_INFO;			// Use a balloon ToolTip instead of a standard ToolTip.
		pNotifyData->uTimeout = 7000;			// milliseconds, This member is deprecated as of Windows Vista.
		pNotifyData->dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;	// Add an information icon to balloon ToolTip.
		// gtkstatusicon.c
		// (create_tray_observer): WNDCLASS.lpszClassName = "gtkstatusicon-observer"
		pNotifyData->hWnd = FindWindowA ("gtkstatusicon-observer", NULL);
	}

	if (pNotifyData->hWnd == NULL)
		return;
	// gtkstatusicon.c
	// (gtk_status_icon_init): priv->nid.uID = GPOINTER_TO_UINT (status_icon);
	pNotifyData->uID = GPOINTER_TO_UINT (app->trayicon.self);
	// title
	string = g_strconcat (UG_APP_GTK_NAME " - ", title, NULL);
	string_wcs = g_utf8_to_utf16 (string,  -1, NULL, NULL, NULL);
	wcsncpy (pNotifyData->szInfoTitle, string_wcs, 64 -1);	// null-terminated
	g_free (string);
	g_free (string_wcs);
	// body
	string_wcs = g_utf8_to_utf16 (body, -1, NULL, NULL, NULL);
	wcsncpy (pNotifyData->szInfo, string_wcs, 256 -1);	// null-terminated
	g_free (string_wcs);

	Shell_NotifyIconW (NIM_MODIFY, pNotifyData);
}
#else
static void ug_app_notify (UgAppGtk* app, const gchar* title, const gchar* info)
{
	// do nothing
}
#endif	// HAVE_LIBNOTIFY

#define	NOTIFICATION_STARTING_TITLE			_("Download Starting")
#define	NOTIFICATION_STARTING_STRING		_("Starting download queue.")
#define	NOTIFICATION_COMPLETED_TITLE		_("Download Completed")
#define	NOTIFICATION_COMPLETED_STRING		_("All queuing downloads have been completed.")

static void	ug_app_notify_completed (UgAppGtk* app)
{
	gchar*	path;

	ug_app_notify (app,
			NOTIFICATION_COMPLETED_TITLE,
			NOTIFICATION_COMPLETED_STRING);

	if (app->setting.ui.sound_notification) {
		path = g_build_filename (ug_get_data_dir (),
				"sounds", "uget", "notification.wav",  NULL);
		uget_play_sound (path);
		g_free (path);
	}
}

static void	ug_app_notify_starting (UgAppGtk* app)
{
//	gchar*	path;

	ug_app_notify (app,
			NOTIFICATION_STARTING_TITLE,
			NOTIFICATION_STARTING_STRING);

//	path = g_build_filename (ug_get_data_dir (), "sounds", "uget",
//			"notification.wav",  NULL);
//	uget_play_sound (path);
//	g_free (path);
}

