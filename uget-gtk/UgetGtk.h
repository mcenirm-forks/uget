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

#ifndef UGET_GTK_H
#define UGET_GTK_H

#include <gtk/gtk.h>
// uglib
#include <UgIpc.h>
#include <UgOption.h>
#include <UgRunning.h>
#include <UgetGtk-setting.h>
#include <UgSummary.h>
#include <UgCategory-gtk.h>
#include <UgCategoryWidget.h>

#ifdef __cplusplus
extern "C" {
#endif


#define UGET_GTK_DIR					"uGet"
#define UGET_GTK_SETTING_FILE			"Setting.xml"
#define UGET_GTK_CATEGORY_FILE			"CategoryList.xml"
#define UGET_GTK_DOWNLOAD_FILE			"DownloadList.xml"
#define UGET_GTK_ICON_NAME				"uget-icon"
#define UGET_GTK_ICON_ERROR_NAME		"uget-error"
#define UGET_GTK_ICON_ACTIVE_NAME		"uget-downloading"
#define	UGET_GTK_ACCEL_PATH_CTRL_N		"<uGet>/New/Download"
#define	UGET_GTK_CATEGORY_STOCK			GTK_STOCK_DND_MULTIPLE

typedef struct	UgetGtk_				UgetGtk;

// implemented in uget-gtk/main.c
const gchar*	ug_get_data_dir (void);
// return g_get_user_config_dir () + UGET_GTK_DIR + "attachment"
const gchar*	ug_get_attachment_dir (void);

// ----------------------------------------------------------------------------
// UgetGtk: Uget GTK+ version
//
struct UgetGtk_
{
	// command argument, IPC, UgRunning
	UgOption		option;			// initialize in uget-gtk/main.c
	UgIpc			ipc;			// initialize in uget-gtk/main.c
	UgRunning*		running;

	UgetGtkSetting	setting;		// uget-gtk-setting.h
	gboolean		user_action;	// some job stop by user
	UgScheduleState	schedule_state;

	// Launch application
	GRegex*			launch_regex;
	// Clipboard
	struct UgetGtkClipboard
	{
		GtkClipboard*	self;
		gchar*			text;
		GRegex*			regex;
	} clipboard;

	// dialogs
	struct UgetGtkDialogs
	{
		GtkWidget*		error;
		GtkWidget*		message;
		GtkWidget*		close_confirmation;
		GtkWidget*		delete_confirmation;
		GtkWidget*		setting;
	} dialogs;

	// -------------------------------------------------------
	// GUI (initialize in uget-gtk-gui.c)
	GtkAccelGroup*		accel_group;
	UgCategoryWidget	cwidget;
	UgSummary			summary;

	// --------------------------------
	// System tray icon
	struct UgetGtkTrayIcon
	{
		GtkStatusIcon*	self;
		gboolean		error_occurred;
		guint			last_status;

		struct UgetGtkTrayIconMenu
		{
			GtkWidget*		self;		// (GtkMenu) pop-up menu

			GtkWidget*		create_download;
			GtkWidget*		create_clipboard;
			GtkWidget*		settings;
			GtkWidget*		about;
			GtkWidget*		offline_mode;
			GtkWidget*		quit;
		} menu;
	} tray_icon;

	// --------------------------------
	// Main Window
	struct UgetGtkWindow
	{
		GtkWindow*		self;
		// layout
		GtkPaned*		vpaned;		// right side (UgDownloadWidget and UgSummary)
		GtkPaned*		hpaned;		// separate left side and right side
	} window;

	// --------------------------------
	// status bar
	struct UgetGtkStatusbar
	{
		GtkStatusbar*	self;
		GtkLabel*		speed;
	} statusbar;

	// --------------------------------
	// Toolbar
	struct UgetGtkToolbar
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
	struct UgetGtkMenubar
	{
		GtkWidget*	self;	// GtkMenuBar

		// GtkWidget*	self;		// GtkMenu*
		// GtkWidget*	shell;		// GtkMenuShell*
		// GtkWidget*	other;		// GtkMenuItem*
		struct UgetGtkFileMenu
		{
			// file.create
			struct UgetGtkFileCreateMenu
			{
				GtkWidget*	download;
				GtkWidget*	category;
				GtkWidget*	batch;
				GtkWidget*	from_clipboard;
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

		struct UgetGtkEditMenu
		{
			GtkWidget*	clipboard_monitor;
			GtkWidget*	clipboard_option;
			GtkWidget*	shutdown;
			GtkWidget*	settings;
		} edit;

		struct UgetGtkViewMenu
		{
			GtkWidget*	toolbar;
			GtkWidget*	statusbar;
			GtkWidget*	category;
			GtkWidget*	summary;

			struct UgetGtkViewItemMenu
			{
				GtkWidget*	name;
				GtkWidget*	folder;
				GtkWidget*	category;
//				GtkWidget*	elapsed;
				GtkWidget*	url;
				GtkWidget*	message;
			} summary_items;

			struct UgetGtkViewColMenu
			{
				GtkWidget*	self;		// GtkMenu

				GtkWidget*	completed;
				GtkWidget*	total;
				GtkWidget*	percent;
				GtkWidget*	elapsed;	// consuming time
				GtkWidget*	left;		// remaining time
				GtkWidget*	speed;
				GtkWidget*	retry;
				GtkWidget*	category;
				GtkWidget*	url;
				GtkWidget*	added_on;
				GtkWidget*	completed_on;
			} columns;					// download columns
		} view;

		struct UgetGtkCategoryMenu
		{
			GtkWidget*	self;		// GtkMenu

			GtkWidget*	create;
			GtkWidget*	delete;
			GtkWidget*	properties;
			GtkWidget*	default_for_new;
		} category;

		struct UgetGtkDownloadMenu
		{
			GtkWidget*	self;		// GtkMenu

			GtkWidget*	create;
			GtkWidget*	delete;
			GtkWidget*	delete_file;	// delete file and data.
			GtkWidget*	open;
			GtkWidget*	open_folder;	// open containing folder
			GtkWidget*	runnable;
			GtkWidget*	pause;

			struct UgetGtkDownloadMoveToMenu
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

		struct UgetGtkHelpMenu
		{
			GtkWidget*	about_uget;
		} help;
	} menubar;
	// Menubar --- end ---
	// --------------------------------
};

void	uget_gtk_init (UgetGtk* ugtk);
void	uget_gtk_quit (UgetGtk* ugtk);
void	uget_gtk_save (UgetGtk* ugtk);
void	uget_gtk_load (UgetGtk* ugtk);
void	uget_gtk_set_setting (UgetGtk* ugtk, UgetGtkSetting* setting);
void	uget_gtk_get_setting (UgetGtk* ugtk, UgetGtkSetting* setting);

// uget-gtk-gui.c
void	uget_gtk_init_gui (UgetGtk* ugtk);
// uget-gtk-callback.c
void	uget_gtk_init_callback (UgetGtk* ugtk);
// uget-gtk-timeout.c
void	uget_gtk_init_timeout (UgetGtk* ugtk);

// -------------------------------------------------------
// UgetGtkClipboard
void	uget_gtk_clipboard_init (struct UgetGtkClipboard* clipboard, const gchar* pattern);
void	uget_gtk_clipboard_set_pattern (struct UgetGtkClipboard* clipboard, const gchar* pattern);
void	uget_gtk_clipboard_set_text (struct UgetGtkClipboard* clipboard, gchar* text);
GList*	uget_gtk_clipboard_get_uris (struct UgetGtkClipboard* clipboard);
GList*	uget_gtk_clipboard_get_matched (struct UgetGtkClipboard* clipboard, const gchar* text);

// -------------------------------------------------------
// utility functions
void	uget_gtk_close_window (UgetGtk* ugtk);
void	uget_gtk_confirm_to_quit (UgetGtk* ugtk);
void	uget_gtk_confirm_to_delete (UgetGtk* ugtk, GCallback response, gpointer response_data);
void	uget_gtk_show_message (UgetGtk* ugtk, GtkMessageType type, const gchar* message);

// -------------------------------------------------------
// Functions are used to refresh status and data.
void	uget_gtk_move_menu_refresh (struct UgetGtkMenubar* menubar, UgetGtk* ugtk, gboolean reset);
void	uget_gtk_tray_icon_refresh (struct UgetGtkTrayIcon* icon, guint n_active, gdouble speed);
void	uget_gtk_statusbar_refresh (struct UgetGtkStatusbar* statusbar, UgDownloadWidget* dwidget);
void	uget_gtk_statusbar_refresh_speed (struct UgetGtkStatusbar* statusbar, gdouble speed);
void	uget_gtk_refresh_download_column (UgetGtk* ugtk);
void	uget_gtk_refresh_download_sensitive (UgetGtk* ugtk);
void	uget_gtk_refresh_category_sensitive (UgetGtk* ugtk);


#ifdef __cplusplus
}
#endif

#endif  // End of UGET_GTK_H

