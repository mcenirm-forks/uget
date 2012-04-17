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

#include <gdk/gdkkeysyms.h>    // for GDK_KEY_key...
// for GTK+ 2.18
#ifndef GDK_KEY_n
#define GDK_KEY_n			GDK_n
#endif

#ifdef	HAVE_CONFIG_H
#include <config.h>
#endif
// uglib
#include <UgApp-gtk.h>

#include <glib/gi18n.h>

static void	ug_trayicon_init	(struct UgTrayIcon* app_trayicon);
static void	ug_window_init		(struct UgWindow* app_window, UgAppGtk* app);
static void	ug_statusbar_init	(struct UgStatusbar* app_statusbar);
static void	ug_toolbar_init		(struct UgToolbar* app_toolbar, GtkAccelGroup* accel_group);
static void	ug_menubar_init		(struct UgMenubar* app_menubar, GtkAccelGroup* accel_group);

void	ug_app_init_gui (UgAppGtk* app)
{
#ifdef _WIN32
	// This will use icons\hicolor\index.theme
	GtkIconTheme*	icon_theme;
	gchar*			path;

	icon_theme = gtk_icon_theme_get_default ();
	path = g_build_filename (ug_get_data_dir (), "icons", NULL);
	gtk_icon_theme_append_search_path (icon_theme, path);
	g_free (path);
#endif	// _WIN32

	// Registers a new accelerator "Ctrl+N" with the global accelerator map.
	gtk_accel_map_add_entry (UG_APP_GTK_ACCEL_PATH_CTRL_N, GDK_KEY_n, GDK_CONTROL_MASK);
	// accelerators
	app->accel_group = gtk_accel_group_new ();
	// tray icon
	ug_trayicon_init (&app->trayicon);
	// main window
	ug_category_widget_init (&app->cwidget);
	ug_summary_init (&app->summary, app->accel_group);
	ug_statusbar_init (&app->statusbar);
	ug_toolbar_init (&app->toolbar, app->accel_group);
	ug_menubar_init (&app->menubar, app->accel_group);
	ug_window_init  (&app->window, app);
}

