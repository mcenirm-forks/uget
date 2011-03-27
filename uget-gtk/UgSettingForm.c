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

#include <UgSettingForm.h>

#include <glib/gi18n.h>

// ----------------------------------------------------------------------------
// UgClipboardSettingForm
//
static void	on_clipboard_monitor_toggled (GtkWidget* widget, struct UgClipboardSettingForm* csform)
{
	gboolean	sensitive;

	sensitive = gtk_toggle_button_get_active (csform->monitor);
	gtk_widget_set_sensitive ((GtkWidget*) csform->pattern, sensitive);
}

static void	on_clipboard_quiet_mode_toggled (GtkWidget* widget, struct UgClipboardSettingForm* csform)
{
	gboolean	sensitive;

	sensitive = gtk_toggle_button_get_active (csform->quiet);
	gtk_widget_set_sensitive ((GtkWidget*) csform->nth_label, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) csform->nth_spin,  sensitive);
}

void	ug_clipboard_setting_form_init (struct UgClipboardSettingForm* csform)
{
	GtkWidget*			widget;
	GtkWidget*			entry;
	GtkBox*				vbox;
	GtkBox*				hbox;

	csform->self = gtk_vbox_new (FALSE, 0);
	vbox = (GtkBox*) csform->self;
	// Monitor button
	widget = gtk_check_button_new_with_mnemonic (
			_("_Monitor clipboard for specified file types:"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_clipboard_monitor_toggled), csform);
	csform->monitor = (GtkToggleButton*) widget;

	// file type pattern : entry
	entry = gtk_entry_new ();
	gtk_box_pack_start (vbox, entry, FALSE, FALSE, 2);
	csform->pattern = (GtkEntry*) entry;

	// tips
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	gtk_box_pack_end (hbox,
			gtk_label_new (_("Separate the types with character '|'.")),
			FALSE, FALSE, 2);
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	gtk_box_pack_end (hbox,
			gtk_label_new (_("You can use regular expressions here.")),
			FALSE, FALSE, 2);

	gtk_box_pack_start (vbox, gtk_label_new (""),
			FALSE, FALSE, 2);
	// quiet mode
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 2);
	widget = gtk_check_button_new_with_mnemonic (_("_Quiet mode"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_clipboard_quiet_mode_toggled), csform);
	csform->quiet = (GtkToggleButton*) widget;
	// Nth category
	widget = gtk_spin_button_new_with_range (-1.0, 100.0, 1.0);
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	csform->nth_spin = (GtkSpinButton*) widget;
	widget = gtk_label_new (_("Add URL to Nth category"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	csform->nth_label = widget;
}

void	ug_clipboard_setting_form_set (struct UgClipboardSettingForm* csform, UgSetting* setting)
{
	if (setting->clipboard.pattern)
		gtk_entry_set_text (csform->pattern, setting->clipboard.pattern);
	gtk_toggle_button_set_active (csform->monitor, setting->clipboard.monitor);
	gtk_toggle_button_set_active (csform->quiet, setting->clipboard.quiet);
	gtk_spin_button_set_value (csform->nth_spin, setting->clipboard.nth_category);
	gtk_toggle_button_toggled (csform->monitor);
	gtk_toggle_button_toggled (csform->quiet);
//	on_clipboard_monitor_toggled ((GtkWidget*) csform->monitor, csform);
//	on_clipboard_quiet_mode_toggled ((GtkWidget*) csform->quiet, csform);
}

void	ug_clipboard_setting_form_get (struct UgClipboardSettingForm* csform, UgSetting* setting)
{
	g_free (setting->clipboard.pattern);
	setting->clipboard.pattern = g_strdup (gtk_entry_get_text (csform->pattern));
	setting->clipboard.monitor = gtk_toggle_button_get_active (csform->monitor);
	setting->clipboard.quiet = gtk_toggle_button_get_active (csform->quiet);
	setting->clipboard.nth_category = gtk_spin_button_get_value_as_int (csform->nth_spin);
}

// ----------------------------------------------------------------------------
// UgUserInterfaceForm
//
void	ug_user_interface_form_init (struct UgUserInterfaceForm* uiform)
{
	GtkWidget*	widget;
	GtkBox*		vbox;

	uiform->self = gtk_vbox_new (FALSE, 0);
	vbox = (GtkBox*) uiform->self;
	// check button
	widget = gtk_check_button_new_with_label (_("Show confirmation dialog on close"));
	uiform->confirm_close = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Confirm when deleting files"));
	uiform->confirm_delete = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Always show tray icon"));
	uiform->show_trayicon = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Minimize to tray on startup"));
	uiform->start_in_tray = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Enable offline mode on startup"));
	uiform->start_in_offline_mode = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Download starting notification"));
	uiform->start_notification = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_label (_("Sound when download is finished"));
	uiform->sound_notification = (GtkToggleButton*) widget;
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 1);
}

