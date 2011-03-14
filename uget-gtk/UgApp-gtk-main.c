/*
 *
 *   Copyright (C) 2005-2011 by plushuang
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

// ----------------------------------------------------------------------------
#ifdef _WIN32
// This is for ATTACH_PARENT_PROCESS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif
#define _CRT_SECURE_NO_WARNINGS		1
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <winsock.h>
// ------------------------------------
// Windows functions
static void	win32_winsock_init (void)
{
	WSADATA WSAData;

	WSAStartup (MAKEWORD (2, 2), &WSAData);
}
static void	win32_winsock_finalize (void)
{
	WSACleanup ();
}

#if defined (_WINDOWS)
static void atexit_callback (void)
{
	FreeConsole ();
}

static void win32_console_init (void)
{
	typedef BOOL (CALLBACK* LPFNATTACHCONSOLE) (DWORD);
	LPFNATTACHCONSOLE	AttachConsole;
	HMODULE				hmod;

	// If stdout hasn't been redirected to a file, alloc a console
	//  (_istty() doesn't work for stuff using the GUI subsystem)
	if (_fileno(stdout) == -1 || _fileno(stdout) == -2) {
		AttachConsole = NULL;
#ifdef UNICODE
		if ((hmod = GetModuleHandle(L"kernel32.dll")))
#else
		if ((hmod = GetModuleHandle("kernel32.dll")))
#endif
		{
			AttachConsole = (LPFNATTACHCONSOLE) GetProcAddress(hmod, "AttachConsole");
		}
		if ( (AttachConsole && AttachConsole (ATTACH_PARENT_PROCESS)) || AllocConsole() )
		{
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			atexit (atexit_callback);
		}
	}
}

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
	int	main (int argc, char** argv);	// below main()

	return  main (__argc, __argv);
}
#endif  // _WINDOWS
#endif	// _WIN32

// ----------------------------------------------------------------------------
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>		// exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <signal.h>
// uglib
#include <UgApp-gtk.h>

// GStreamer
#ifdef HAVE_GSTREAMER
#include <gst/gst.h>
gboolean	gst_inited	= FALSE;
#endif

// libnotify
#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif

// I18N
#ifndef GETTEXT_PACKAGE
#define GETTEXT_PACKAGE		"uget"
#endif

#include <glib/gi18n.h>

// ----------------------------------------------------------------------------
// ug_get_data_dir() defined in UgApp-gtk.h
const gchar*	ug_get_data_dir (void)
{
#ifdef _WIN32
	static	gchar*	data_dir = NULL;

	if (data_dir == NULL) {
		gchar*		path;
		gunichar2*	path_wcs;
		HMODULE		hmod;

		hmod = GetModuleHandle (NULL);
		// UNICODE
		path_wcs = g_malloc (sizeof (wchar_t) * MAX_PATH);
		GetModuleFileNameW (hmod, path_wcs, MAX_PATH);
		path = g_utf16_to_utf8 (path_wcs, -1, NULL, NULL, NULL);
		g_free (path_wcs);
		data_dir = g_path_get_dirname (path);
		g_free (path);
	}
	return data_dir;
#elif defined (DATADIR)
	return DATADIR;
#else
	return "/usr/share";
#endif  // _WIN32
}

// ----------------------------------------------------------------------------
static	UgAppGtk*	app;

// SIGTERM
static void term_signal_handler (int sig)
{
	// This will quit  gtk_main()  to  main()
	ug_app_quit (app);
}

// ----------------------------------------------------------------------------
int main (int argc, char** argv)
{
	gchar*		string;
	gint		ipc_status;

	// I18N
#ifdef LOCALEDIR
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
#else
	string = g_build_filename (ug_get_data_dir (), "locale", NULL);
	bindtextdomain (GETTEXT_PACKAGE, string);
	g_free (string);
#endif
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	string = ug_arg_find_version (argc, argv);
	if (string) {
#if defined (_WIN32) && defined (_WINDOWS)
		win32_console_init ();
#endif  // _WIN32 && _WINDOWS
		g_print ("uGet " PACKAGE_VERSION " for GTK+" "\n");
		return EXIT_SUCCESS;
	}
	string = ug_arg_find_help (argc, argv);
	// initialize for MS-Windows
#if defined (_WIN32)
#if defined (_WINDOWS)
	if (string)
		win32_console_init ();
#endif  // _WINDOWS
	win32_winsock_init ();
#endif  // _WIN32

	// uglib: options
	app = g_slice_alloc0 (sizeof (UgAppGtk));
	ug_option_init (&app->option);
	ug_option_add (&app->option, NULL, gtk_get_option_group (TRUE));
	if (string) {
		g_print ("uGet " PACKAGE_VERSION " for GTK+" "\n");
		ug_option_help (&app->option, argv[0], string);
	}

	// GLib: GThread
	if (g_thread_supported () == FALSE)
		g_thread_init (NULL);
	// GTK+
	gtk_init (&argc, &argv);

	// GStreamer
#ifdef HAVE_GSTREAMER
	// initialize the GLib thread system using g_thread_init()
	// before any other GLib functions are called.
	gst_inited = gst_init_check (&argc, &argv, NULL);
#endif

	// IPC initialize & check exist Uget program
	ipc_status = ug_ipc_init_with_args (&app->ipc, argc, argv);
	if (ipc_status < 1) {
		if (ipc_status == -1)
			g_print ("uget: IPC failed.\n");
//		if (ipc_status == 0)
//			g_print ("uget: Server exists.\n");
		ug_ipc_finalize (&app->ipc);
		goto exit;
	}
	// libnotify
#ifdef HAVE_LIBNOTIFY
	notify_init ("uGet");
#endif

	// register uget interfaces
	uglib_init ();

	// main program
	ug_app_init (app);
	signal (SIGTERM, term_signal_handler);
	gdk_threads_enter ();
	gtk_main ();
	gdk_threads_leave ();

	// libnotify
#ifdef HAVE_LIBNOTIFY
	if (notify_is_initted ())
		notify_uninit ();
#endif
	// shutdown IPC and sleep 2 second to wait thread
	ug_ipc_finalize (&app->ipc);
	g_usleep (2 * 1000000);

exit:
	// finalize for MS-Windows
#ifdef _WIN32
	win32_winsock_finalize ();
#endif

	return EXIT_SUCCESS;
}

