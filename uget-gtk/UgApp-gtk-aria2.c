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

#include <stdlib.h>

#ifdef	_WIN32
#ifndef _WIN32_WINNT		// This is for GetProcessId()
#define _WIN32_WINNT 0x501	// 0x501 = WinXP
#include <windows.h>
#endif	// _WIN32_WINNT
#endif	// _WIN32

#include <UgXmlrpc.h>
#include <UgApp-gtk.h>
#include <UgPlugin-aria2.h>

#include <glib/gi18n.h>

#ifdef _WIN32
HANDLE	hProc = NULL;

BOOL CALLBACK ugEnumWindowsProc (HWND hwCurHwnd, LPARAM lparam)
{
	DWORD	dwCurPid = 0;
	struct {
		DWORD	pid;
		HWND	hWnd;
		char*	buf;
		int		bufLen;
	} *param;

	param = GUINT_TO_POINTER (lparam);
	// compare process ID
	GetWindowThreadProcessId (hwCurHwnd, &dwCurPid);
	if(dwCurPid != param->pid)
		return TRUE;
	// compare class name
	GetClassName (hwCurHwnd, param->buf, param->bufLen);
	if(strcmp (param->buf, "ConsoleWindowClass") == 0) {
		param->hWnd = hwCurHwnd;
		return FALSE;
	}

	return TRUE;
}

BOOL ugHideWindowByHandle (HANDLE hProcess)
{
	struct {
		DWORD	pid;
		HWND	hWnd;
		char*	buf;
		int		bufLen;
	} param;

	param.pid = GetProcessId (hProcess);
	param.hWnd = NULL;
	param.bufLen = 256;
	param.buf = g_malloc (param.bufLen);
	EnumWindows (ugEnumWindowsProc, GPOINTER_TO_UINT (&param));
	g_free (param.buf);
	if (param.hWnd) {
		ShowWindow (param.hWnd, SW_HIDE);
		return TRUE;
	}
	return FALSE;
}
#endif	// _WIN32

// ------------------------------------------------------
// aria2
//
#ifdef HAVE_PLUGIN_ARIA2

static gpointer	aria2_ctrl_thread (UgAppGtk* app)
{
	UgXmlrpc*			xmlrpc;
	UgXmlrpcValue*		values;
	UgXmlrpcValue*		member;
	UgXmlrpcResponse	response;
	gchar*				string_down;
	gchar*				string_up;

	xmlrpc = g_malloc0 (sizeof (UgXmlrpc));
	ug_xmlrpc_init (xmlrpc);

	for (;;) {
		// sleep one second
		g_usleep (1 * 1000 * 1000);
#ifdef _WIN32
		if (hProc && ugHideWindowByHandle (hProc) == TRUE) {
			CloseHandle (hProc);
			hProc = NULL;
		}
#endif
		if (app->setting.plugin.aria2.enable == FALSE || app->setting.offline_mode)
			continue;

		// update URI
		g_mutex_lock (&app->aria2.mutex);
		if (app->aria2.uri) {
			ug_xmlrpc_use_client (xmlrpc, app->aria2.uri, NULL);
			g_free (app->aria2.uri);
			app->aria2.uri = NULL;
		}
		g_mutex_unlock (&app->aria2.mutex);

		if (app->aria2.remote_updated == FALSE) {
			string_down = g_strdup_printf ("%uK",
					app->setting.speed_limit.normal.download);
			string_up   = g_strdup_printf ("%uK",
					app->setting.speed_limit.normal.upload);
			values = ug_xmlrpc_value_new_struct (2);
			// max-overall-download-limit
			member = ug_xmlrpc_value_alloc (values);
			member->name = "max-overall-download-limit";
			member->type = UG_XMLRPC_STRING;
			member->c.string = string_down;
			// max-overall-upload-limit
			member = ug_xmlrpc_value_alloc (values);
			member->name = "max-overall-upload-limit";
			member->type = UG_XMLRPC_STRING;
			member->c.string = string_up;
			// call aria2.changeGlobalOption
			response = ug_xmlrpc_call (xmlrpc,
					"aria2.changeGlobalOption",
					UG_XMLRPC_STRUCT, values,
					UG_XMLRPC_NONE);
			// free unused data
			g_free (string_down);
			g_free (string_up);
			ug_xmlrpc_value_free (values);
			// check response
			if (response == UG_XMLRPC_OK) {
				app->aria2.remote_updated = TRUE;
				app->aria2.failed_count = 0;
			}
			else {
				app->aria2.failed_count++;
			}
		}

/*
		// get overall speed
		// call aria2.getGlobalStat
		response = ug_xmlrpc_call (xmlrpc, "aria2.getGlobalStat", UG_XMLRPC_NONE);
		if (response != UG_XMLRPC_OK)
			app->aria2.failed_count++;
		else {
			app->aria2.failed_count = 0;
			// get response
			values = ug_xmlrpc_get_value (xmlrpc);
			// download speed
			member = ug_xmlrpc_value_find (values, "downloadSpeed");
#if  defined (_MSC_VER)  ||  defined (__MINGW32__)
			app->aria2.download_speed = _atoi64 (member->c.string);
#else		// C99 Standard
			app->aria2.download_speed = atoll (member->c.string);
#endif
			// upload speed
			member = ug_xmlrpc_value_find (values, "uploadSpeed");
#if  defined (_MSC_VER)  ||  defined (__MINGW32__)
			app->aria2.upload_speed = _atoi64 (member->c.string);
#else		// C99 Standard
			app->aria2.upload_speed = atoll (member->c.string);
#endif
		}
*/
	}

	ug_xmlrpc_finalize (xmlrpc);
	g_free (xmlrpc);
	return NULL;
}

