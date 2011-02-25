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

#include <UgetGtk.h>
#include <UgPlugin-aria2.h>

#include <glib/gi18n.h>

// ------------------------------------------------------
// aria2
//
#ifdef HAVE_PLUGIN_ARIA2

void	uget_gtk_aria2_init (UgetGtk* ugtk)
{
	ug_xmlrpc_init (&ugtk->xmlrpc);
}

gboolean	uget_gtk_aria2_setup (UgetGtk* ugtk)
{
	const UgPluginClass*	pclass;

	ug_xmlrpc_use_client (&ugtk->xmlrpc, ugtk->setting.plugin.aria2.uri, NULL);
	ug_plugin_global_set (UgPluginAria2Class,
			UG_DATA_TYPE_STRING, ugtk->setting.plugin.aria2.uri);

	pclass = ug_plugin_class_find ("aria2", 0);
	if (pclass)
		ug_plugin_class_unregister (pclass);
	if (ugtk->setting.plugin.aria2.enable) {
		ug_plugin_class_register (UgPluginAria2Class);
		if (ugtk->setting.plugin.aria2.launch)
			uget_gtk_aria2_launch (ugtk);
	}

	return TRUE;
}

gboolean	uget_gtk_aria2_launch (UgetGtk* ugtk)
{
	gboolean	result;
	gchar*		string;

	if (ugtk->aria2_launched == TRUE)
		return TRUE;

	string = g_strconcat (ugtk->setting.plugin.aria2.path, " ",
			ugtk->setting.plugin.aria2.args, NULL);
	result = g_spawn_command_line_async (string, NULL);
	g_free (string);

	if (result == TRUE)
		ugtk->aria2_launched = TRUE;
	else
		uget_gtk_show_message (ugtk, GTK_MESSAGE_ERROR, _("failed to launch aria2."));
	return result;
}

void	uget_gtk_aria2_shutdown (UgetGtk* ugtk)
{
	if (ugtk->setting.plugin.aria2.shutdown) {
		ug_xmlrpc_call (&ugtk->xmlrpc, "aria2.shutdown", NULL);
		ugtk->aria2_launched = FALSE;
	}
}

#endif	// HAVE_PLUGIN_ARIA2

