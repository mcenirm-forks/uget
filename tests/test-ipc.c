#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <winsock.h>
#else
#include <unistd.h>
#endif

#include <UgOption.h>
#include <UgIpc.h>

#define	COUNT_LIMIT		3

static gboolean server_timer  (gpointer);
static gpointer client_thread (gpointer);

GMainLoop*	main_loop;

// server
static gboolean server_timer (gpointer data)
{
	static UgIpc	ipc;
	static guint	count  = 0;
	static gboolean	inited = FALSE;
	GPtrArray*		args;

	if (inited == FALSE) {
		inited = TRUE;
		g_print ("Startup uGet IPC server...\n");
		ug_ipc_init (&ipc, NULL);
		ug_ipc_server_start (&ipc);
	}

	if (count < COUNT_LIMIT) {
		args = ug_ipc_pop (&ipc);
		if (args) {
			ug_arg_free (args, TRUE);
			count++;
		}
		return TRUE;
	}

	g_print ("Shutdown uGet IPC server...\n");
	// server shutdown
	ug_ipc_finalize (&ipc);
	// exit main loop
	g_main_loop_quit (main_loop);
	// exit thread
	return FALSE;
}

// client
static gpointer client_thread (gpointer data)
{
	UgIpc	ipc;
	guint	count;
	gchar*	string = "http://test.org/index.htm";

	g_print ("Connect to uGet IPC server...\n");
	ug_ipc_init (&ipc, NULL);
	ug_ipc_client_connect (&ipc);

	for (count=0;  count < COUNT_LIMIT;  count++) {
		// sleep 500,000 microseconds (0.5 seconds)
		g_usleep (500 * 1000);

		// ping
		g_print ("uGet IPC client: ping...\n");
		if (ug_ipc_ping (&ipc) == FALSE)
			g_print ("uGet IPC client: ping fail\n");
		else
			g_print ("uGet IPC client: ping ok\n");
		// send
		g_print ("uGet IPC client: send...\n");
		if (ug_ipc_send (&ipc, 1, &string) == FALSE)
			g_print ("uGet IPC client: send fail\n");
		else
			g_print ("uGet IPC client: send ok\n");
	}

	g_print ("Disconnect...\n");
	// disconnect
	ug_ipc_finalize (&ipc);

	return NULL;
}


int main (int argc, char* argv[])
{
#ifdef _WIN32
	WSADATA WSAData;

	WSAStartup (MAKEWORD (2, 2), &WSAData);
#endif

	// GLib: GThread
	if (g_thread_supported () == FALSE)
		g_thread_init (NULL);

	main_loop = g_main_loop_new (NULL, FALSE);

	// start server
	server_timer (NULL);
	g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 500, (GSourceFunc) server_timer, NULL, NULL);
	// start client thread
	g_thread_create ((GThreadFunc) client_thread, NULL, FALSE, NULL);

	g_main_loop_run (main_loop);

#ifdef _WIN32
	WSACleanup ();
#endif

	return 0;	// EXIT_SUCCESS
}
