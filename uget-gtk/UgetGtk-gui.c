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

#include <gdk/gdkkeysyms.h>    // for GDK_KEY_key...
// for GTK+ 2.18
#ifndef GDK_KEY_n
#define GDK_KEY_n			GDK_n
#endif

// uglib
#include <UgetGtk.h>

#include <glib/gi18n.h>

static void	uget_gtk_tray_icon_init	(struct UgetGtkTrayIcon* ugtk_tray_icon);
static void	uget_gtk_window_init	(struct UgetGtkWindow* ugtk_window, UgetGtk* ugtk);
static void	uget_gtk_statusbar_init	(struct UgetGtkStatusbar* ugtk_statusbar);
static void	uget_gtk_toolbar_init	(struct UgetGtkToolbar* ugtk_toolbar, GtkAccelGroup* accel_group);
static void	uget_gtk_menubar_init	(struct UgetGtkMenubar* ugtk_menubar, GtkAccelGroup* accel_group);

void	uget_gtk_init_gui (UgetGtk* ugtk)
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
	gtk_accel_map_add_entry (UGET_GTK_ACCEL_PATH_CTRL_N, GDK_KEY_n, GDK_CONTROL_MASK);
	// accelerators
	ugtk->accel_group = gtk_accel_group_new ();
	// tray icon
	uget_gtk_tray_icon_init (&ugtk->tray_icon);
	// main window
	ug_category_widget_init (&ugtk->cwidget);
	ug_summary_init (&ugtk->summary, ugtk->accel_group);
	uget_gtk_statusbar_init (&ugtk->statusbar);
	uget_gtk_toolbar_init (&ugtk->toolbar, ugtk->accel_group);
	uget_gtk_menubar_init (&ugtk->menubar, ugtk->accel_group);
	uget_gtk_window_init  (&ugtk->window, ugtk);
}

// ----------------------------------------------------------------------------
// UgetGtkTrayIcon
//
static void uget_gtk_tray_icon_init (struct UgetGtkTrayIcon* icon)
{
	GtkWidget*		image;
	GtkWidget*		menu;
	GtkWidget*		menu_item;
	gchar*			file;

	file = g_build_filename (ug_get_data_dir (), "icons",
	                         "hicolor", "16x16", "apps",
	                         "uget-icon.png", NULL);
	if (g_file_test (file, G_FILE_TEST_IS_REGULAR))
		icon->self = gtk_status_icon_new_from_icon_name (UGET_GTK_ICON_NAME);
	else
		icon->self = gtk_status_icon_new_from_stock (GTK_STOCK_GO_DOWN);
	g_free (file);
	gtk_status_icon_set_visible (icon->self, FALSE);
	uget_gtk_tray_icon_refresh (icon, 0, 0.0);

	// UgetGtkTrayIcon.menu
	menu = gtk_menu_new ();
	// New Download
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _Download..."));
	image = gtk_image_new_from_stock (GTK_STOCK_FILE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	icon->menu.create_download = menu_item;

	// New Download from Clipboard
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("New _from Clipboard..."));
	image = gtk_image_new_from_stock (GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	icon->menu.create_clipboard = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Settings
	menu_item = gtk_image_menu_item_new_with_mnemonic (_("_Settings..."));
	image = gtk_image_new_from_stock (GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image ((GtkImageMenuItem*)menu_item, image);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	icon->menu.settings = menu_item;

	// About
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	icon->menu.about = menu_item;

	gtk_menu_shell_append ((GtkMenuShell*)menu, gtk_separator_menu_item_new() );

	// Offline mode
	menu_item = gtk_check_menu_item_new_with_mnemonic (_("_Offline Mode"));
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	icon->menu.offline_mode = menu_item;

	// Quit
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_QUIT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	icon->menu.quit = menu_item;

	gtk_widget_show_all (menu);
	icon->menu.self = menu;
}

// ----------------------------------------------------------------------------
// UgetGtkWindow
//
static void uget_gtk_window_init  (struct UgetGtkWindow* window, UgetGtk* ugtk)
{
	GtkBox*			vbox;
	GtkBox*			rbox;

	window->self = (GtkWindow*) gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (window->self, UGET_GTK_NAME);
	gtk_window_set_default_size (window->self, 620, 400);
	gtk_window_add_accel_group (window->self, ugtk->accel_group);
	gtk_window_set_default_icon_name (UGET_GTK_ICON_NAME);

	// top container for Main Window
	vbox = (GtkBox*) gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window->self), GTK_WIDGET (vbox));
	gtk_box_pack_start (vbox, ugtk->menubar.self, FALSE, FALSE, 0);
	// right side vbox
	rbox = (GtkBox*) gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (rbox, ugtk->toolbar.self, FALSE, FALSE, 0);
	// hpaned
	window->hpaned = (GtkPaned*) gtk_hpaned_new ();
	gtk_box_pack_start (vbox, GTK_WIDGET (window->hpaned), TRUE, TRUE, 0);
	gtk_paned_pack1 (window->hpaned, ugtk->cwidget.self, FALSE, TRUE);
	gtk_paned_pack2 (window->hpaned, GTK_WIDGET (rbox), TRUE, FALSE);
	// vpaned
	window->vpaned = (GtkPaned*) gtk_vpaned_new ();
	gtk_box_pack_start (rbox, (GtkWidget*) window->vpaned, TRUE, TRUE, 0);
//	gtk_paned_pack1 (window->vpaned, GTK_WIDGET (ugtk->cwidget.primary->all.self), TRUE, TRUE);
	gtk_paned_pack2 (window->vpaned, GTK_WIDGET (ugtk->summary.self), FALSE, TRUE);

	gtk_box_pack_start (vbox, GTK_WIDGET (ugtk->statusbar.self), FALSE, FALSE, 0);
	gtk_widget_show_all ((GtkWidget*) vbox);
}