void	ug_user_interface_form_set (struct UgUserInterfaceForm* uiform, UgSetting* setting)
{
	gtk_toggle_button_set_active (uiform->confirm_close,
			setting->ui.close_confirmation);
	gtk_toggle_button_set_active (uiform->confirm_delete,
			setting->ui.delete_confirmation);
	gtk_toggle_button_set_active (uiform->show_trayicon,
			setting->ui.show_trayicon);
	gtk_toggle_button_set_active (uiform->start_in_tray,
			setting->ui.start_in_tray);
	gtk_toggle_button_set_active (uiform->start_in_offline_mode,
			setting->ui.start_in_offline_mode);
	gtk_toggle_button_set_active (uiform->start_notification,
			setting->ui.start_notification);
	gtk_toggle_button_set_active (uiform->sound_notification,
			setting->ui.sound_notification);
}

void	ug_user_interface_form_get (struct UgUserInterfaceForm* uiform, UgSetting* setting)
{
	setting->ui.close_confirmation = gtk_toggle_button_get_active (uiform->confirm_close);
	setting->ui.delete_confirmation = gtk_toggle_button_get_active (uiform->confirm_delete);
	setting->ui.show_trayicon = gtk_toggle_button_get_active (uiform->show_trayicon);
	setting->ui.start_in_tray = gtk_toggle_button_get_active (uiform->start_in_tray);
	setting->ui.start_in_offline_mode = gtk_toggle_button_get_active (uiform->start_in_offline_mode);
	setting->ui.start_notification = gtk_toggle_button_get_active (uiform->start_notification);
	setting->ui.sound_notification = gtk_toggle_button_get_active (uiform->sound_notification);
}

// ----------------------------------------------------------------------------
// UgLaunchSettingForm
//
static void	on_launch_app_toggled (GtkWidget* widget, struct UgLaunchSettingForm* lsform)
{
	gboolean	sensitive;

	sensitive = gtk_toggle_button_get_active (lsform->active);
	gtk_widget_set_sensitive ((GtkWidget*) lsform->types, sensitive);
}

void	ug_launch_setting_form_init (struct UgLaunchSettingForm* lsform)
{
	GtkWidget*			widget;
	GtkWidget*			entry;
	GtkBox*				vbox;
	GtkBox*				hbox;

	lsform->self = gtk_vbox_new (FALSE, 0);
	vbox = (GtkBox*) lsform->self;
	// active button
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	widget = gtk_check_button_new_with_mnemonic (
			_("_Launch default application for specified file types:"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_launch_app_toggled), lsform);
	lsform->active = (GtkToggleButton*) widget;
	// launch app entry
	entry = gtk_entry_new ();
	gtk_box_pack_start (vbox, entry, FALSE, FALSE, 2);
	lsform->types = (GtkEntry*) entry;
	// Launch app tips
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	gtk_box_pack_end (hbox,
			gtk_label_new (_("Separate the types with character '|'.")),
			FALSE, FALSE, 2);
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, FALSE, 1);
	gtk_box_pack_end (hbox,
			gtk_label_new (_("You can use regular expressions here.")),
			FALSE, FALSE, 2);
}

