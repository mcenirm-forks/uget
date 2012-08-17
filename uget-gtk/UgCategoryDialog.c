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

#include <UgCategoryDialog.h>

#include <glib/gi18n.h>

UgCategoryDialog*	ug_category_dialog_new (const gchar* title, GtkWindow* parent)
{
	UgCategoryDialog*	cdialog;
	GtkNotebook*		notebook;
	GtkWidget*			vbox;

	cdialog = g_malloc0 (sizeof (UgCategoryDialog));
	cdialog->self = (GtkDialog*) gtk_dialog_new_with_buttons (title, parent,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK,     GTK_RESPONSE_OK,
			NULL);
	gtk_dialog_set_default_response (cdialog->self, GTK_RESPONSE_OK);
	ug_proxy_form_init (&cdialog->proxy);
	ug_download_form_init (&cdialog->download, &cdialog->proxy, (GtkWindow*) cdialog->self);
	ug_download_form_set_multiple (&cdialog->download, TRUE);
	ug_category_form_init (&cdialog->category);

	notebook = (GtkNotebook*) gtk_notebook_new ();
	gtk_widget_show ((GtkWidget*) notebook);
	gtk_notebook_append_page (notebook, cdialog->category.self,
			gtk_label_new (_("Category settings")));
	gtk_notebook_append_page (notebook, cdialog->download.page1,
			gtk_label_new (_("Default for new download 1")));
	gtk_notebook_append_page (notebook, cdialog->download.page2,
			gtk_label_new (_("Default 2")));

	vbox = gtk_dialog_get_content_area (cdialog->self);
	gtk_box_pack_start ((GtkBox*) vbox, (GtkWidget*) notebook, FALSE, FALSE, 0);
	gtk_widget_show (vbox);

	gtk_container_set_focus_child (GTK_CONTAINER (cdialog->self), cdialog->category.name_entry);

	return cdialog;
}

void	ug_category_dialog_free (UgCategoryDialog* cdialog)
{
	gtk_widget_destroy ((GtkWidget*) cdialog->self);
	g_free (cdialog);
}

void	ug_category_dialog_get (UgCategoryDialog* cdialog, UgCategory* category)
{
	ug_category_form_get (&cdialog->category, category);
	if (category->defaults == NULL)
		category->defaults = ug_dataset_new ();

	ug_download_form_get (&cdialog->download, category->defaults);
	ug_proxy_form_get (&cdialog->proxy, category->defaults);
}

void	ug_category_dialog_set (UgCategoryDialog* cdialog, UgCategory* category)
{
	ug_category_form_set (&cdialog->category, category);
	if (category->defaults) {
		ug_download_form_set (&cdialog->download, category->defaults, FALSE);
		ug_proxy_form_set (&cdialog->proxy, category->defaults, FALSE);
	}
}

