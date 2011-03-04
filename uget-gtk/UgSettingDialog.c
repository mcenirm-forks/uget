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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <UgSettingDialog.h>
#include <UgUtils.h>

#include <glib/gi18n.h>


UgSettingDialog*	ug_setting_dialog_new (const gchar* title, GtkWindow* parent)
{
	UgSettingDialog*	dialog;
	GtkWidget*			widget;
	GtkBox*				vbox;

	dialog = g_malloc0 (sizeof (UgSettingDialog));
	dialog->self = (GtkDialog*) gtk_dialog_new_with_buttons (title, parent,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK,     GTK_RESPONSE_OK,
			NULL);
	gtk_dialog_set_default_response (dialog->self, GTK_RESPONSE_OK);
	widget = gtk_notebook_new ();
	gtk_widget_set_size_request (widget, 410, 300);
	vbox = (GtkBox*) gtk_dialog_get_content_area (dialog->self);
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 0);
	dialog->notebook = (GtkNotebook*) widget;

	// ------------------------------------------------------------------------
	// UI settings page
	vbox = (GtkBox*) gtk_vbox_new (FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_notebook_append_page (dialog->notebook, (GtkWidget*) vbox,
			gtk_label_new (_("UI Settings")));
	ug_user_interface_form_init (&dialog->ui);
	gtk_box_pack_start (vbox, dialog->ui.self, FALSE, FALSE, 2);

	// ------------------------------------------------------------------------
	// Clipboard settings page
	ug_clipboard_setting_form_init (&dialog->clipboard);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->clipboard.self), 2);
	gtk_notebook_append_page (dialog->notebook, dialog->clipboard.self,
			gtk_label_new (_("Clipboard")));

	// ------------------------------------------------------------------------
	// Scheduler settings page
	ug_schedule_grid_init (&dialog->scheduler);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->scheduler.self), 2);
	gtk_notebook_append_page (dialog->notebook, dialog->scheduler.self,
			gtk_label_new (_("Scheduler")));

#ifdef HAVE_PLUGIN_ARIA2
	// ------------------------------------------------------------------------
	// Plugin settings page
	ug_plugin_setting_form_init (&dialog->plugin);
	gtk_container_set_border_width (GTK_CONTAINER (dialog->plugin.self), 2);
	gtk_notebook_append_page (dialog->notebook, dialog->plugin.self,
			gtk_label_new (_("Plug-in")));
#endif	// HAVE_PLUGIN_ARIA2

	// ------------------------------------------------------------------------
	// Others settings page
	vbox = (GtkBox*) gtk_vbox_new (FALSE, 2);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_notebook_append_page (dialog->notebook, (GtkWidget*) vbox,
			gtk_label_new (_("Others")));
	ug_auto_save_form_init (&dialog->auto_save);
	gtk_box_pack_start (vbox, dialog->auto_save.self, FALSE, FALSE, 2);
	gtk_box_pack_start (vbox, gtk_hseparator_new (), FALSE, FALSE, 2);
	ug_launch_setting_form_init (&dialog->launch);
	gtk_box_pack_start (vbox, dialog->launch.self, FALSE, FALSE, 2);

	gtk_widget_show_all ((GtkWidget*) dialog->notebook);
//	gtk_container_set_focus_child (GTK_CONTAINER (dialog->self), dialog->pattern_entry);
//	g_signal_connect (dialog->pattern_entry, "key-press-event", G_CALLBACK (on_key_press_event), dialog);

	return dialog;
}

void	ug_setting_dialog_free (UgSettingDialog* dialog)
{
	gtk_widget_destroy ((GtkWidget*) dialog->self);
	g_free (dialog);
}

void	ug_setting_dialog_set (UgSettingDialog* dialog, UgetGtkSetting* setting)
{
	ug_schedule_grid_set (&dialog->scheduler, setting);
	ug_clipboard_setting_form_set (&dialog->clipboard, setting);
	ug_user_interface_form_set (&dialog->ui, setting);
	ug_launch_setting_form_set (&dialog->launch, setting);
	ug_auto_save_form_set (&dialog->auto_save, setting);
#ifdef HAVE_PLUGIN_ARIA2
	ug_plugin_setting_form_set (&dialog->plugin, setting);
#endif
}

void	ug_setting_dialog_get (UgSettingDialog* dialog, UgetGtkSetting* setting)
{
	ug_schedule_grid_get (&dialog->scheduler, setting);
	ug_clipboard_setting_form_get (&dialog->clipboard, setting);
	ug_user_interface_form_get (&dialog->ui, setting);
	ug_launch_setting_form_get (&dialog->launch, setting);
	ug_auto_save_form_get (&dialog->auto_save, setting);
#ifdef HAVE_PLUGIN_ARIA2
	ug_plugin_setting_form_get (&dialog->plugin, setting);
#endif
}

