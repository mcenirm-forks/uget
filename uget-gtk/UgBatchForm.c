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

#include <UgBatchForm.h>
#include <UgUri.h>

#include <glib/gi18n.h>

// static functions
static void	ug_batch_form_preview_init (struct UgBatchFormPreview* preview);
static void	ug_batch_form_preview_show (struct UgBatchFormPreview* preview, const gchar* message);
// signal handlers
static void	on_radio1_clicked (GtkWidget* widget, UgBatchForm* bform);
static void	on_radio2_clicked (GtkWidget* widget, UgBatchForm* bform);

void	ug_batch_form_init (UgBatchForm* bform)
{
	GtkTable*		table;
	GtkWidget*		label;
	GtkWidget*		entry;
	GtkAdjustment*	spin_adj;

	// top widget
	bform->self = gtk_table_new (6, 6, FALSE);
	table = (GtkTable*) bform->self;
	// URL entry
	entry = gtk_entry_new ();
	label = gtk_label_new_with_mnemonic (_("_URL:"));
	bform->entry = GTK_ENTRY (entry);
	gtk_label_set_mnemonic_widget(GTK_LABEL (label), entry);
	gtk_entry_set_activates_default (bform->entry, TRUE);
	gtk_table_attach (table, label, 0, 1, 0, 1,
			GTK_SHRINK, GTK_SHRINK, 3, 3);
	gtk_table_attach (table, entry, 1, 6, 0, 1,
			GTK_FILL | GTK_EXPAND, GTK_SHRINK, 3, 3);
	g_signal_connect_swapped (GTK_EDITABLE (entry), "changed",
			G_CALLBACK (ug_batch_form_update_preview), bform);
	// e.g.
	label = gtk_label_new (_("e.g."));
	gtk_table_attach (table, label, 0, 1, 1, 2,
			GTK_SHRINK, GTK_SHRINK, 3, 3);
	label = gtk_label_new ("http://for.example/path/pre*.jpg");
	gtk_table_attach (table, label, 1, 6, 1, 2,
			GTK_SHRINK, GTK_SHRINK, 3, 3);
	gtk_table_attach (table, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, 6, 2, 3,
			GTK_FILL | GTK_EXPAND, GTK_SHRINK, 3, 3);

	// -------------------------------------------------------
	// radio "From"
	bform->radio = gtk_radio_button_new_with_mnemonic (NULL, _("_From:"));
	g_signal_connect (bform->radio, "clicked",
			G_CALLBACK (on_radio1_clicked), bform);
	gtk_table_attach (table, bform->radio, 0, 1, 3, 4,
			GTK_SHRINK, GTK_SHRINK, 3, 3);
	// spin "From"
	spin_adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0,
				99999.0, 1.0, 5.0, 0.0);
	bform->spin_from = gtk_spin_button_new (spin_adj, 1.0, 0);
	gtk_table_attach (table, bform->spin_from, 1, 2, 3, 4,
			GTK_FILL | GTK_EXPAND, GTK_SHRINK, 3, 3);
	g_signal_connect_swapped (bform->spin_from, "value-changed",
			G_CALLBACK (ug_batch_form_update_preview), bform);

	// spin "To"
	spin_adj = (GtkAdjustment *) gtk_adjustment_new (10.0, 1.0,
				99999.0, 1.0, 5.0, 0.0);
	bform->spin_to = gtk_spin_button_new (spin_adj, 1.0, 0);
	gtk_table_attach (table, bform->spin_to, 3, 4, 3, 4,
			GTK_FILL | GTK_EXPAND, GTK_SHRINK, 3, 3);
	g_signal_connect_swapped (bform->spin_to, "value-changed",
			G_CALLBACK (ug_batch_form_update_preview), bform);
	// label "To"
	label = gtk_label_new_with_mnemonic (_("To:"));
	gtk_table_attach (table, label, 2, 3, 3, 4, GTK_SHRINK, GTK_SHRINK, 3, 3);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), bform->spin_to);
	bform->label_to = label;

	// spin "digits"
	spin_adj = (GtkAdjustment *) gtk_adjustment_new (2.0, 1.0,
			20.0, 1.0, 5.0, 0.0);
	bform->spin_digits = gtk_spin_button_new (spin_adj, 1.0, 0);
	gtk_table_attach (table, bform->spin_digits, 5, 6, 3, 4,
			GTK_FILL | GTK_EXPAND, GTK_SHRINK, 3, 3);
	g_signal_connect_swapped (bform->spin_digits, "value-changed",
			G_CALLBACK (ug_batch_form_update_preview), bform);
	// label "digits"
	label = gtk_label_new_with_mnemonic (_("digits:"));
	gtk_table_attach (table, label, 4, 5, 3, 4, GTK_SHRINK, GTK_SHRINK, 3, 3);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), bform->spin_digits);
	bform->label_digits = label;

	// -------------------------------------------------------
	// radio "From"
	bform->radio = gtk_radio_button_new_with_mnemonic_from_widget (
			GTK_RADIO_BUTTON (bform->radio), _("F_rom:"));
	gtk_table_attach (table, bform->radio, 0, 1, 4, 5,
			GTK_SHRINK, GTK_SHRINK, 3, 3);
	g_signal_connect (bform->radio, "clicked",
			G_CALLBACK (on_radio2_clicked), bform);
	// entry "From"
	entry = gtk_entry_new ();
	bform->entry_from = GTK_ENTRY (entry);
	gtk_entry_set_text (bform->entry_from, "a");
	gtk_entry_set_max_length (bform->entry_from, 1);
	gtk_entry_set_width_chars (bform->entry_from, 2);
	gtk_widget_set_sensitive (entry, FALSE);
	gtk_table_attach (table, entry, 1, 2, 4, 5,
			GTK_FILL | GTK_EXPAND, GTK_SHRINK, 3, 3);
	g_signal_connect_swapped (GTK_EDITABLE (entry), "changed",
			G_CALLBACK (ug_batch_form_update_preview), bform);

	// entry "To"
	entry = gtk_entry_new ();
	bform->entry_to = GTK_ENTRY (entry);
	gtk_entry_set_text (bform->entry_to, "z");
	gtk_entry_set_max_length (bform->entry_to, 1);
	gtk_entry_set_width_chars (bform->entry_to, 2);
	gtk_widget_set_sensitive (entry, FALSE);
	gtk_table_attach (table, entry, 3, 4, 4, 5,
			GTK_FILL | GTK_EXPAND, GTK_SHRINK, 3, 3);
	g_signal_connect_swapped (GTK_EDITABLE (bform->entry_to), "changed",
			G_CALLBACK(ug_batch_form_update_preview), bform);

	// label case-sensitive
	label = gtk_label_new (_("case-sensitive"));
	gtk_widget_set_sensitive (label, FALSE);
	gtk_table_attach (table, label, 4, 6, 4, 5, GTK_FILL, GTK_SHRINK, 3, 3);
	bform->label_case = label;

	// -------------------------------------------------------
	// preview
	ug_batch_form_preview_init (&bform->preview);
	gtk_table_attach (table, bform->preview.self, 0, 6, 7, 8,
			GTK_FILL, GTK_FILL | GTK_EXPAND, 3, 3);

	ug_batch_form_update_preview (bform);
	gtk_widget_show_all (bform->self);
}

