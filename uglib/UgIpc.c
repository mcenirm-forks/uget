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

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <UgIpc.h>
#include <UgUtils.h>
#include <UgOption.h>

// server callback functions
static gboolean server_accept  (GIOChannel *source, GIOCondition condition, UgIpc* ipc);
static gboolean server_connect (GIOChannel *source, GIOCondition condition, UgIpc* ipc);

#ifdef _WIN32
// Windows code -----------------------
#include <winsock.h>
//#include <ws2tcpip.h>	// ADDRINFO

gboolean	ug_ipc_use_server (UgIpc* ipc)
{
	SOCKET		fd;
	struct sockaddr_in	saddr;
/*
	ADDRINFO	addrInfo;
	ADDRINFO*	ai_result = NULL;

	// AI_PASSIVE     // Socket address will be used in bind() call
	// AI_CANONNAME   // Return canonical name in first ai_canonname
	// AI_NUMERICHOST // Nodename must be a numeric address string
	// AI_NUMERICSERV // Servicename must be a numeric port number
	memset(&addrInfo, 0, sizeof(addrInfo));
	addrInfo.ai_family		= AF_INET;
	addrInfo.ai_socktype	= SOCK_STREAM;
	addrInfo.ai_flags		= AI_NUMERICSERV | AI_PASSIVE;

	if (getaddrinfo ("localhost", UGET_SOCKET_PORT_S, &addrInfo, &ai_result))
		return FALSE;

	fd = socket (AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET)
		return FALSE;

	if (bind(fd, ai_result->ai_addr, ai_result->ai_addrlen) == SOCKET_ERROR) {
		closesocket (fd);
		return FALSE;
	}
*/

	if (ipc->channel)
		return FALSE;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons (UGET_SOCKET_PORT);
	saddr.sin_addr.S_un.S_addr = inet_addr ("127.0.0.1");

	fd = socket (AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET)
		return FALSE;

	if (bind (fd, (struct sockaddr *)&saddr, sizeof (saddr)) == SOCKET_ERROR) {
		closesocket (fd);
		return FALSE;
	}
	if (listen (fd, 5) == SOCKET_ERROR) {
		closesocket (fd);
		return FALSE;
	}

	ipc->server_inited = TRUE;
	ipc->channel = g_io_channel_win32_new_socket (fd);
//	g_io_channel_set_flags (ipc->channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_encoding (ipc->channel, NULL, NULL);	// binary
	g_io_add_watch (ipc->channel, G_IO_IN, (GIOFunc)server_accept, ipc);
	return TRUE;
}

gboolean	ug_ipc_use_client (UgIpc* ipc)
{
	SOCKET	fd;
	struct sockaddr_in	saddr;
//	int  error;

	if (ipc->channel)
		return FALSE;

	fd = socket (AF_INET, SOCK_STREAM, 0);
//	error = WSAGetLastError();
	if (fd == INVALID_SOCKET)
		return FALSE;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons (UGET_SOCKET_PORT);
	saddr.sin_addr.S_un.S_addr = inet_addr ("127.0.0.1");

	if (connect (fd, (struct sockaddr *) &saddr, sizeof(saddr)) == SOCKET_ERROR) {
		closesocket (fd);
		return FALSE;
	}

	ipc->server_inited = FALSE;
	ipc->channel = g_io_channel_win32_new_socket (fd);
//	g_io_channel_set_flags (ipc->channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_encoding (ipc->channel, NULL, NULL);	// binary
	return TRUE;
}
// End of Windows code ----------------
#else
// UNIX code --------------------------
#include <sys/un.h>			// struct sockaddr_un
#include <sys/socket.h>		// socket api
#include <unistd.h>			// uid_t and others
#include <errno.h>

#define SOCKET			int
#define INVALID_SOCKET	(-1)
#define closesocket		close