// ----------------------------------------------------------------------------
// UgTrayIcon
//
static void ug_trayicon_init (struct UgTrayIcon* trayicon)
{
	GtkWidget*		image;
	GtkWidget*		menu;
	GtkWidget*		menu_item;
	gchar*			icon_name;
	gchar*			file_name;

	// UgTrayIcon.menu
	menu = gtk_menu_new ();
	// New Download
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Download..."));
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.create_download = menu_item;

	// New Download from Clipboard
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _from Clipboard..."));
	image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.create_clipboard = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// New Torrent
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Torrent..."));
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.create_torrent = menu_item;

	// New Metalink
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Metalink..."));
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.create_metalink = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Settings
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Settings..."));
	image = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.settings = menu_item;

	// About
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.about = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Show window
	menu_item = gtk_menu_item_new_with_mnemonic (_("Show window"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.show_window = menu_item;

	// Offline mode
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Offline Mode"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.offline_mode = menu_item;

	// Quit
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	trayicon->menu.quit = menu_item;

	gtk_widget_show_all (menu);
	trayicon->menu.self = menu;

	// decide tray icon
	file_name = g_build_filename (ug_get_data_dir (), "icons",
	                         "hicolor", "16x16", "apps",
	                         "uget-icon.png", NULL);
	if (g_file_test (file_name, G_FILE_TEST_IS_REGULAR))
		icon_name = UG_APP_GTK_ICON_NAME;
	else
		icon_name = GTK_STOCK_GO_DOWN;
	g_free (file_name);
#ifdef HAVE_APP_INDICATOR
	trayicon->indicator = app_indicator_new ("uget-gtk", icon_name,
			APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	if (trayicon->indicator == NULL)
		return;
	app_indicator_set_menu (trayicon->indicator, GTK_MENU (trayicon->menu.self));
//	app_indicator_set_attention_icon_full (trayicon->indicator,
//			UG_APP_GTK_ICON_ACTIVE_NAME, NULL);
	app_indicator_set_attention_icon (trayicon->indicator,
			UG_APP_GTK_ICON_ACTIVE_NAME);
#else
	trayicon->self = gtk_status_icon_new_from_icon_name (icon_name);
	gtk_status_icon_set_visible (trayicon->self, FALSE);
#endif
	ug_trayicon_set_info (trayicon, 0, 0, 0);
}

// ----------------------------------------------------------------------------
// UgWindow
//
static void ug_window_init  (struct UgWindow* window, UgAppGtk* app)
{
	GtkBox*			vbox;
	GtkBox*			rbox;

	window->self = (GtkWindow*) gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (window->self, UG_APP_GTK_NAME);
	gtk_window_set_default_size (window->self, 620, 400);
	gtk_window_add_accel_group (window->self, app->accel_group);
	gtk_window_set_default_icon_name (UG_APP_GTK_ICON_NAME);

	// top container for Main Window
	vbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add (GTK_CONTAINER (window->self), GTK_WIDGET (vbox));
	gtk_box_pack_start (vbox, app->menubar.self, FALSE, FALSE, 0);
	// right side vbox
	rbox = (GtkBox*) gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (rbox, app->toolbar.self, FALSE, FALSE, 0);
	// hpaned
	window->hpaned = (GtkPaned*) gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_box_pack_start (vbox, GTK_WIDGET (window->hpaned), TRUE, TRUE, 0);
	gtk_paned_pack1 (window->hpaned, app->cwidget.self, FALSE, TRUE);
	gtk_paned_pack2 (window->hpaned, GTK_WIDGET (rbox), TRUE, FALSE);
	// vpaned
	window->vpaned = (GtkPaned*) gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	gtk_box_pack_start (rbox, (GtkWidget*) window->vpaned, TRUE, TRUE, 0);
//	gtk_paned_pack1 (window->vpaned, GTK_WIDGET (app->cwidget.primary->all.self), TRUE, TRUE);
	gtk_paned_pack2 (window->vpaned, GTK_WIDGET (app->summary.self), FALSE, TRUE);

	gtk_box_pack_start (vbox, GTK_WIDGET (app->statusbar.self), FALSE, FALSE, 0);
	gtk_widget_show_all ((GtkWidget*) vbox);
}

// ----------------------------------------------------------------------------
// UgStatusbar
//
static void ug_statusbar_init (struct UgStatusbar* sbar)
{
	GtkBox*		hbox;
	GtkWidget*	widget;

	sbar->self = (GtkStatusbar*) gtk_statusbar_new ();
	hbox = GTK_BOX (sbar->self);

#if GTK_MAJOR_VERSION < 3
	gtk_statusbar_set_has_resize_grip (sbar->self, FALSE);
#endif

	// upload speed label
	widget = gtk_label_new ("");
	sbar->up_speed = (GtkLabel*) widget;
	gtk_widget_set_size_request (widget, 100, 0);
	gtk_box_pack_end (hbox, widget, FALSE, TRUE, 2);
//	gtk_label_set_width_chars (sbar->down_speed, 15);
	gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);
	gtk_box_pack_end (hbox, gtk_label_new ("U:"), FALSE, TRUE, 2);

	gtk_box_pack_end (hbox, gtk_separator_new (GTK_ORIENTATION_VERTICAL), FALSE, TRUE, 8);

	// download speed label
	widget = gtk_label_new ("");
	sbar->down_speed = (GtkLabel*) widget;
	gtk_widget_set_size_request (widget, 100, 0);
	gtk_box_pack_end (hbox, widget, FALSE, TRUE, 2);
//	gtk_label_set_width_chars (sbar->down_speed, 15);
	gtk_misc_set_alignment (GTK_MISC (widget), 0, 0.5);
	gtk_box_pack_end (hbox, gtk_label_new ("D:"), FALSE, TRUE, 2);

	ug_statusbar_set_speed (sbar, 0, 0);
}

// ----------------------------------------------------------------------------
// UgToolbar
//
static void ug_toolbar_init (struct UgToolbar* ugt, GtkAccelGroup* accel_group)
{
	GtkToolbar*		toolbar;
	GtkToolItem*	tool_item;
	GtkWidget*		image;
	GtkWidget*		menu;
	GtkWidget*		menu_item;

	// toolbar
	ugt->self = gtk_toolbar_new ();
	toolbar = GTK_TOOLBAR (ugt->self);
	gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_icon_size (toolbar, GTK_ICON_SIZE_SMALL_TOOLBAR);

	// New button --- start ---
	tool_item = (GtkToolItem*) gtk_menu_tool_button_new_from_stock (GTK_STOCK_NEW);
	gtk_tool_item_set_tooltip_text (tool_item, _("Create new download"));
	gtk_menu_tool_button_set_arrow_tooltip_text ((GtkMenuToolButton*)tool_item, "Create new item");
	gtk_tool_item_set_homogeneous (tool_item, FALSE);
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->create = GTK_WIDGET (tool_item);
	// menu for tool button (accelerators)
	menu = gtk_menu_new ();
	gtk_menu_set_accel_group ((GtkMenu*) menu, accel_group);
	gtk_menu_tool_button_set_menu ((GtkMenuToolButton*)tool_item, menu);
	// New Download (accelerators)
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Download..."));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UG_APP_GTK_ACCEL_PATH_CTRL_N);
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_download = menu_item;
	// New Category
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Category..."));
	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_category = menu_item;
	// New Batch download
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Batch download..."));
	image = gtk_image_new_from_stock (GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_batch = menu_item;
	gtk_widget_show_all (menu);
	// New from Clipboard
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _from Clipboard..."));
	image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_clipboard = menu_item;
	gtk_widget_show_all (menu);

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// New Torrent
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Torrent..."));
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_torrent = menu_item;
	gtk_widget_show_all (menu);
	// New Metalink
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New Metalink..."));
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	ugt->create_metalink = menu_item;
	gtk_widget_show_all (menu);
	// New button --- end ---

	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_SAVE);
	gtk_tool_item_set_tooltip_text (tool_item, _("Save all"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->save = GTK_WIDGET (tool_item);

	tool_item = gtk_separator_tool_item_new ();
	gtk_toolbar_insert (toolbar, tool_item, -1);

	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_PLAY);
	gtk_tool_item_set_tooltip_text (tool_item, _("Set selected download runnable"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->runnable = GTK_WIDGET (tool_item);

	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_MEDIA_PAUSE);
	gtk_tool_item_set_tooltip_text (tool_item, _("Set selected download to pause"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->pause = GTK_WIDGET (tool_item);

	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_PROPERTIES);
	gtk_tool_item_set_tooltip_text (tool_item, _("Set selected download properties"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->properties = GTK_WIDGET (tool_item);

	tool_item = gtk_separator_tool_item_new ();
	gtk_toolbar_insert (toolbar, tool_item, -1);

	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_GO_UP);
	gtk_tool_item_set_tooltip_text (tool_item, _("Move selected download up"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->move_up = GTK_WIDGET (tool_item);

	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_GO_DOWN);
	gtk_tool_item_set_tooltip_text (tool_item, _("Move selected download down"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->move_down = GTK_WIDGET (tool_item);

	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_TOP);
	gtk_tool_item_set_tooltip_text (tool_item, _("Move selected download to top"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->move_top = GTK_WIDGET (tool_item);

	tool_item = (GtkToolItem*) gtk_tool_button_new_from_stock (GTK_STOCK_GOTO_BOTTOM);
	gtk_tool_item_set_tooltip_text (tool_item, _("Move selected download to bottom"));
	gtk_toolbar_insert (toolbar, tool_item, -1);
	ugt->move_bottom = GTK_WIDGET (tool_item);

	gtk_widget_show_all ((GtkWidget*) toolbar);
}

// ----------------------------------------------------------------------------
// UgMenubar
//
static void ug_menubar_init (struct UgMenubar* menubar, GtkAccelGroup* accel_group)
{
	GtkWidget*		image;
	GtkWidget*		menu;
	GtkWidget*		sub_menu;
	GtkWidget*		menu_item;

	// Menubar
	menubar->self = gtk_menu_bar_new ();

	// ----------------------------------------------------
	// UgFileMenu
	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_File"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);
//	menu.gtk_menu_shell_append((GtkMenuShell*)menu, gtk_tearoff_menu_item_new() );

	// New --- start --- (accelerators)
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW, NULL);
	sub_menu = gtk_menu_new ();
	gtk_menu_set_accel_group ((GtkMenu*)sub_menu, accel_group);
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, sub_menu);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	// New - Download (accelerators)
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Download..."));
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UG_APP_GTK_ACCEL_PATH_CTRL_N);
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)sub_menu, menu_item);
	menubar->file.create.download = menu_item;
	// New - Category
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Category..."));
	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)sub_menu, menu_item);
	menubar->file.create.category = menu_item;
	// New - Batch download
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Batch download..."));
	image = gtk_image_new_from_stock (GTK_STOCK_SORT_ASCENDING, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)sub_menu, menu_item);
	menubar->file.create.batch = menu_item;
	// New - from clipboard
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_From Clipboard..."));
	image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)sub_menu, menu_item);
	menubar->file.create.from_clipboard = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)sub_menu, gtk_separator_menu_item_new() );

	// New - Torrent
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("Torrent..."));
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)sub_menu, menu_item);
	menubar->file.create.torrent = menu_item;
	// New - Metalink
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("Metalink..."));
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)sub_menu, menu_item);
	menubar->file.create.metalink = menu_item;
	// New --- end ---

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Save
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_SAVE, accel_group);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.save = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_image_menu_item_new_with_mnemonic (_("Import _HTML file..."));
	image = gtk_image_new_from_stock (GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.import_html = menu_item;

	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Import text file..."));
	image = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.import_text = menu_item;

	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Export text file..."));
	image = gtk_image_new_from_stock (GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.export_text = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Offline mode
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Offline Mode"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.offline_mode = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, accel_group);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->file.quit = menu_item;

	// ----------------------------------------------------
	// UgEditMenu
	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Edit"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);
//	menu.gtk_menu_shell_append((GtkMenuShell*)menu, gtk_tearoff_menu_item_new() );

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Clipboard _Monitor"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.clipboard_monitor = menu_item;

	menu_item = gtk_menu_item_new_with_mnemonic (_("_Clipboard Option..."));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.clipboard_option = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Shutdown when downloads complete"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.shutdown = menu_item;

//	menu_item = gtk_menu_item_new_with_mnemonic (_("_Settings..."));
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Settings..."));
	image = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->edit.settings = menu_item;

	// ----------------------------------------------------
	// UgViewMenu
	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_View"));
	gtk_menu_item_set_submenu ((GtkMenuItem*) menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*) menubar->self, menu_item);

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Toolbar"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.toolbar = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Statusbar"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.statusbar = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Category"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.category = menu_item;

	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Summary"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.summary = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Summary Items --- start ---
	menu_item = gtk_menu_item_new_with_mnemonic (_("Summary _Items"));
	sub_menu  = gtk_menu_new ();
	gtk_menu_item_set_submenu ((GtkMenuItem*) menu_item, sub_menu);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	// Summary Items - Name
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Name"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	menubar->view.summary_items.name = menu_item;
	// Summary Items - Folder
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Folder"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	menubar->view.summary_items.folder = menu_item;
	// Summary Items - Category
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Category"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	menubar->view.summary_items.category = menu_item;
	// Summary Items - Elapsed
//	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Elapsed"));
//	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
//	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
//	menubar->view.summary_items.elapsed = menu_item;
	// Summary Items - URL
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_URL"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	menubar->view.summary_items.url = menu_item;
	// Summary Items - Message
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Message"));
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	menubar->view.summary_items.message = menu_item;
	// Summary Items --- end ---

//	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

//	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Download _Rules Hint"));
//	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
//	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
//	menubar->view.rules_hint = menu_item;

	// Download Columns --- start ---
	sub_menu  = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("Download _Columns"));
	gtk_menu_item_set_submenu ((GtkMenuItem*) menu_item, sub_menu);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->view.columns.self = sub_menu;
	// Download Columns - Completed
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Complete"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.completed = menu_item;
	// Download Columns - Total
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Size"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.total = menu_item;
	// Download Columns - Percent (%)
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Percent '%'"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.percent = menu_item;
	// Download Columns - Elapsed
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Elapsed"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.elapsed = menu_item;
	// Download Columns - Left
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Left"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.left = menu_item;
	// Download Columns - Speed
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Speed"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.speed = menu_item;
	// Download Columns - Up Speed
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Up Speed"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.upload_speed = menu_item;
	// Download Columns - Uploaded
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Uploaded"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.uploaded = menu_item;
	// Download Columns - Ratio
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Ratio"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.ratio = menu_item;
	// Download Columns - Retry
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Retry"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.retry = menu_item;
	// Download Columns - Category
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Category"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.category = menu_item;
	// Download Columns - URL
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_URL"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.url = menu_item;
	// Download Columns - Added On
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Added On"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.added_on = menu_item;
	// Download Columns - Completed On
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("Completed On"));
	gtk_menu_shell_append ((GtkMenuShell*) sub_menu, menu_item);
	gtk_check_menu_item_set_active ((GtkCheckMenuItem*) menu_item, TRUE);
	menubar->view.columns.completed_on = menu_item;
	// Download Columns --- end ---

	// ----------------------------------------------------
	// UgCategoryMenu
	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Category"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);
	menubar->category.self = menu;
	// New Category
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_New Category..."));
	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->category.create = menu_item;
	// Delete Category
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Delete Category"));
	image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->category.delete = menu_item;
	// Properties
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PROPERTIES, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->category.properties = menu_item;

	// ----------------------------------------------------
	// UgDownloadMenu
	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Download"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);
	menubar->download.self = menu;

