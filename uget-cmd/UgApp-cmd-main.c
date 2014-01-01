/*
 *
 *   Copyright (C) 2005-2014 by C.H. Huang
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

// ----------------------------------------------------------------------------
#ifdef _WIN32
#include <windows.h>
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
#endif	// _WIN32

// ----------------------------------------------------------------------------
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
// PACKAGE_VERSION
#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION		"1.7.0"
#endif
// std
#include <stdlib.h>		// exit(), EXIT_SUCCESS, EXIT_FAILURE
#include <signal.h>
// uglib
#include <UgApp-base.h>
#include <UgApp-cmd.h>

// ----------------------------------------------------------------------------
static	UgAppCmd*	app;

static void term_signal_handler (int sig)
{
	ug_app_cmd_save (app);
	ug_ipc_finalize (&app->ipc);
#if defined (_WIN32)
	win32_winsock_finalize ();
#endif
	exit (0);
}

// ----------------------------------------------------------------------------
int main (int argc, char** argv)
{
	gchar*		string;
	gint		ipc_status;

	string = ug_arg_find_version (argc, argv);
	if (string) {
#if defined (_WIN32) && defined (_WINDOWS)
		win32_console_init ();
#endif  // _WIN32 && _WINDOWS
		g_print ("uGet " PACKAGE_VERSION " for commandline" "\n");
		return EXIT_SUCCESS;
	}
	string = ug_arg_find_help (argc, argv);
	// initialize for MS-Windows
#if defined (_WIN32)
	win32_winsock_init ();
#endif  // _WIN32

	// allocate memory for application
	app = g_slice_alloc0 (sizeof (UgAppCmd));
	// uglib options
	ug_option_init (&app->option);
	if (string) {
		g_print ("uGet " PACKAGE_VERSION " for commandline" "\n");
		ug_option_help (&app->option, argv[0], string);
	}

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
	// register uget interfaces
	uglib_init ();

	// main program
	signal (SIGTERM, term_signal_handler);
	ug_app_cmd_run (app);

	// shutdown IPC and sleep 2 second to wait thread
	ug_ipc_finalize (&app->ipc);
	g_usleep (2 * 1000000);

exit:
	// finalize for MS-Windows
#if defined (_WIN32)
	win32_winsock_finalize ();
#endif

	return EXIT_SUCCESS;
}

