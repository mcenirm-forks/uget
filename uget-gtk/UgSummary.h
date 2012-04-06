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

#ifndef UG_SUMMARY_H
#define UG_SUMMARY_H

#include <gtk/gtk.h>
#include <UgDataset.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef	struct	UgSummary					UgSummary;

struct UgSummary
{
	GtkWidget*		self;		// (GtkScrolledWindow) container for view
	GtkTreeView*	view;
	GtkListStore*	store;

	struct UgSummaryMenu
	{
		GtkMenu*		self;		// (GtkMenu) pop-up menu
		GtkWidget*		copy;		// GtkMenuItem
		GtkWidget*		copy_all;	// GtkMenuItem
	} menu;

	struct UgSummaryVisible
	{
		gboolean	name:1;
		gboolean	folder:1;
		gboolean	category:1;
		gboolean	url:1;
		gboolean	message:1;
	} visible;
};

void	ug_summary_init (UgSummary* summary, GtkAccelGroup* accel_group);
void	ug_summary_show (UgSummary* summary, UgDataset* dataset);

// call g_free() to free returned string.
gchar*	ug_summary_get_text_selected (UgSummary* summary);
gchar*	ug_summary_get_text_all (UgSummary* summary);


#ifdef __cplusplus
}
#endif

#endif  // End of UG_SUMMARY_H

