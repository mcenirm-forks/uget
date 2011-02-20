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

#ifndef UG_SETTING_DIALOG_H
#define UG_SETTING_DIALOG_H

#include <gtk/gtk.h>
#include <UgSettingForm.h>
#include <UgScheduleGrid.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct	UgSettingDialog			UgSettingDialog;

// ----------------------------------------------------------------------------
// UgSettingDialog
enum UgSettingDialogPage
{
	UG_SETTING_PAGE_UI,
	UG_SETTING_PAGE_CLIPBOARD,
	UG_SETTING_PAGE_OTHERS,
};

struct UgSettingDialog
{
	GtkDialog*		self;

	GtkNotebook*	notebook;

	struct UgClipboardSettingForm	clipboard;
	struct UgUserInterfaceForm		ui;
	struct UgLaunchSettingForm		launch;
	struct UgAutoSaveForm			auto_save;
	struct UgScheduleGrid			scheduler;

	gpointer		user_data;
};

UgSettingDialog*	ug_setting_dialog_new (const gchar* title, GtkWindow* parent);
void				ug_setting_dialog_free (UgSettingDialog* dialog);

void	ug_setting_dialog_get (UgSettingDialog* dialog, UgetGtkSetting* setting);
void	ug_setting_dialog_set (UgSettingDialog* dialog, UgetGtkSetting* setting);


#ifdef __cplusplus
}
#endif

#endif  // End of UG_SETTING_DIALOG_H