// ----------------------------------------------------------------------------
// UgetGtkStatusbar
//
static void uget_gtk_statusbar_init (struct UgetGtkStatusbar* sbar)
{
	sbar->self = (GtkStatusbar*) gtk_statusbar_new ();

	sbar->speed = (GtkLabel*) gtk_label_new ("");
//	gtk_label_set_width_chars (sbar->speed, 15);
//	gtk_label_set_justify (sbar->speed, GTK_JUSTIFY_RIGHT);
	gtk_box_pack_end (GTK_BOX (sbar->self), (GtkWidget*) sbar->speed,
			FALSE, TRUE, 2);

	uget_gtk_statusbar_refresh_speed (sbar, 0.0);
}

// ----------------------------------------------------------------------------
// UgetGtkToolbar
//
static void uget_gtk_toolbar_init (struct UgetGtkToolbar* ugt, GtkAccelGroup* accel_group)
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
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGET_GTK_ACCEL_PATH_CTRL_N);
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
// UgetGtkMenubar
//
static void uget_gtk_menubar_init (struct UgetGtkMenubar* menubar, GtkAccelGroup* accel_group)
{
	GtkWidget*		image;
	GtkWidget*		menu;
	GtkWidget*		sub_menu;
	GtkWidget*		menu_item;

	// Menubar
	menubar->self = gtk_menu_bar_new ();

	// ----------------------------------------------------
	// UgetGtkFileMenu
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
	gtk_menu_item_set_accel_path ((GtkMenuItem*) menu_item, UGET_GTK_ACCEL_PATH_CTRL_N);
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
	// UgetGtkEditMenu
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
	// UgetGtkViewMenu
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
	menubar->view.columns.up_speed = menu_item;
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
	// UgetGtkCategoryMenu
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
	// UgetGtkDownloadMenu
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
	// UgetGtkHelpMenu
	menu = gtk_menu_new ();
	menu_item = gtk_menu_item_new_with_mnemonic(_("_Help"));
	gtk_menu_item_set_submenu ((GtkMenuItem*)menu_item, menu);
	gtk_menu_shell_append ((GtkMenuShell*)menubar->self, menu_item);

	// About Uget
	menu_item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	gtk_menu_shell_append ((GtkMenuShell*)menu, menu_item);
	menubar->help.about_uget = menu_item;
}