//	gtk_menu_shell_append((GtkMenuShell*)menu, gtk_tearoff_menu_item_new() );

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_NEW, accel_group);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.create = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_DELETE, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Delete"));
//	image = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.delete = menu_item;

	menu_item = gtk_image_menu_item_new_with_mnemonic (_("Delete _File and Data"));
//	image = gtk_image_new_from_stock (GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.delete_file = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_OPEN, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.open = menu_item;
	gtk_widget_hide (menu_item);

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Open _containing folder"));
	image = gtk_image_new_from_stock (GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.open_folder = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Force Start"));
//	image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.force_start = menu_item;

	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Runnable"));
	image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.runnable = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_MEDIA_PAUSE, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("P_ause"));
//	image = gtk_image_new_from_stock (GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.pause = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Move to --- start ---
	menu_item = gtk_image_menu_item_new_with_mnemonic(_("_Move To"));
	image = gtk_image_new_from_stock (GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_to.item = menu_item;
	// Move to - submenu
	sub_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, sub_menu);
	menubar->download.move_to.self = sub_menu;
	menubar->download.move_to.array = g_ptr_array_sized_new (16*2);
	// Move to --- end ---

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GO_UP, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Up"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_up = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GO_DOWN, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Down"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_down = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GOTO_TOP, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Top"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GOTO_TOP, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_top = menu_item;

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_GOTO_BOTTOM, NULL);
//	menu_item = gtk_image_menu_item_new_with_mnemonic(_("Move _Bottom"));
//	image = gtk_image_new_from_stock (GTK_STOCK_GOTO_BOTTOM, GTK_ICON_SIZE_MENU);
//	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.move_bottom = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PROPERTIES, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->download.properties = menu_item;

	// ----------------------------------------------------
	// UgHelpMenu
	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);

	// About Uget
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.about_uget = menu_item;
}

