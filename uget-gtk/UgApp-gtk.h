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

#ifndef UG_APP_GTK_H
#define UG_APP_GTK_H

#include <gtk/gtk.h>
// uglib
#include <UgApp-base.h>
#include <UgIpc.h>
#include <UgOption.h>
#include <UgXmlrpc.h>
#include <UgRunning.h>
#include <UgSetting.h>
#include <UgSummary.h>
#include <UgCategory-gtk.h>
#include <UgCategoryWidget.h>

#ifdef HAVE_APP_INDICATOR
#include <libappindicator/app-indicator.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define UG_APP_GTK_DIR					"uGet"
#define UG_APP_GTK_SETTING_FILE			"Setting.xml"
#define UG_APP_GTK_CATEGORY_FILE		"CategoryList.xml"
#define UG_APP_GTK_DOWNLOAD_FILE		"DownloadList.xml"
#define UG_APP_GTK_ICON_NAME			"uget-icon"
#define UG_APP_GTK_ICON_ERROR_NAME		"uget-error"
#define UG_APP_GTK_ICON_ACTIVE_NAME		"uget-downloading"
#define	UG_APP_GTK_ACCEL_PATH_CTRL_N	"<uGet>/New/Download"
#define	UG_APP_GTK_CATEGORY_STOCK		GTK_STOCK_DND_MULTIPLE

typedef struct	UgAppGtk				UgAppGtk;

// implemented in UgApp-gtk-main.c
const gchar*	ug_get_data_dir (void);
// return g_get_user_config_dir () + UG_APP_GTK_DIR + "attachment"
const gchar*	ug_get_attachment_dir (void);


// ----------------------------------------------------------------------------
// UgAppGtk: Uget application for GTK+

struct UgAppGtk
{
	// command argument, IPC, UgRunning
	UgIpc			ipc;			// initialize in UgApp-gtk-main.c
	UgOption		option;			// initialize in UgApp-gtk-main.c
	UgRunning		running;

	UgSetting		setting;		// UgSetting.h
	gboolean		user_action;	// some job stop by user
	UgScheduleState	schedule_state;

	// Launch application
	GRegex*			launch_regex;
	// aria2 plug-in
	struct
	{
		UgXmlrpc	xmlrpc;
		gboolean	launched;
	} aria2;

	// Clipboard
	struct UgClipboard
	{
		GtkClipboard*	self;
		gchar*			text;
		GRegex*			regex;
	} clipboard;

	// dialogs
	struct UgDialogs
	{
		GtkWidget*		error;
		GtkWidget*		message;
		GtkWidget*		close_confirmation;
		GtkWidget*		delete_confirmation;
		GtkWidget*		setting;
	} dialogs;

	// -------------------------------------------------------
	// GUI (initialize in UgApp-gtk-gui.c)
	GtkAccelGroup*		accel_group;
	UgCategoryWidget	cwidget;
	UgSummary			summary;

	// --------------------------------
	// System tray icon
	struct UgTrayIcon
	{
#ifdef HAVE_APP_INDICATOR
		AppIndicator*	indicator;
#else
		GtkStatusIcon*	self;
#endif
		gboolean		error_occurred;
		guint			last_status;

		struct UgTrayIconMenu
		{
			GtkWidget*		self;		// (GtkMenu) pop-up menu

			GtkWidget*		create_download;
			GtkWidget*		create_clipboard;
			GtkWidget*		create_torrent;
			GtkWidget*		create_metalink;
			GtkWidget*		settings;
			GtkWidget*		about;
			GtkWidget*		show_window;
			GtkWidget*		offline_mode;
			GtkWidget*		quit;
		} menu;
	} trayicon;

	// --------------------------------
	// Main Window
	struct UgWindow
	{
		GtkWindow*		self;
		// layout
		GtkPaned*		vpaned;		// right side (UgDownloadWidget and UgSummary)
		GtkPaned*		hpaned;		// separate left side and right side
	} window;