void	ug_launch_setting_form_set (struct UgLaunchSettingForm* lsform, UgSetting* setting)
{
	// launch app
	gtk_toggle_button_set_active (lsform->active, setting->launch.active);
	if (setting->launch.types)
		gtk_entry_set_text (lsform->types, setting->launch.types);
	gtk_toggle_button_toggled (lsform->active);
//	on_launch_app_toggled ((GtkWidget*) lsform->active, lsform);
}

void	ug_launch_setting_form_get (struct UgLaunchSettingForm* lsform, UgSetting* setting)
{
	setting->launch.active = gtk_toggle_button_get_active (lsform->active);
	g_free (setting->launch.types);
	setting->launch.types = g_strdup (gtk_entry_get_text (lsform->types));
}

// ----------------------------------------------------------------------------
// UgAutoSaveForm
//
static void	on_auto_save_toggled (GtkWidget* widget, struct UgAutoSaveForm* asform)
{
	gboolean	sensitive;

	sensitive = gtk_toggle_button_get_active (asform->active);
	gtk_widget_set_sensitive (asform->interval_label, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) asform->interval_spin, sensitive);
	gtk_widget_set_sensitive (asform->minutes_label, sensitive);
}

void	ug_auto_save_form_init (struct UgAutoSaveForm* asform)
{
	GtkBox*				hbox;
	GtkWidget*			widget;

	asform->self = gtk_hbox_new (FALSE, 0);
	hbox = (GtkBox*) asform->self;
	widget = gtk_check_button_new_with_mnemonic (_("_Auto save"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_auto_save_toggled), asform);
	asform->active = (GtkToggleButton*) widget;
	// auto save spin & label (interval)
	widget = gtk_label_new_with_mnemonic (_("minutes"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	asform->minutes_label = widget;
	widget = gtk_spin_button_new_with_range (1.0, 120.0, 1.0);
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	asform->interval_spin = (GtkSpinButton*) widget;
	// auto save label
	widget = gtk_label_new_with_mnemonic (_("_Interval:"));
	gtk_box_pack_end (hbox, widget, FALSE, FALSE, 2);
	asform->interval_label = widget;
	gtk_label_set_mnemonic_widget (GTK_LABEL (asform->interval_label),
			(GtkWidget*) asform->interval_spin);
}

void	ug_auto_save_form_set (struct UgAutoSaveForm* asform, UgSetting* setting)
{
	gtk_toggle_button_set_active (asform->active, setting->auto_save.active);
	gtk_spin_button_set_value (asform->interval_spin, (gdouble) setting->auto_save.interval);
	gtk_toggle_button_toggled (asform->active);
//	on_auto_save_toggled ((GtkWidget*) asform->active, asform);
}

void	ug_auto_save_form_get (struct UgAutoSaveForm* asform, UgSetting* setting)
{
	setting->auto_save.active = gtk_toggle_button_get_active (asform->active);
	setting->auto_save.interval = gtk_spin_button_get_value_as_int (asform->interval_spin);
}

// ----------------------------------------------------------------------------
// UgPluginSettingForm
//
static void	on_plugin_aria2_toggled (GtkWidget* widget, struct UgPluginSettingForm* psform)
{
	gboolean	sensitive;

	sensitive = gtk_toggle_button_get_active (psform->enable);
	gtk_widget_set_sensitive ((GtkWidget*) psform->launch, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) psform->shutdown, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) psform->uri, sensitive);

	if (sensitive)
		sensitive = gtk_toggle_button_get_active (psform->launch);
	gtk_widget_set_sensitive ((GtkWidget*) psform->path, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) psform->args, sensitive);
}

static void	on_plugin_aria2_launch_toggled (GtkWidget* widget, struct UgPluginSettingForm* psform)
{
	gboolean	sensitive;

	sensitive = gtk_toggle_button_get_active (psform->launch);
	gtk_widget_set_sensitive ((GtkWidget*) psform->path, sensitive);
	gtk_widget_set_sensitive ((GtkWidget*) psform->args, sensitive);
}

