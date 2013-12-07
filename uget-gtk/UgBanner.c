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

static GtkWidget* create_x_button (UgBanner* banner);

void ug_banner_init (struct UgBanner* banner)
{
	GtkStyleContext* style_context;
	GdkRGBA    rgba;

	hand_cursor = gdk_cursor_new (GDK_HAND2);
	regular_cursor = gdk_cursor_new (GDK_XTERM);

	banner->self = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	banner->buffer = gtk_text_buffer_new (NULL);
	banner->tag_link = gtk_text_buffer_create_tag (banner->buffer, NULL,
			"underline", PANGO_UNDERLINE_SINGLE,
			NULL);

	banner->text_view = (GtkTextView*) gtk_text_view_new_with_buffer (banner->buffer);
	gtk_text_view_set_cursor_visible (banner->text_view, FALSE);
	gtk_text_view_set_editable (banner->text_view, FALSE);
	gtk_box_pack_start (GTK_BOX (banner->self),
			GTK_WIDGET (banner->text_view), TRUE, TRUE, 0);

	g_signal_connect (banner->text_view, "event-after",
			G_CALLBACK (event_after), banner);
	g_signal_connect (banner->text_view, "motion-notify-event",
			G_CALLBACK (motion_notify_event), banner);
	// style: color
	style_context = gtk_widget_get_style_context (GTK_WIDGET (banner->text_view));
	gtk_style_context_get_background_color (style_context,
			GTK_STATE_FLAG_SELECTED, &rgba);
	gtk_widget_override_background_color (
			GTK_WIDGET (banner->text_view), GTK_STATE_FLAG_NORMAL, &rgba);
	gtk_style_context_get_color (style_context,
			GTK_STATE_FLAG_SELECTED, &rgba);
	gtk_widget_override_color (
			GTK_WIDGET (banner->text_view), GTK_STATE_FLAG_NORMAL, &rgba);
	// close button
	gtk_box_pack_end (GTK_BOX (banner->self),
			create_x_button (banner), FALSE, FALSE, 0);

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
			_("we are running a Donation Drive "
			  "for uGet's Future Development, please click "), -1);
	gtk_text_buffer_insert_with_tags (banner->buffer, &iter,
			_("HERE"), -1, banner->tag_link, NULL);
}

void  ug_banner_show_survey (struct UgBanner* banner)
{
	GtkTextIter iter;

	banner->status = UG_BANNER_SURVEY;
	gtk_text_buffer_set_text(banner->buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (banner->buffer, &iter, 0);
	gtk_text_buffer_insert (banner->buffer, &iter, "  ", 2);
	gtk_text_buffer_insert (banner->buffer, &iter, _("Attention uGetters:"), -1);
	gtk_text_buffer_insert (banner->buffer, &iter, " ", 1);
	gtk_text_buffer_insert (banner->buffer, &iter,
			_("please fill out this quick User Survey for uGet."), -1);
	gtk_text_buffer_insert (banner->buffer, &iter, " - ", 3);
	gtk_text_buffer_insert_with_tags (banner->buffer, &iter,
			_("click here to take survey"), -1, banner->tag_link, NULL);
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
//			ug_launch_uri ("https://sourceforge.net/p/urlget/donate/?source=navbar");
			ug_launch_uri ("http://ugetdm.com/donate");
			break;

		case UG_BANNER_SURVEY:
			ug_launch_uri ("http://ugetdm.com/survey");
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

// ------------------------------------

static gboolean
on_x_button_release (GtkWidget* text_view, GdkEvent* ev, UgBanner* banner)
{
	GdkEventButton *event;

	event = (GdkEventButton*) ev;
	if (event->button != GDK_BUTTON_PRIMARY)
		return FALSE;

	switch (banner->status) {
	case UG_BANNER_DONATION:
		ug_banner_show_survey (banner);
		break;

	case UG_BANNER_SURVEY:
		gtk_widget_hide (banner->self);
		break;
	}

	return FALSE;
}

static GtkWidget* create_x_button (UgBanner* banner)
{
	GtkWidget* event_box;
	GtkWidget* label;

	label = gtk_label_new (" X ");
	event_box = gtk_event_box_new ();
	gtk_container_add (GTK_CONTAINER (event_box), label);

	g_signal_connect (event_box, "button-release-event",
			G_CALLBACK (on_x_button_release), banner);
	return event_box;
}