	// --------------------------------
	// status bar
	struct UgStatusbar
	{
		GtkStatusbar*	self;
		GtkLabel*		speed;
	} statusbar;

	// --------------------------------
	// Toolbar
	struct UgToolbar
	{
		GtkWidget*		self;			// GtkToolbar

		// GtkToolItem
		GtkWidget*		create;

		// GtkMenuItem
		// menu for tool button
		GtkWidget*		create_download;
		GtkWidget*		create_category;
		GtkWidget*		create_batch;
		GtkWidget*		create_clipboard;
		GtkWidget*		create_torrent;
		GtkWidget*		create_metalink;

		// GtkToolItem
		GtkWidget*		save;
		GtkWidget*		runnable;
		GtkWidget*		pause;
		GtkWidget*		properties;
		GtkWidget*		move_up;
		GtkWidget*		move_down;
		GtkWidget*		move_top;
		GtkWidget*		move_bottom;
	} toolbar;

	// --------------------------------
	// Menubar --- start ---
	struct UgMenubar
	{
		GtkWidget*	self;	// GtkMenuBar

		// GtkWidget*	self;		// GtkMenu*
		// GtkWidget*	shell;		// GtkMenuShell*
		// GtkWidget*	other;		// GtkMenuItem*
		struct UgFileMenu
		{
			// file.create
			struct UgFileCreateMenu
			{
				GtkWidget*	download;
				GtkWidget*	category;
				GtkWidget*	batch;
				GtkWidget*	from_clipboard;
				GtkWidget*	torrent;
				GtkWidget*	metalink;
			} create;
			// file.save
			GtkWidget*	save;
			// file.import & export
			GtkWidget*	import_html;
			GtkWidget*	import_text;
			GtkWidget*	export_text;

			GtkWidget*	offline_mode;
			GtkWidget*	quit;
		} file;

		struct UgEditMenu
		{
			GtkWidget*	clipboard_monitor;
			GtkWidget*	clipboard_option;
			GtkWidget*	shutdown;
			GtkWidget*	settings;
		} edit;

		struct UgViewMenu
		{
			GtkWidget*	toolbar;
			GtkWidget*	statusbar;
			GtkWidget*	category;
			GtkWidget*	summary;

			struct UgViewItemMenu
			{
				GtkWidget*	name;
				GtkWidget*	folder;
				GtkWidget*	category;
//				GtkWidget*	elapsed;
				GtkWidget*	url;
				GtkWidget*	message;
			} summary_items;

			struct UgViewColMenu
			{
				GtkWidget*	self;		// GtkMenu

				GtkWidget*	completed;
				GtkWidget*	total;
				GtkWidget*	percent;
				GtkWidget*	elapsed;	// consuming time
				GtkWidget*	left;		// remaining time
				GtkWidget*	speed;
				GtkWidget*	up_speed;
				GtkWidget*	retry;
				GtkWidget*	category;
				GtkWidget*	url;
				GtkWidget*	added_on;
				GtkWidget*	completed_on;
			} columns;					// download columns
		} view;

		struct UgCategoryMenu
		{
			GtkWidget*	self;		// GtkMenu

			GtkWidget*	create;
			GtkWidget*	delete;
			GtkWidget*	properties;
		} category;

		struct UgDownloadMenu
		{
			GtkWidget*	self;		// GtkMenu

			GtkWidget*	create;
			GtkWidget*	delete;
			GtkWidget*	delete_file;	// delete file and data.
			GtkWidget*	open;
			GtkWidget*	open_folder;	// open containing folder
			GtkWidget*	runnable;
			GtkWidget*	pause;

			struct UgDownloadMoveToMenu
			{
				GtkWidget*		self;		// GtkMenu
				GtkWidget*		item;		// GtkMenuItem

