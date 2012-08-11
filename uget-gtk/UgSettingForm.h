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

#ifndef UG_SETTING_FORM_H
#define UG_SETTING_FORM_H

#include <gtk/gtk.h>
#include <UgSetting.h>

#ifdef __cplusplus
extern "C" {
#endif


// ----------------------------------------------------------------------------
// UgClipboardSettingForm
struct UgClipboardSettingForm
{
	GtkWidget*		self;

	GtkEntry*			pattern;
	GtkToggleButton*	monitor;
	GtkToggleButton*	quiet;
	// add download to Nth category
	GtkWidget*			nth_label;
	GtkSpinButton*		nth_spin;
};

void	ug_clipboard_setting_form_init (struct UgClipboardSettingForm* csform);
void	ug_clipboard_setting_form_set (struct UgClipboardSettingForm* csform, UgSetting* setting);
void	ug_clipboard_setting_form_get (struct UgClipboardSettingForm* csform, UgSetting* setting);

// ----------------------------------------------------------------------------
// UgUserInterfaceForm
struct UgUserInterfaceForm
{
	GtkWidget*		self;

	GtkToggleButton*	confirm_close;
	GtkToggleButton*	confirm_delete;
	GtkToggleButton*	show_trayicon;
	GtkToggleButton*	start_in_tray;
	GtkToggleButton*	start_in_offline_mode;
	GtkToggleButton*	start_notification;
	GtkToggleButton*	sound_notification;
	GtkToggleButton*	apply_recently;
};

void	ug_user_interface_form_init (struct UgUserInterfaceForm* uiform);
void	ug_user_interface_form_set (struct UgUserInterfaceForm* uiform, UgSetting* setting);
void	ug_user_interface_form_get (struct UgUserInterfaceForm* uiform, UgSetting* setting);

// ----------------------------------------------------------------------------
// UgLaunchSettingForm
struct UgLaunchSettingForm
{
	GtkWidget*	self;

	GtkToggleButton*	active;
	GtkEntry*			types;
};

void	ug_launch_setting_form_init (struct UgLaunchSettingForm* lsform);
void	ug_launch_setting_form_set (struct UgLaunchSettingForm* lsform, UgSetting* setting);
void	ug_launch_setting_form_get (struct UgLaunchSettingForm* lsform, UgSetting* setting);

// ----------------------------------------------------------------------------
// UgAutoSaveForm
struct UgAutoSaveForm
{
	GtkWidget*	self;

	// auto save and interval
	GtkToggleButton*	active;
	GtkWidget*			interval_label;
	GtkSpinButton*		interval_spin;
	GtkWidget*			minutes_label;	// minutes
};

void	ug_auto_save_form_init (struct UgAutoSaveForm* asform);
void	ug_auto_save_form_set (struct UgAutoSaveForm* asform, UgSetting* setting);
void	ug_auto_save_form_get (struct UgAutoSaveForm* asform, UgSetting* setting);

// ----------------------------------------------------------------------------
// UgCommandlineSettingForm
struct UgCommandlineSettingForm
{
	GtkWidget*		self;

	// --quiet
	GtkToggleButton*	quiet;
	// --category-index
	GtkWidget*			index_label;
	GtkSpinButton*		index_spin;
};

void	ug_commandline_setting_form_init (struct UgCommandlineSettingForm* csform);
void	ug_commandline_setting_form_set (struct UgCommandlineSettingForm* csform, UgSetting* setting);
void	ug_commandline_setting_form_get (struct UgCommandlineSettingForm* csform, UgSetting* setting);

// ----------------------------------------------------------------------------
// UgPluginSettingForm
struct UgPluginSettingForm
{
	GtkWidget*	self;

	// auto save and interval
	GtkToggleButton*	enable;
	GtkToggleButton*	launch;
	GtkToggleButton*	shutdown;
	GtkEntry*			path;
	GtkEntry*			args;
	GtkEntry*			uri;

	// Speed Limits
	GtkSpinButton*		upload;		// KiB/s
	GtkSpinButton*		download;	// KiB/s
};

void	ug_plugin_setting_form_init (struct UgPluginSettingForm* psform);
void	ug_plugin_setting_form_set (struct UgPluginSettingForm* psform, UgSetting* setting);
void	ug_plugin_setting_form_get (struct UgPluginSettingForm* psform, UgSetting* setting);


#ifdef __cplusplus
}
#endif

#endif  // End of UG_SETTING_FORM_H