void	ug_app_aria2_init (UgAppGtk* app)
{
	g_mutex_init (&app->aria2.mutex);
}

void	ug_app_aria2_finalize (UgAppGtk* app)
{
//	g_mutex_clear (&app->aria2.mutex);
}

gboolean	ug_app_aria2_setup (UgAppGtk* app)
{
	const UgPluginInterface*	iface;

	// update URI
	g_mutex_lock (&app->aria2.mutex);
	if (app->aria2.uri)
		g_free (app->aria2.uri);
	app->aria2.uri = g_strdup (app->setting.plugin.aria2.uri);
	g_mutex_unlock (&app->aria2.mutex);

	ug_plugin_global_set (&ug_plugin_aria2_iface,
			UG_TYPE_STRING, app->setting.plugin.aria2.uri);

	iface = ug_plugin_interface_find ("aria2", 0);
	if (iface)
		ug_plugin_interface_unregister (iface);
	if (app->setting.plugin.aria2.enable) {
		ug_plugin_interface_register (&ug_plugin_aria2_iface);
		if (app->setting.plugin.aria2.launch)
			ug_app_aria2_launch (app);
		if (app->aria2.thread == NULL) {
			app->aria2.thread = g_thread_new ("aria2-ctrl",
					(GThreadFunc) aria2_ctrl_thread, app);
		}
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

	// If path is not absolute path, program will search PATH.
	if (strrchr (app->setting.plugin.aria2.path, G_DIR_SEPARATOR) == NULL)
		flags = G_SPAWN_SEARCH_PATH;
	else
		flags = 0;
#ifdef _WIN32
	// child_pid will be filled with a handle to the child process only if
	// you specified the G_SPAWN_DO_NOT_REAP_CHILD flag.
	// You should close the handle with CloseHandle() or g_spawn_close_pid()
	// when you no longer need it.
	flags |= G_SPAWN_DO_NOT_REAP_CHILD;
#endif
	// spawn command
	retval = g_spawn_async (NULL,			// working_directory
		                    (gchar**) args->pdata,	// argv
	                        NULL,			// envp
	                        flags,			// GSpawnFlags
	                        NULL,			// child_setup
	                        NULL,			// user_data
#ifdef _WIN32
	                        &hProc,			// child_pid (Windows HANDLE)
#else
	                        NULL,			// child_pid
#endif
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
	UgXmlrpc*			xmlrpc;

	xmlrpc = g_malloc0 (sizeof (UgXmlrpc));
	ug_xmlrpc_init (xmlrpc);
	ug_xmlrpc_use_client (xmlrpc, app->setting.plugin.aria2.uri, NULL);

	if (app->setting.plugin.aria2.shutdown) {
		ug_xmlrpc_call (xmlrpc, "aria2.shutdown", NULL);
		app->aria2.launched = FALSE;
	}

	ug_xmlrpc_finalize (xmlrpc);
	g_free (xmlrpc);
}

#endif	// HAVE_PLUGIN_ARIA2