void	ug_plugin_setting_form_init (struct UgPluginSettingForm* psform)
{
	GtkBox*				vbox;
	GtkBox*				hbox;
	GtkWidget*			widget;

	psform->self = gtk_vbox_new (FALSE, 0);
	vbox = (GtkBox*) psform->self;

	hbox = (GtkBox*) gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, TRUE, 2);

	widget = gtk_check_button_new_with_mnemonic (_("_Enable aria2 plug-in"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	gtk_box_pack_start (hbox, gtk_hseparator_new (), TRUE, TRUE, 2);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_plugin_aria2_toggled), psform);
	psform->enable = (GtkToggleButton*) widget;

	// hint
	widget = gtk_label_new (_("Make sure that all of the downloads have been completed."));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 3);
	widget = gtk_label_new ("");
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 3);

	// URI entry
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, TRUE, 2);
	widget = gtk_label_new ("URI");
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_entry_new ();
	gtk_box_pack_start (hbox, widget, TRUE,  TRUE,  4);
	psform->uri = (GtkEntry*) widget;

	gtk_box_pack_start (vbox, gtk_hseparator_new (), FALSE, FALSE, 6);

	widget = gtk_check_button_new_with_mnemonic (_("_Launch aria2 on startup"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 2);
	g_signal_connect (widget, "toggled",
			G_CALLBACK (on_plugin_aria2_launch_toggled), psform);
	psform->launch = (GtkToggleButton*) widget;

	widget = gtk_check_button_new_with_mnemonic (_("_Shutdown aria2 on exit"));
	gtk_box_pack_start (vbox, widget, FALSE, FALSE, 2);
	psform->shutdown = (GtkToggleButton*) widget;

	// path
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, TRUE, 2);
	widget = gtk_label_new (_("Path"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_entry_new ();
	gtk_box_pack_start (hbox, widget, TRUE,  TRUE,  4);
	psform->path = (GtkEntry*) widget;
	// argument
	hbox = (GtkBox*) gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (vbox, (GtkWidget*) hbox, FALSE, TRUE, 2);
	widget = gtk_label_new (_("Arguments"));
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 2);
	widget = gtk_entry_new ();
	gtk_box_pack_start (hbox, widget, TRUE,  TRUE,  4);
	psform->args = (GtkEntry*) widget;

	gtk_widget_show (psform->self);
}

void	ug_plugin_setting_form_set (struct UgPluginSettingForm* psform, UgSetting* setting)
{
	gtk_toggle_button_set_active (psform->enable, setting->plugin.aria2.enable);
	gtk_toggle_button_set_active (psform->launch, setting->plugin.aria2.launch);
	gtk_toggle_button_set_active (psform->shutdown, setting->plugin.aria2.shutdown);

	gtk_entry_set_text (psform->uri,  setting->plugin.aria2.uri);
	gtk_entry_set_text (psform->path, setting->plugin.aria2.path);
	gtk_entry_set_text (psform->args, setting->plugin.aria2.args);

	on_plugin_aria2_toggled ((GtkWidget*) psform->enable, psform);
}

void	ug_plugin_setting_form_get (struct UgPluginSettingForm* psform, UgSetting* setting)
{
	setting->plugin.aria2.enable = gtk_toggle_button_get_active (psform->enable);
	setting->plugin.aria2.launch = gtk_toggle_button_get_active (psform->launch);
	setting->plugin.aria2.shutdown = gtk_toggle_button_get_active (psform->shutdown);

	setting->plugin.aria2.uri  = g_strdup (gtk_entry_get_text (psform->uri));
	setting->plugin.aria2.path = g_strdup (gtk_entry_get_text (psform->path));
	setting->plugin.aria2.args = g_strdup (gtk_entry_get_text (psform->args));
}