void	ug_batch_form_update_preview (UgBatchForm* bform)
{
	GtkTreeIter		iter;
	GList*			list;
	GList*			link;

	list = ug_batch_form_get_list (bform, TRUE);
	if (list == NULL) {
		if (bform->preview.status == 1) {
			ug_batch_form_preview_show (&bform->preview,
					_("No wildcard(*) character in URL entry."));
		}
		else if (bform->preview.status == 2) {
			ug_batch_form_preview_show (&bform->preview,
					_("URL is not valid."));
		}
		else if (bform->preview.status == 3) {
			ug_batch_form_preview_show (&bform->preview,
					_("No character in 'From' or 'To' entry."));
		}
		// notify
		if (bform->notify.func)
			bform->notify.func (bform->notify.data, FALSE);
		return;
	}

	gtk_list_store_clear (bform->preview.store);
	for (link = list;  link;  link = link->next) {
		gtk_list_store_append (bform->preview.store, &iter);
		gtk_list_store_set (bform->preview.store, &iter, 0, link->data, -1);
	}
	g_list_foreach (list, (GFunc) g_free, NULL);
	g_list_free (list);
	// notify
	if (bform->notify.func)
		bform->notify.func (bform->notify.data, TRUE);
	return;
}

GList*	ug_batch_form_get_list (UgBatchForm* bform, gboolean preview)
{
	GString*		gstr;
	GList*			list;
	gint			from, to, cur, digits;
	gboolean		char_mode;
	const gchar*	string;
	gint			offset;

	string = gtk_entry_get_text (bform->entry);
	offset = strcspn (string, "*");
	if (string [offset] == 0) {
		bform->preview.status = 1;
		return NULL;
	}
	if (ug_uri_scheme_len (string) == 0) {
		bform->preview.status = 2;
		return NULL;
	}
	// char or digit
	char_mode = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bform->radio));
	if (char_mode) {
		// char
		from   = *gtk_entry_get_text (bform->entry_from);
		to     = *gtk_entry_get_text (bform->entry_to);
		digits = 1;
		if (from == 0 || to == 0) {
			bform->preview.status = 3;
			return NULL;
		}
	}
	else {
		// digit
		from   = gtk_spin_button_get_value_as_int ((GtkSpinButton*) bform->spin_from);
		to     = gtk_spin_button_get_value_as_int ((GtkSpinButton*) bform->spin_to);
		digits = gtk_spin_button_get_value_as_int ((GtkSpinButton*) bform->spin_digits);
	}
	// swap from & to
	if (from > to) {
		cur = from;
		from = to;
		to = cur;
	}
	// create URI list
	list = NULL;
	gstr = g_string_sized_new (80);
	g_string_append_len (gstr, string, offset);
	for (cur = to;  cur >= from;  cur--) {
		if (preview  &&  from < to -4  &&  cur == to -2) {
			list = g_list_prepend (list, g_strdup (" ..."));
			cur = from + 1;
		}
		if (char_mode)
			g_string_append_printf (gstr, "%c", cur);
		else
			g_string_append_printf (gstr, "%.*d", digits, cur);
		g_string_append (gstr, string + offset + 1);
		list = g_list_prepend (list, g_strdup (gstr->str));
		g_string_truncate (gstr, offset);
	}
	g_string_free (gstr, TRUE);
	bform->preview.status = 0;
	return list;
}


