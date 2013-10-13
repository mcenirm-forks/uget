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

#include <UgUtils.h>
#include <UgBanner.h>

#include <glib/gi18n.h>

static GdkCursor* hand_cursor = NULL;
static GdkCursor* regular_cursor = NULL;

static gboolean
motion_notify_event (GtkWidget* tv_widget, GdkEventMotion* event, UgBanner* banner);

static gboolean
event_after (GtkWidget* text_view, GdkEvent* ev, UgBanner* banner);

static void
on_clicked (GtkWidget* button, UgBanner* banner);

void ug_banner_init (struct UgBanner* banner)
{
	GtkWidget* button;
	GdkRGBA    rgba;

	hand_cursor = gdk_cursor_new (GDK_HAND2);
	regular_cursor = gdk_cursor_new (GDK_XTERM);

	banner->self = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	banner->buffer = gtk_text_buffer_new (NULL);
	banner->tag_link = gtk_text_buffer_create_tag (banner->buffer, NULL,
			"foreground", "blue",
			"underline", PANGO_UNDERLINE_SINGLE,
			NULL);
	banner->text_view = (GtkTextView*) gtk_text_view_new_with_buffer (banner->buffer);
	gtk_box_pack_start (GTK_BOX (banner->self),
			GTK_WIDGET (banner->text_view), TRUE, TRUE, 0);
	rgba.alpha = 1.0;
	rgba.blue  = (double)69  / 255;
	rgba.green = (double)118 / 255;
	rgba.red   = (double)240 / 255;
	gtk_widget_override_background_color (
			GTK_WIDGET (banner->text_view), GTK_STATE_FLAG_NORMAL, &rgba);
	rgba.alpha = 1.0;
	rgba.blue  = 1.0;
	rgba.green = 1.0;
	rgba.red   = 1.0;
	gtk_widget_override_color (
			GTK_WIDGET (banner->text_view), GTK_STATE_FLAG_NORMAL, &rgba);
	// close button
	button = gtk_button_new ();
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
	gtk_button_set_label (GTK_BUTTON (button), "X");
	gtk_box_pack_end (GTK_BOX (banner->self),
			button, FALSE, FALSE, 0);
	g_signal_connect (button, "clicked", G_CALLBACK (on_clicked), banner);

	gtk_text_view_set_cursor_visible (banner->text_view, FALSE);
	gtk_text_view_set_editable (banner->text_view, FALSE);

	g_signal_connect (banner->text_view, "event-after",
			G_CALLBACK (event_after), banner);
	g_signal_connect (banner->text_view, "motion-notify-event",
			G_CALLBACK (motion_notify_event), banner);

	ug_banner_show_donation (banner);
}

void  ug_banner_show_donation (struct UgBanner* banner)
{
	GtkTextIter iter;

	banner->status = UG_BANNER_DONATION;
	gtk_text_buffer_set_text(banner->buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (banner->buffer, &iter, 0);
	gtk_text_buffer_insert (banner->buffer, &iter, "  ", 2);
	gtk_text_buffer_insert (banner->buffer, &iter, _("Attention uGetters:"), -1);
	gtk_text_buffer_insert (banner->buffer, &iter, " ", 1);
	gtk_text_buffer_insert (banner->buffer, &iter,
			"we are running a Donation Drive "
			"for uGet's Future Development, please click ",
			-1);
	gtk_text_buffer_insert_with_tags (banner->buffer, &iter,
			_("HERE"), -1, banner->tag_link, NULL);
}

void  ug_banner_show_survery (struct UgBanner* banner)
{
	GtkTextIter iter;

	banner->status = UG_BANNER_SURVERY;
	gtk_text_buffer_set_text(banner->buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (banner->buffer, &iter, 0);
	gtk_text_buffer_insert (banner->buffer, &iter, "  ", 2);
	gtk_text_buffer_insert (banner->buffer, &iter, _("Attention uGetters:"), -1);
	gtk_text_buffer_insert (banner->buffer, &iter, " ", 1);
	gtk_text_buffer_insert (banner->buffer, &iter,
			_("please fill out this quick User Survery for uGet."), -1);
	gtk_text_buffer_insert (banner->buffer, &iter, " - ", 3);
	gtk_text_buffer_insert_with_tags (banner->buffer, &iter,
			_("click here to take survery"), -1, banner->tag_link, NULL);
}

// ----------------------------------------------------------------------------
// static functions

/* Links can also be activated by clicking.
 */
static gboolean
event_after (GtkWidget* text_view, GdkEvent* ev, UgBanner* banner)
{
	GtkTextIter start, end, iter;
	GtkTextBuffer *buffer;
	GdkEventButton *event;
	gint x, y;
	GSList* slist;

	if (ev->type != GDK_BUTTON_RELEASE)
		return FALSE;

	event = (GdkEventButton *)ev;

	if (event->button != GDK_BUTTON_PRIMARY)
		return FALSE;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

	/* we shouldn't follow a link if the user has selected something */
	gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
	if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
		return FALSE;

	gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view),
                                         GTK_TEXT_WINDOW_WIDGET,
                                         event->x, event->y, &x, &y);

	gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (text_view), &iter, x, y);

	slist = gtk_text_iter_get_tags (&iter);
	if (slist) {
		switch (banner->status) {
		case UG_BANNER_DONATION:
			ug_launch_uri ("https://sourceforge.net/p/urlget/donate/?source=navbar");
			break;

		case UG_BANNER_SURVERY:
			ug_launch_uri ("http://ugetdm.com/forum/");
			break;
		}
	}
	if (slist)
		g_slist_free (slist);

	return FALSE;
}

/* Update the cursor image if the pointer moved.
 */
/* Looks at all tags covering the position (x, y) in the text view,
 * and if one of them is a link, change the cursor to the "hands" cursor
 * typically used by web browsers.
 */
static gboolean
motion_notify_event (GtkWidget* tv_widget, GdkEventMotion* event, UgBanner* banner)
{
	GtkTextView* text_view;
	gint x, y;
	gboolean  hovering = FALSE;
	GSList* slist;
	GtkTextIter iter;

	text_view = GTK_TEXT_VIEW (tv_widget);
	gtk_text_view_window_to_buffer_coords (text_view,
			GTK_TEXT_WINDOW_WIDGET, event->x, event->y, &x, &y);

	// set cursor if appropriate
	gtk_text_view_get_iter_at_location (text_view, &iter, x, y);

	slist = gtk_text_iter_get_tags (&iter);
	if (slist)
		hovering = TRUE;
	if (banner->hovering_over_link != hovering) {
		banner->hovering_over_link = hovering;

		if (banner->hovering_over_link) {
			gdk_window_set_cursor (gtk_text_view_get_window (text_view,
					GTK_TEXT_WINDOW_TEXT), hand_cursor);
		}
		else {
			gdk_window_set_cursor (gtk_text_view_get_window (text_view,
					GTK_TEXT_WINDOW_TEXT), regular_cursor);
		}
	}
	if (slist)
		g_slist_free (slist);

	return FALSE;
}

static void
on_clicked (GtkWidget* button, UgBanner* banner)
{
	switch (banner->status) {
	case UG_BANNER_DONATION:
		ug_banner_show_survery (banner);
		break;

	case UG_BANNER_SURVERY:
		gtk_widget_hide (banner->self);
		break;
	}
}
