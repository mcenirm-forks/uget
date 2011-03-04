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


#ifndef UG_DOWNLOAD_DIALOG_H
#define UG_DOWNLOAD_DIALOG_H

#include <gtk/gtk.h>
#include <UgCategoryWidget.h>
#include <UgDownloadForm.h>
#include <UgBatchForm.h>
#include <UgSelector.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct	UgDownloadDialog			UgDownloadDialog;

struct UgDownloadDialog
{
	GtkDialog*		self;

	GtkBox*			hbox;
	GtkTreeView*	category_view;
	GtkWidget*		button_back;
	GtkWidget*		button_forward;

	UgSelector			selector;
	UgBatchForm			batch;
	UgProxyForm			proxy;
	UgDownloadForm		download;

	struct UgDownloadDialogPage
	{
		gboolean		completed[2];
		GtkWidget*		array[2];
		GtkWidget*		current;
	} page;

	// External data
	UgDataset*		dataset;

	// Additional data
	struct
	{
		gpointer	app;
		gpointer	data;
	} user;
};

UgDownloadDialog*	ug_download_dialog_new (const gchar* title, GtkWindow* parent);
void				ug_download_dialog_free (UgDownloadDialog* ddialog);

void	ug_download_dialog_use_batch    (UgDownloadDialog* ddialog);
void	ug_download_dialog_use_selector (UgDownloadDialog* ddialog);

void	ug_download_dialog_set_category (UgDownloadDialog* ddialog, UgCategoryWidget* cwidget);
void	ug_download_dialog_set_current_page (UgDownloadDialog* ddialog, gint nth_page);

void	ug_download_dialog_get (UgDownloadDialog* ddialog, UgDataset* dataset);	// replace
void	ug_download_dialog_set (UgDownloadDialog* ddialog, UgDataset* dataset);	// change widget value

// return current UgCategoryGtk in cursor position.
UgCategory*	ug_download_dialog_get_category (UgDownloadDialog* ddialog);

// return newly-created list of UgDataset.
// To free the returned value, use:
//	g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
//	g_list_free (list);
GList*		ug_download_dialog_get_downloads (UgDownloadDialog* ddialog);


#ifdef __cplusplus
}
#endif

#endif  // End of UG_DOWNLOAD_DIALOG_H

