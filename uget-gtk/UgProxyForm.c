/*
 *
 *   Copyright (C) 2005-2013 by C.H. Huang
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

#include <UgProxyForm.h>
#include <UgData-download.h>
#include <UgString.h>

#include <glib/gi18n.h>

static void	ug_proxy_form_std_init (UgProxyForm* pform);
//	signal handler
static void on_type_changed (GtkComboBox* widget, UgProxyForm* pform);
static void on_entry_std_changed  (GtkEditable* editable, UgProxyForm* pform);

#ifdef HAVE_LIBPWMD
static void	ug_proxy_form_pwmd_init (struct UgProxyFormPwmd* pfp, UgProxyForm* pform);
static void on_entry_pwmd_changed (GtkEditable* editable, UgProxyForm* pform);
#endif


void	ug_proxy_form_init (UgProxyForm* pform)
{
	GtkWidget*	vbox;
	GtkWidget*	hbox;
	GtkWidget*	widget;

	pform->changed.enable = TRUE;
	pform->changed.type   = FALSE;

	// proxy type label & combo box
	widget = gtk_label_new (_("Proxy:"));
	pform->type = gtk_combo_box_text_new ();
	gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (pform->type),
			UG_DATA_PROXY_NONE, _("Don't use"));
	gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (pform->type),
			UG_DATA_PROXY_DEFAULT, _("Default"));
	gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (pform->type),
			UG_DATA_PROXY_HTTP, "HTTP");
	gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (pform->type),
			UG_DATA_PROXY_SOCKS4, "SOCKS v4");
	gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (pform->type),
			UG_DATA_PROXY_SOCKS5, "SOCKS v5");
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 1);
	gtk_box_pack_start (GTK_BOX (hbox), pform->type, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox),
			gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), TRUE, TRUE, 2);
	g_signal_connect (pform->type, "changed",
			G_CALLBACK (on_type_changed), pform);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	ug_proxy_form_std_init (pform);
	gtk_box_pack_end (GTK_BOX (vbox), pform->std, TRUE, TRUE, 0);

#ifdef HAVE_LIBPWMD
	gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (pform->type),
			UG_DATA_PROXY_PWMD, "PWMD");

	ug_proxy_form_pwmd_init (&pform->pwmd, pform);
	gtk_box_pack_end (GTK_BOX (vbox), pform->pwmd.self, TRUE, TRUE, 0);
#endif	// HAVE_LIBPWMD

	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);
	pform->self = (GtkWidget*) vbox;

	gtk_widget_show_all (pform->self);
}

static void	ug_proxy_form_std_init (UgProxyForm* pform)
{
	GtkGrid*	grid;
	GtkWidget*	widget;
	GtkWidget*	hbox;

	pform->changed.host = FALSE;
	pform->changed.port = FALSE;
	pform->changed.user = FALSE;
	pform->changed.password = FALSE;

	pform->std = gtk_grid_new ();
	grid       = (GtkGrid*) pform->std;
	// host label & entry
	widget = gtk_label_new_with_mnemonic (_("Host:"));
	pform->host = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (pform->host), 8);
	gtk_entry_set_activates_default (GTK_ENTRY (pform->host), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (widget), pform->host);
	g_object_set (widget, "margin-left", 3, "margin-right", 3, NULL);
	g_object_set (widget, "margin-top", 1, "margin-bottom", 1, NULL);
	g_object_set (pform->host, "margin", 1, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, widget, 0, 0, 1, 1);
	gtk_grid_attach (grid, pform->host, 1, 0, 1, 1);
	// port label & entry
	widget = gtk_label_new_with_mnemonic (_("Port:"));
	pform->port  = gtk_spin_button_new_with_range (0.0, 65535.0, 1.0);
	gtk_entry_set_width_chars (GTK_ENTRY (pform->port), 5);
	gtk_label_set_mnemonic_widget (GTK_LABEL (widget), pform->port);
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (hbox), pform->port, FALSE, FALSE, 0);
	g_object_set (widget, "margin-left", 3, "margin-right", 3, NULL);
	g_object_set (widget, "margin-top", 1, "margin-bottom", 1, NULL);
	g_object_set (hbox, "margin", 1, NULL);
	gtk_grid_attach (grid, widget, 0, 1, 1, 1);
	gtk_grid_attach (grid, hbox, 1, 1, 1, 1);
	// center separator
	widget = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	g_object_set (widget, "margin", 1, NULL);
	gtk_grid_attach (grid, widget, 2, 0, 1, 2);
	// user label & entry
	widget = gtk_label_new_with_mnemonic (_("User:"));
	pform->user = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (pform->user), 7);
	gtk_entry_set_activates_default (GTK_ENTRY (pform->user), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (widget), pform->user);
	g_object_set (widget, "margin-left", 3, "margin-right", 3, NULL);
	g_object_set (widget, "margin-top", 1, "margin-bottom", 1, NULL);
	g_object_set (pform->user, "margin", 1, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, widget, 3, 0, 1, 1);
	gtk_grid_attach (grid, pform->user, 4, 0, 1, 1);
	// password label & entry
	widget = gtk_label_new_with_mnemonic (_("Password:"));
	pform->password = gtk_entry_new ();
	gtk_entry_set_visibility (GTK_ENTRY (pform->password), FALSE);
	gtk_entry_set_width_chars (GTK_ENTRY (pform->password), 7);
	gtk_entry_set_activates_default (GTK_ENTRY (pform->password), TRUE);
	gtk_label_set_mnemonic_widget (GTK_LABEL (widget), pform->password);
	g_object_set (widget, "margin-left", 3, "margin-right", 3, NULL);
	g_object_set (widget, "margin-top", 1, "margin-bottom", 1, NULL);
	g_object_set (pform->password, "margin", 1, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, widget, 3, 1, 1, 1);
	gtk_grid_attach (grid, pform->password, 4, 1, 1, 1);

	g_signal_connect (GTK_EDITABLE (pform->user), "changed",
			G_CALLBACK (on_entry_std_changed), pform);
	g_signal_connect (GTK_EDITABLE (pform->password), "changed",
			G_CALLBACK (on_entry_std_changed), pform);
	g_signal_connect (GTK_EDITABLE (pform->host), "changed",
			G_CALLBACK (on_entry_std_changed), pform);
	g_signal_connect (GTK_EDITABLE (pform->port), "changed",
			G_CALLBACK (on_entry_std_changed), pform);

	gtk_widget_show_all (pform->std);
}

void	ug_proxy_form_get  (UgProxyForm* pform, UgDataset* dataset)
{
	UgDataProxy*	proxy;
	gint			index;

	index = gtk_combo_box_get_active ((GtkComboBox*) pform->type);
	proxy = ug_dataset_realloc (dataset, UG_DATA_PROXY_I, 0);
	proxy->type = index;

	ug_str_set (&proxy->user,     gtk_entry_get_text ((GtkEntry*)pform->user), -1);
	ug_str_set (&proxy->password, gtk_entry_get_text ((GtkEntry*)pform->password), -1);
	ug_str_set (&proxy->host,     gtk_entry_get_text ((GtkEntry*)pform->host), -1);
	proxy->port = gtk_spin_button_get_value_as_int ((GtkSpinButton*) pform->port);

#ifdef HAVE_LIBPWMD
	ug_str_set (&proxy->pwmd.socket,  gtk_entry_get_text ((GtkEntry*)pform->pwmd.socket),  -1);
	ug_str_set (&proxy->pwmd.file,    gtk_entry_get_text ((GtkEntry*)pform->pwmd.file),    -1);
	ug_str_set (&proxy->pwmd.element, gtk_entry_get_text ((GtkEntry*)pform->pwmd.element), -1);
#endif	// HAVE_LIBPWMD
}

void	ug_proxy_form_set  (UgProxyForm* pform, UgDataset* dataset, gboolean keep_changed)
{
	UgDataProxy*	proxy;

	proxy = ug_dataset_get (dataset, UG_DATA_PROXY_I, 0);
	// if no proxy data
	if (proxy == NULL) {
		pform->changed.enable = FALSE;	// disable changed flags
		if (keep_changed == FALSE  ||  pform->changed.type == FALSE)
			gtk_spin_button_set_value ((GtkSpinButton*) pform->port, 80);
		if (keep_changed == FALSE  ||  pform->changed.port == FALSE)
			gtk_combo_box_set_active ((GtkComboBox*) pform->type, UG_DATA_PROXY_NONE);
		pform->changed.enable = TRUE;	// enable changed flags
		return;
	}

	// disable changed flags
	pform->changed.enable = FALSE;
	// set changed flags
	if (keep_changed == FALSE) {
		pform->changed.type     = proxy->keeping.type;
		pform->changed.host     = proxy->keeping.host;
		pform->changed.port     = proxy->keeping.port;
		pform->changed.user     = proxy->keeping.user;
		pform->changed.password = proxy->keeping.password;
	}
	// Type
	if (keep_changed == FALSE  ||  pform->changed.type == FALSE)
		gtk_combo_box_set_active ((GtkComboBox*) pform->type, proxy->type);
	// User
	if (keep_changed == FALSE  ||  pform->changed.user == FALSE) {
		gtk_entry_set_text ((GtkEntry*) pform->user,
				(proxy->user) ? proxy->user : "");
	}
	// Password
	if (keep_changed == FALSE  ||  pform->changed.password == FALSE) {
		gtk_entry_set_text ((GtkEntry*) pform->password,
				(proxy->password) ? proxy->password : "");
	}
	// Host
	if (keep_changed == FALSE  ||  pform->changed.host == FALSE) {
		gtk_entry_set_text ((GtkEntry*) pform->host,
				(proxy->host) ? proxy->host : "");
	}
	// Port
	if (keep_changed == FALSE  ||  pform->changed.port == FALSE)
		gtk_spin_button_set_value ((GtkSpinButton*) pform->port, proxy->port);

#ifdef HAVE_LIBPWMD
	if (keep_changed == FALSE) {
		pform->pwmd.changed.socket  = proxy->pwmd.keeping.socket;
		pform->pwmd.changed.file    = proxy->pwmd.keeping.file;
		pform->pwmd.changed.element = proxy->pwmd.keeping.element;
	}

	if (keep_changed == FALSE  ||  pform->pwmd.changed.socket == FALSE) {
		gtk_entry_set_text ((GtkEntry*) pform->pwmd.socket,
				(proxy->pwmd.socket) ? proxy->pwmd.socket: "");
	}
	if (keep_changed == FALSE  ||  pform->pwmd.changed.file == FALSE) {
		gtk_entry_set_text ((GtkEntry*) pform->pwmd.file,
			    (proxy->pwmd.file) ? proxy->pwmd.file: "");
	}
	if (keep_changed == FALSE  ||  pform->pwmd.changed.element == FALSE) {
	    gtk_entry_set_text ((GtkEntry*) pform->pwmd.element,
				(proxy->pwmd.element) ? proxy->pwmd.element: "");
	}
#endif	// HAVE_LIBPWMD

	// enable changed flags
	pform->changed.enable = TRUE;
}

//-------------------------------------------------------------------
// signal
static void on_type_changed (GtkComboBox* widget, UgProxyForm* pform)
{
	gint		index;
	gboolean	sensitive;

	if (pform->changed.enable)
		pform->changed.type = TRUE;
	index = gtk_combo_box_get_active (widget);
	if (index == UG_DATA_PROXY_NONE)
		sensitive = FALSE;
	else
		sensitive = TRUE;

	gtk_widget_set_sensitive (pform->std, sensitive);

#ifdef HAVE_LIBPWMD
	gtk_widget_set_sensitive (pform->pwmd.self, sensitive);

	if (index == UG_DATA_PROXY_PWMD) {
		gtk_widget_set_visible (pform->std, FALSE);
		gtk_widget_set_visible (pform->pwmd.self, TRUE);
	}
	else {
		gtk_widget_set_visible (pform->pwmd.self, FALSE);
		gtk_widget_set_visible (pform->std, TRUE);
	}
#endif	// HAVE_LIBPWMD
}

static void on_entry_std_changed (GtkEditable* editable, UgProxyForm* pform)
{
	if (pform->changed.enable) {
		if (editable == GTK_EDITABLE (pform->host))
			pform->changed.host = TRUE;
		else if (editable == GTK_EDITABLE (pform->port))
			pform->changed.port = TRUE;
		else if (editable == GTK_EDITABLE (pform->user))
			pform->changed.user = TRUE;
		else if (editable == GTK_EDITABLE (pform->password))
			pform->changed.password = TRUE;
	}
}


// ----------------------------------------------------------------------------
// PWMD
//
#ifdef HAVE_LIBPWMD
static void	ug_proxy_form_pwmd_init (struct UgProxyFormPwmd* pfp, UgProxyForm* pform)
{
	GtkGrid*	grid;
	GtkWidget*	widget;

	pfp->self = gtk_grid_new ();
	grid = (GtkGrid*) pfp->self;
	widget = gtk_label_new_with_mnemonic (_("Socket:"));
	pfp->socket = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (pfp->socket), 16);
	gtk_label_set_mnemonic_widget (GTK_LABEL (widget), pfp->socket);
	g_object_set (widget, "margin-left", 3, "margin-right", 3, NULL);
	g_object_set (widget, "margin-top", 1, "margin-bottom", 1, NULL);
	g_object_set (pfp->socket, "margin", 1, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, widget, 0, 0, 1, 1);
	gtk_grid_attach (grid, pfp->socket, 1, 0, 1, 1);

	widget = gtk_label_new_with_mnemonic (_("File:"));
	pfp->file = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (pfp->file), 16);
	gtk_label_set_mnemonic_widget (GTK_LABEL (widget), pfp->file);
	g_object_set (widget, "margin-left", 3, "margin-right", 3, NULL);
	g_object_set (widget, "margin-top", 1, "margin-bottom", 1, NULL);
	g_object_set (pfp->file, "margin", 1, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, widget, 0, 1, 1, 1);
	gtk_grid_attach (grid, pfp->file, 1, 1, 1, 1);

	widget = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	g_object_set (widget, "margin", 1, NULL);
	gtk_grid_attach (grid, widget, 2, 0, 1, 2);

	widget = gtk_label_new_with_mnemonic (_("Element:"));
	pfp->element = gtk_entry_new ();
	gtk_entry_set_width_chars (GTK_ENTRY (pfp->element), 16);
	gtk_label_set_mnemonic_widget (GTK_LABEL (widget), pfp->element);
	g_object_set (widget, "margin-left", 3, "margin-right", 3, NULL);
	g_object_set (widget, "margin-top", 1, "margin-bottom", 1, NULL);
	g_object_set (pfp->element, "margin", 1, "hexpand", TRUE, NULL);
	gtk_grid_attach (grid, widget, 3, 0, 1, 1);
	gtk_grid_attach (grid, pfp->element, 4, 0, 1, 1);

	g_signal_connect (GTK_EDITABLE (pform->pwmd.socket), "changed",
			G_CALLBACK (on_entry_pwmd_changed), pform);
	g_signal_connect (GTK_EDITABLE (pform->pwmd.file), "changed",
			G_CALLBACK (on_entry_pwmd_changed), pform);
	g_signal_connect (GTK_EDITABLE (pform->pwmd.element), "changed",
			G_CALLBACK (on_entry_pwmd_changed), pform);

	gtk_widget_show_all ((GtkWidget*) grid);
	gtk_widget_hide ((GtkWidget*) grid);
}

static void on_entry_pwmd_changed (GtkEditable* editable, UgProxyForm* pform)
{
	if (pform->changed.enable) {
		if (editable == GTK_EDITABLE (pform->pwmd.socket))
			pform->pwmd.changed.socket = TRUE;
		else if (editable == GTK_EDITABLE (pform->pwmd.file))
			pform->pwmd.changed.file = TRUE;
		else if (editable == GTK_EDITABLE (pform->pwmd.element))
			pform->pwmd.changed.element = TRUE;
	}
}
#endif	// HAVE_LIBPWMD

