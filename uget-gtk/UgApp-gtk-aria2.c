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

#include <UgApp-gtk.h>
#include <UgPlugin-aria2.h>

#include <glib/gi18n.h>

// ------------------------------------------------------
// aria2
//
#ifdef HAVE_PLUGIN_ARIA2

void	ug_app_aria2_init (UgAppGtk* app)
{
	ug_xmlrpc_init (&app->aria2.xmlrpc);
}

gboolean	ug_app_aria2_setup (UgAppGtk* app)
{
	const UgPluginInterface*	iface;

	ug_xmlrpc_use_client (&app->aria2.xmlrpc, app->setting.plugin.aria2.uri, NULL);
	ug_plugin_global_set (&ug_plugin_aria2_iface,
			UG_DATA_STRING, app->setting.plugin.aria2.uri);

	iface = ug_plugin_interface_find ("aria2", 0);
	if (iface)
		ug_plugin_interface_unregister (iface);
	if (app->setting.plugin.aria2.enable) {
		ug_plugin_interface_register (&ug_plugin_aria2_iface);
		if (app->setting.plugin.aria2.launch)
			ug_app_aria2_launch (app);
	}

	return TRUE;
}

gboolean	ug_app_aria2_launch (UgAppGtk* app)
{
	GPtrArray*	args;
	gchar**		argv;
	gint		argc;
	gint		index;
	gboolean	retval;
	GSpawnFlags	flags;

	if (app->aria2.launched == TRUE)
		return TRUE;
	// arguments
	argc = 0;
	argv = NULL;
	g_shell_parse_argv (app->setting.plugin.aria2.args, &argc, &argv, NULL);
	args = g_ptr_array_sized_new (argc + 2);
	g_ptr_array_add (args, app->setting.plugin.aria2.path);
	for (index = 0;  index < argc;  index++)
		g_ptr_array_add (args, argv[index]);
	g_ptr_array_add (args, NULL);	// NULL-terminated

	// If path is not absolute path, don't search PATH.
	if (strrchr (app->setting.plugin.aria2.path, G_DIR_SEPARATOR) == NULL)
		flags = G_SPAWN_SEARCH_PATH;
	else
		flags = 0;
	// spawn command
	retval = g_spawn_async (NULL,			// working_directory
		                    (gchar**) args->pdata,	// argv
	                        NULL,			// envp
	                        flags,			// GSpawnFlags
	                        NULL,			// child_setup
	                        NULL,			// user_data
	                        NULL,			// child_pid
	                        NULL);			// GError**

	// free arguments
	g_ptr_array_free (args, TRUE);
	g_strfreev (argv);
	// returning value
	if (retval == TRUE)
		app->aria2.launched = TRUE;
	else
		ug_app_show_message (app, GTK_MESSAGE_ERROR, _("failed to launch aria2."));
	return retval;
}

void	ug_app_aria2_shutdown (UgAppGtk* app)
{
	if (app->setting.plugin.aria2.shutdown) {
		ug_xmlrpc_call (&app->aria2.xmlrpc, "aria2.shutdown", NULL);
		app->aria2.launched = FALSE;
	}
}

#endif	// HAVE_PLUGIN_ARIA2

