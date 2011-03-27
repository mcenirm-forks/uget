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

#include <UgCategoryForm.h>
#include <UgString.h>

#include <glib/gi18n.h>

void	ug_category_form_init (UgCategoryForm* cform)
{
	GtkWidget*	label;
	GtkTable*	top_table;
	GtkTable*	table;

	cform->self = gtk_table_new (3, 2, FALSE);
	top_table  = (GtkTable*) cform->self;
	gtk_container_set_border_width (GTK_CONTAINER (top_table), 2);

	cform->name_entry = gtk_entry_new ();
	cform->name_label = gtk_label_new_with_mnemonic (_("Category _name:"));
	gtk_entry_set_activates_default (GTK_ENTRY (cform->name_entry), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (cform->name_label), cform->name_entry);
	gtk_table_attach (top_table, cform->name_label, 0, 1, 0, 1,
			GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach (top_table, cform->name_entry, 1, 2, 0, 1,
			GTK_FILL | GTK_EXPAND, GTK_SHRINK, 2, 2);

	// option table
	table = (GtkTable*) gtk_table_new (3, 2, FALSE);
	gtk_table_attach (top_table, GTK_WIDGET (table), 0, 2, 1, 2,
			GTK_FILL, GTK_SHRINK, 0, 2);

	cform->spin_active = gtk_spin_button_new_with_range (1.0, 20.0, 1.0);
	gtk_entry_set_width_chars (GTK_ENTRY(cform->spin_active), 4);
	label = gtk_label_new_with_mnemonic (_("Active _downloads:"));
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), cform->spin_active);
	gtk_table_attach (table, label, 0, 1, 0, 1,
			GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach (table, cform->spin_active, 1, 2, 0, 1,
			GTK_SHRINK, GTK_SHRINK, 0, 2);

	cform->spin_finished = gtk_spin_button_new_with_range (0.0, 9999.0, 1.0);
	label = gtk_label_new_with_mnemonic (_("Capacity of Finished:"));
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), cform->spin_finished);
	gtk_entry_set_width_chars (GTK_ENTRY(cform->spin_finished), 4);
	gtk_table_attach (table, label, 0, 1, 1, 2,
			GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach (table, cform->spin_finished, 1, 2, 1, 2,
			GTK_SHRINK, GTK_SHRINK, 0, 2);

	cform->spin_recycled = gtk_spin_button_new_with_range (0.0, 9999.0, 1.0);
	label = gtk_label_new_with_mnemonic (_("Capacity of Recycled:"));
	gtk_label_set_mnemonic_widget (GTK_LABEL(label), cform->spin_recycled);
	gtk_entry_set_width_chars (GTK_ENTRY(cform->spin_recycled), 4);
	gtk_table_attach (table, label, 0, 1, 2, 3,
			GTK_SHRINK, GTK_SHRINK, 2, 2);
	gtk_table_attach (table, cform->spin_recycled, 1, 2, 2, 3,
			GTK_SHRINK, GTK_SHRINK, 0, 2);

	gtk_widget_show_all (GTK_WIDGET (top_table));
}

void	ug_category_form_get  (UgCategoryForm*  cform, UgCategory* category)
{
//	category = category->queuing;		// ug_category_dialog_get() has do this

	if (gtk_widget_is_sensitive (cform->name_entry) == TRUE)
		ug_str_set (&category->name, gtk_entry_get_text ((GtkEntry*) cform->name_entry), -1);
	category->active_limit = gtk_spin_button_get_value_as_int ((GtkSpinButton*) cform->spin_active);
	// Finished
	category->finished_limit = gtk_spin_button_get_value_as_int ((GtkSpinButton*) cform->spin_finished);
	// Recycled
	category->recycled_limit = gtk_spin_button_get_value_as_int ((GtkSpinButton*) cform->spin_recycled);
}

void	ug_category_form_set  (UgCategoryForm*  cform, UgCategory* category)
{
//	category = category->queuing;		// ug_category_dialog_get() has do this

	if (gtk_widget_is_sensitive (cform->name_entry) == TRUE)
		gtk_entry_set_text ((GtkEntry*) cform->name_entry, (category->name) ? category->name : "");
	gtk_spin_button_set_value ((GtkSpinButton*) cform->spin_active, (gdouble) category->active_limit);
	// Finished
	gtk_spin_button_set_value ((GtkSpinButton*) cform->spin_finished, (gdouble) category->finished_limit);
	// Recycled
	gtk_spin_button_set_value ((GtkSpinButton*) cform->spin_recycled, (gdouble) category->recycled_limit);
}

void	ug_category_form_set_multiple (UgCategoryForm*  cform, gboolean multiple_mode)
{
	if (multiple_mode) {
		gtk_widget_hide (cform->name_entry);
		gtk_widget_hide (cform->name_label);
		gtk_widget_set_sensitive (cform->name_entry, FALSE);
		gtk_widget_set_sensitive (cform->name_label, FALSE);
	}
	else {
		gtk_widget_show (cform->name_entry);
		gtk_widget_show (cform->name_label);
		gtk_widget_set_sensitive (cform->name_entry, TRUE);
		gtk_widget_set_sensitive (cform->name_label, TRUE);
	}
}

