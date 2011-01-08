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


#ifndef UG_DOWNLOAD_FORM_H
#define UG_DOWNLOAD_FORM_H

#include <gtk/gtk.h>
#include <UgDataset.h>
#include <UgProxyForm.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct	UgDownloadForm_		UgDownloadForm;
typedef	void	(*UgDownloadFormNotify) (gpointer user_data, gboolean completed);

struct UgDownloadForm_
{
	GtkWindow*	parent;			// parent window of UgDownloadForm.self

	// ----------------------------------------------------
	// Page 1
	//
	GtkWidget*	page1;

	GtkWidget*	url_label;
	GtkWidget*	url_entry;
//	GtkWidget*	url_button;

	GtkWidget*	file_label;
	GtkWidget*	file_entry;
//	GtkWidget*	file_button;

	GtkWidget*	folder_combo;
	GtkWidget*	folder_entry;
//	GtkWidget*	folder_button;

//	GtkWidget*	referrer_label;
	GtkWidget*	referrer_entry;

	// widgets for login fields
//	GtkWidget*	username_label;
	GtkWidget*	username_entry;
//	GtkWidget*	password_label;
	GtkWidget*	password_entry;

	// widgets for Status
	GtkWidget*	radio_runnable;
	GtkWidget*	radio_pause;

//	GtkWidget*	spin_parts;
//	GtkWidget*	spin_redirect;
	GtkWidget*	spin_retry;		// counts
	GtkWidget*	spin_delay;		// seconds

	GtkWidget*	spin_split;		// segments

	// ----------------------------------------------------
	// Page 2
	//
	GtkWidget*	page2;

//	GtkWidget*	cookie_label;
	GtkWidget*	cookie_entry;
//	GtkWidget*	post_label;
	GtkWidget*	post_entry;

	GtkSpinButton*	spin_speed;

	// ----------------------------------------------------
	// User changed entry
	//
	struct UgDownloadFormChanged
	{
		gboolean	enable:1;
		gboolean	url:1;
		gboolean	file:1;
		gboolean	folder:1;
		gboolean	user:1;
		gboolean	password:1;
		gboolean	referrer:1;
		gboolean	cookie:1;
		gboolean	post:1;
		gboolean	retry:1;	// spin_retry
		gboolean	delay:1;	// spin_delay
		gboolean	split:1;	// spin_split
		gboolean	speed:1;	// spin_speed
	} changed;

	// ----------------------------------------------------
	// callback
	//
	struct
	{
		UgDownloadFormNotify	func;
		gpointer				data;
	} notify;
};

void		ug_download_form_init (UgDownloadForm* dform, UgProxyForm* proxy, GtkWindow* parent);
GtkWidget*	ug_download_from_use_notebook (UgDownloadForm* dform, const gchar* label1, const gchar* label2);

void	ug_download_form_get (UgDownloadForm* dform, UgDataset* dataset);
void	ug_download_form_set (UgDownloadForm* dform, UgDataset* dataset, gboolean keep_changed);

void	ug_download_form_set_multiple (UgDownloadForm* dform, gboolean multiple_mode);
void	ug_download_form_set_relation (UgDownloadForm* dform, gboolean relation_mode);
void	ug_download_form_set_folder_list (UgDownloadForm* dform, GList*  folder_list);
void	ug_download_form_get_folder_list (UgDownloadForm* dform, GList** folder_list);
void	ug_download_form_complete_entry (UgDownloadForm* dform);


#ifdef __cplusplus
}
#endif

#endif  // End of UG_DOWNLOAD_FORM_H