				// This array used for mapping menu item and it's category
				// index 0, 2, 4, 6...	GtkMenuItem*
				// index 1, 3, 5, 7...	UgCategoryGtk*
				GPtrArray*		array;
			} move_to;

			GtkWidget*	move_up;
			GtkWidget*	move_down;
			GtkWidget*	move_top;
			GtkWidget*	move_bottom;
			GtkWidget*	properties;
		} download;

		struct UgHelpMenu
		{
			GtkWidget*	about_uget;
		} help;
	} menubar;
	// Menubar --- end ---
	// --------------------------------
};

// use function name "ug_app_xxx" to replace "ug_app_gtk_xxx" here.
// ug_app_init is easy to understand, ug_app_gtk_init and ug_appgtk_init are not.
void	ug_app_init (UgAppGtk* app);
void	ug_app_quit (UgAppGtk* app);
void	ug_app_save (UgAppGtk* app);
void	ug_app_load (UgAppGtk* app);
void	ug_app_set_setting (UgAppGtk* app, UgSetting* setting);
void	ug_app_get_setting (UgAppGtk* app, UgSetting* setting);

// UgApp-gtk-gui.c
void	ug_app_init_gui (UgAppGtk* app);
// UgApp-gtk-callback.c
void	ug_app_init_callback (UgAppGtk* app);
// UgApp-gtk-timeout.c
void	ug_app_init_timeout (UgAppGtk* app);

// --------------------------------------------------------
// UgClipboard
void	ug_clipboard_init (struct UgClipboard* clipboard, const gchar* pattern);
void	ug_clipboard_set_pattern (struct UgClipboard* clipboard, const gchar* pattern);
void	ug_clipboard_set_text (struct UgClipboard* clipboard, gchar* text);
GList*	ug_clipboard_get_uris (struct UgClipboard* clipboard);
GList*	ug_clipboard_get_matched (struct UgClipboard* clipboard, const gchar* text);

// --------------------------------------------------------
// UgTrayIcon and UgStatusbar
void	ug_trayicon_set_info (struct UgTrayIcon* trayicon, guint n_active, gdouble speed);
void	ug_trayicon_set_visible (struct UgTrayIcon* trayicon, gboolean visible);
void	ug_statusbar_set_info (struct UgStatusbar* statusbar, UgDownloadWidget* dwidget);
void	ug_statusbar_set_speed (struct UgStatusbar* statusbar, gdouble speed);

// --------------------------------------------------------
// utility and integrate functions
void	ug_app_confirm_to_quit (UgAppGtk* app);
void	ug_app_confirm_to_delete (UgAppGtk* app, GCallback response, gpointer response_data);
void	ug_app_show_message (UgAppGtk* app, GtkMessageType type, const gchar* message);
void	ug_app_window_close (UgAppGtk* app);
void	ug_app_trayicon_decide_visible (UgAppGtk* app);
void	ug_app_menubar_sync_category (UgAppGtk* app, gboolean reset);
void	ug_app_reset_download_column (UgAppGtk* app);
void	ug_app_decide_download_sensitive (UgAppGtk* app);
void	ug_app_decide_category_sensitive (UgAppGtk* app);
void	ug_app_decide_bt_meta_sensitive (UgAppGtk* app);

// --------------------------------------------------------
// aria2
#ifdef HAVE_PLUGIN_ARIA2
void		ug_app_aria2_init (UgAppGtk* app);
gboolean	ug_app_aria2_setup (UgAppGtk* app);
gboolean	ug_app_aria2_launch (UgAppGtk* app);
void		ug_app_aria2_shutdown (UgAppGtk* app);
#else
#define		ug_app_aria2_init(app)
#define		ug_app_aria2_setup(app)
#define		ug_app_aria2_launch(app)
#define		ug_app_aria2_shutdown(app)
#endif	// HAVE_PLUGIN_ARIA2


#ifdef __cplusplus
}
#endif

#endif  // End of UG_APP_GTK_H