gboolean	ug_ipc_use_server (UgIpc* ipc)
{
	int		fd;
	struct sockaddr_un	saddr;

	if (ipc->channel)
		return FALSE;

	fd = socket (AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
		return FALSE;

	saddr.sun_family = AF_UNIX;
	strncpy (saddr.sun_path, ipc->file, sizeof (saddr.sun_path) -1);
	saddr.sun_path [sizeof (saddr.sun_path) -1] = 0;

	// --------------------------------
	// if server exist, return FALSE.
	if (connect (fd, (struct sockaddr *) &saddr, sizeof(saddr)) == 0) {
		close (fd);
		return FALSE;
	}
	// --------------------------------
	// delete socket file before server start
	ug_unlink (saddr.sun_path);

	if (bind (fd, (struct sockaddr *)&saddr, sizeof (saddr)) == -1) {
		close (fd);
		return FALSE;
	}
	if (listen (fd, 5) == -1) {
		close (fd);
		return FALSE;
	}

	ipc->server_inited = TRUE;
	ipc->channel = g_io_channel_unix_new (fd);
//	g_io_channel_set_flags (ipc->channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_encoding (ipc->channel, NULL, NULL);	// binary
	g_io_add_watch (ipc->channel, G_IO_IN, (GIOFunc)server_accept, ipc);

	return TRUE;
}

gboolean	ug_ipc_use_client (UgIpc* ipc)
{
	int		fd;
	struct sockaddr_un	saddr;

	if (ipc->channel)
		return FALSE;

	fd = socket (AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1)
		return FALSE;

	saddr.sun_family = AF_UNIX;
	strncpy (saddr.sun_path, ipc->file, sizeof (saddr.sun_path) -1);
	saddr.sun_path [sizeof (saddr.sun_path) -1] = 0;

	if (connect (fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
		close (fd);
		return FALSE;
	}

	ipc->server_inited = FALSE;
	ipc->channel = g_io_channel_unix_new (fd);
//	g_io_channel_set_flags (ipc->channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_channel_set_encoding (ipc->channel, NULL, NULL);	// binary
	return TRUE;
}
// End of UNIX code -------------------
#endif  // _WIN32

void	ug_ipc_init (UgIpc* ipc, const gchar* socket_file)
{
#ifdef _WIN32
	ipc->file = NULL;
#else
// UNIX code --------------------------
	uid_t	stored_uid, euid;

	if (socket_file)
		ipc->file = g_strdup (socket_file);
	else {
		stored_uid = getuid();
		euid = geteuid();
		setuid(euid);
		ipc->file = g_strconcat (g_get_tmp_dir (), G_DIR_SEPARATOR_S,
		                         UGET_SOCKET_NAME, "-",
		                         g_get_user_name (), NULL);
		setreuid(stored_uid, euid);
	}
// End of UNIX code -------------------
#endif  // _WIN32

	ipc->server_inited = FALSE;
	ipc->channel = NULL;
	g_queue_init (&ipc->queue);
	ipc->queue_limit = 1024;
}

void	ug_ipc_finalize (UgIpc* ipc)
{
	SOCKET		ipc_fd;
	GList*		link;

	if (ipc->channel) {
		ipc_fd = g_io_channel_unix_get_fd (ipc->channel);
		g_io_channel_shutdown (ipc->channel, FALSE, NULL);
		g_io_channel_unref (ipc->channel);
		closesocket (ipc_fd);
		ipc->channel = NULL;
	}

	if (ipc->file) {
		if (ipc->server_inited)
			ug_unlink (ipc->file);
		g_free (ipc->file);
		ipc->file = NULL;
	}

	// free argument pointer and strings
	for (link = ipc->queue.head;  link;  link = link->next)
		ug_arg_free (link->data, TRUE);
	g_queue_clear (&ipc->queue);

	ipc->server_inited = FALSE;
}

// return  1: server start and push arguments to queue.
// return  0: server exist and send arguments to server.
// return -1: error
gint	ug_ipc_init_with_args (UgIpc* ipc, int argc, char** argv)
{
	guint		count;
	gboolean	workable;

	ug_ipc_init (ipc, NULL);
	// try to start server.
	if (ug_ipc_use_server (ipc)) {
		if (argc > 1)
			g_queue_push_tail (&ipc->queue, ug_arg_new (argc, argv, TRUE));
		return 1;
	}
	else {
		// connecting to server if server already exist.
		workable = ug_ipc_use_client (ipc);
		// try to send command-line options
		for (count=0;  count<3;  count++) {
			if (workable == FALSE) {
				g_usleep (500 * 1000);
				workable = ug_ipc_use_client (ipc);
				continue;
			}
			if (ug_ipc_ping (ipc) == FALSE) {
				g_usleep (500 * 1000);
				continue;
			}
			ug_ipc_send (ipc, argc, argv);
			return 0;
		}
	}

	return -1;
}


// ----------------------------------------------------------------------------
// client functions, ping server or send arguments to server.
gboolean	ug_ipc_ping (UgIpc* ipc)
{
	GIOStatus	status;
	gsize		n_bytes;
	gchar		header[3];

	header[0] = 'U';
	header[1] = 'G';
	header[2] = UG_IPC_PING;

	status = g_io_channel_write_chars (ipc->channel, header, 3, NULL, NULL);
	if (status == G_IO_STATUS_ERROR)
		return FALSE;
	g_io_channel_flush (ipc->channel, NULL);

	// get response
	g_usleep (100 * 1000);
	status = g_io_channel_read_chars (ipc->channel, header, 3, &n_bytes, NULL);
	if (n_bytes == 3 && header[0] == 'U' && header[1] == 'G' && header[2] == UG_IPC_OK)
		return TRUE;
	return FALSE;
}

gboolean	ug_ipc_send (UgIpc* ipc, int argc, char** argv)
{
	GIOStatus	status;
	GString*	gstr;
	gsize		n_bytes;
	gchar		header[3];
	int			index;

	// header, command
	gstr = g_string_sized_new (256);
	g_string_append (gstr, UGET_SOCKET_HEADER_S);
	g_string_append_c (gstr, UG_IPC_SEND);

	// argc, argv
	g_string_append_printf (gstr, "%d\n", argc);
	for (index=0;  index < argc;  index++) {
		g_string_append (gstr, argv[index]);
		g_string_append_c (gstr, '\n');
	}

	status = g_io_channel_write_chars (ipc->channel, gstr->str, gstr->len, NULL, NULL);
	g_string_free (gstr, TRUE);
	if (status == G_IO_STATUS_ERROR)
		return FALSE;
	g_io_channel_flush (ipc->channel, NULL);

	// get response
	g_usleep (100 * 1000);
	status = g_io_channel_read_chars (ipc->channel, header, 3, &n_bytes, NULL);
	if (n_bytes == 3 && header[0] == 'U' && header[1] == 'G' && header[2] == UG_IPC_OK)
		return TRUE;

	return FALSE;
}

// ----------------------------------------------------------------------------
// server functions, get arguments from queue.
GPtrArray*	ug_ipc_peek (UgIpc* ipc)
{
	return g_queue_peek_head (&ipc->queue);
}

GPtrArray*	ug_ipc_pop  (UgIpc* ipc)
{
	return g_queue_pop_head (&ipc->queue);
}

// ----------------------------------------------------------------------------
// server callback functions
static gboolean server_accept (GIOChannel *source, GIOCondition condition, UgIpc* ipc)
{
	SOCKET		source_fd;
	SOCKET		new_fd;
	GIOChannel*	new_channel;

	source_fd = g_io_channel_unix_get_fd (source);
	new_fd = accept (source_fd, NULL, NULL);
	if (new_fd == INVALID_SOCKET)
		return TRUE;

#ifdef _WIN32
	new_channel = g_io_channel_win32_new_socket (new_fd);
#else
	new_channel = g_io_channel_unix_new (new_fd);
#endif
	g_io_channel_set_encoding (new_channel, NULL, NULL);	// binary
	g_io_add_watch (new_channel, G_IO_IN | G_IO_HUP, (GIOFunc)server_connect, ipc);

	return TRUE;
}

static gboolean server_connect (GIOChannel *source, GIOCondition condition, UgIpc* ipc)
{
	SOCKET		source_fd;
	GIOStatus	status;
	GString*	gstr;
	GPtrArray*	args;
	gint		args_count;
	gsize		read_count;
	gchar		header[3];

	if (condition == G_IO_HUP)
		goto shutdown;

	g_io_channel_read_chars (source, header, 3, &read_count, NULL);
	if (read_count < 3)
		goto shutdown;
	if (header[0] != 'U' || header[1] != 'G') {
#ifndef NDEBUG
		g_print ("uGet IPC server: '%c%c' is NOT 'UG' header\n", header[0], header[1]);
#endif
		goto shutdown;
	}

#ifndef NDEBUG
	g_print ("uGet IPC server: get message code '%d' from client.\n", header[2]);
#endif

	switch (header[2]) {
	case UG_IPC_SEND:
		// read argument counts
		gstr = g_string_sized_new (256);
		g_io_channel_read_line_string (source, gstr, &read_count, NULL);
		if (read_count == 0) {
			g_string_free (gstr, TRUE);
			goto error;
		}
		args_count = atoi (gstr->str);
		// read arguments
		args = g_ptr_array_sized_new (args_count);
		while (args_count) {
			status = g_io_channel_read_line_string (source, gstr, &read_count, NULL);
			if (status == G_IO_STATUS_ERROR)
				break;
			gstr->str [read_count] = 0;		// clear '\n' in tail
			g_ptr_array_add (args, g_strndup (gstr->str, gstr->len));
			args_count--;
		}
		// push to queue. If queue full, response busy.
		if (ipc->queue.length < ipc->queue_limit)
			g_queue_push_tail (&ipc->queue, args);
		else {
			header[2] = UG_IPC_BUSY;
			g_io_channel_write_chars (source, header, 3, NULL, NULL);
			g_io_channel_flush (source, NULL);
			// discard arguments
			ug_arg_free (args, TRUE);
			break;
		}
		// break;	// don't break, I need response UG_IPC_OK

	case UG_IPC_PING:
		header[2] = UG_IPC_OK;
		g_io_channel_write_chars (source, header, 3, NULL, NULL);
		g_io_channel_flush (source, NULL);
		break;

	default:
		goto error;
	}

	return TRUE;

error:
	header[0] = 'U';
	header[1] = 'G';
	header[2] = UG_IPC_ERROR;
	g_io_channel_write_chars (source, header, 3, NULL, NULL);
	g_io_channel_flush (source, NULL);
	return TRUE;

shutdown:
#ifndef NDEBUG
	g_print ("uGet IPC server: close connection\n");
#endif
	source_fd = g_io_channel_unix_get_fd (source);
	g_io_channel_shutdown (source, TRUE, NULL);
	g_io_channel_unref (source);
	closesocket (source_fd);
	return FALSE;
}