// ----------------------------------------------------------------------------
//	static functions
//
static void	ug_batch_form_preview_init (struct UgBatchFormPreview* preview)
{
	GtkScrolledWindow*	scrolled;
	GtkCellRenderer*	renderer;
	GtkTreeViewColumn*	column;
	GtkTreeSelection*	selection;

	preview->view  = (GtkTreeView*) gtk_tree_view_new ();
	preview->store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_view_set_model (preview->view, (GtkTreeModel*) preview->store);
//	gtk_tree_view_set_fixed_height_mode (preview->view, TRUE);
	gtk_widget_set_size_request ((GtkWidget*) preview->view, 140, 140);
	selection = gtk_tree_view_get_selection (preview->view);
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_NONE);
	// It will free UgBatchForm.preview_store when UgBatchForm.preview_view destroy.
	g_object_unref (preview->store);

	renderer = gtk_cell_renderer_text_new ();
	column   = gtk_tree_view_column_new_with_attributes (
			_("Preview"), renderer, "text", 0, NULL);
//	gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column (preview->view, column);

	preview->self = gtk_scrolled_window_new (NULL, NULL);
	scrolled = GTK_SCROLLED_WINDOW (preview->self);
	gtk_scrolled_window_set_shadow_type (scrolled, GTK_SHADOW_IN);
	gtk_scrolled_window_set_policy (scrolled,
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled), GTK_WIDGET (preview->view));
}

static void	ug_batch_form_preview_show (struct UgBatchFormPreview* preview, const gchar* message)
{
	GtkTreeIter		iter;

	gtk_list_store_clear (preview->store);
	// skip first row
	gtk_list_store_append (preview->store, &iter);
	// show message in second row
	gtk_list_store_append (preview->store, &iter);
	gtk_list_store_set (preview->store, &iter, 0, message, -1);
}

// ----------------------------------------------------------------------------
//	signal handler
//
static void on_radio1_clicked (GtkWidget* widget, UgBatchForm* bform)
{
	// digit
	gtk_widget_set_sensitive (bform->spin_from, TRUE);
	gtk_widget_set_sensitive (bform->spin_to, TRUE);
	gtk_widget_set_sensitive (bform->spin_digits, TRUE);
	gtk_widget_set_sensitive (bform->label_to, TRUE);
	gtk_widget_set_sensitive (bform->label_digits, TRUE);
	// character
	gtk_widget_set_sensitive (GTK_WIDGET (bform->entry_from), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (bform->entry_to), FALSE);
	gtk_widget_set_sensitive (bform->label_case, FALSE);
	ug_batch_form_update_preview (bform);
}

static void on_radio2_clicked (GtkWidget* widget, UgBatchForm* bform)
{
	// digit
	gtk_widget_set_sensitive (bform->spin_from, FALSE);
	gtk_widget_set_sensitive (bform->spin_to, FALSE);
	gtk_widget_set_sensitive (bform->spin_digits, FALSE);
	gtk_widget_set_sensitive (bform->label_to, FALSE);
	gtk_widget_set_sensitive (bform->label_digits, FALSE);
	// character
	gtk_widget_set_sensitive (GTK_WIDGET (bform->entry_from), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (bform->entry_to), TRUE);
	gtk_widget_set_sensitive (bform->label_case, TRUE);
	ug_batch_form_update_preview (bform);
}

