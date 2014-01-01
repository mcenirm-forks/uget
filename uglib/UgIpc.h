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

#ifndef UG_IPC_H
#define UG_IPC_H

#include <glib.h>

#define UGET_SOCKET_PORT		14777
#define UGET_SOCKET_PORT_S		"14777"
#define UGET_SOCKET_NAME		"uGetIPC"
#define UGET_SOCKET_HEADER_S	"UG"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgIpc			UgIpc;
typedef enum	UgIpcCode		UgIpcCode;

struct UgIpc
{
	GIOChannel*		channel;
	gboolean		server_inited;

	// server queue
	GQueue			queue;
	guint			queue_limit;

	// UNIX
	gchar*			file;
};

enum UgIpcCode
{
	UG_IPC_ERROR,
	UG_IPC_OK,
	UG_IPC_BUSY,
	UG_IPC_PING,
	UG_IPC_SEND,
};

void	ug_ipc_init (UgIpc* ipc, const gchar* socket_file);
void	ug_ipc_finalize (UgIpc* ipc);

// return  1: server start and push arguments to queue.
// return  0: server exist and send arguments to server.
// return -1: error
gint	ug_ipc_init_with_args (UgIpc* ipc, int argc, char** argv);

// client or server
gboolean	ug_ipc_use_server (UgIpc* ipc);
gboolean	ug_ipc_use_client (UgIpc* ipc);

// use with ug_ipc_use_client()
// client functions, ping server or send arguments to server.
gboolean	ug_ipc_ping (UgIpc* ipc);
gboolean	ug_ipc_send (UgIpc* ipc, int argc, char** argv);

// use with ug_ipc_use_server()
// server functions, get arguments from queue.
GPtrArray*	ug_ipc_peek (UgIpc* ipc);
GPtrArray*	ug_ipc_pop  (UgIpc* ipc);


#ifdef __cplusplus
}
#endif

#endif	// UG_IPC_H

