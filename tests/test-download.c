#include <stdlib.h>		// exit(), EXIT_SUCCESS, EXIT_FAILURE

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <winsock.h>
#endif  // _WIN32

#include <UgApp-base.h>
#include <UgOption.h>
#include <UgDataset.h>
#include <UgData-download.h>
#include <UgPlugin.h>


static void  dump_progress (UgProgress* progress)
{
#ifdef _WIN32
	printf  ("complete  size : %I64d byte\n", progress->complete);
#else
	g_print ("complete  size : %lld byte\n", (long long int) progress->complete);
#endif

#ifdef _WIN32
	printf  ("total size     : %I64d byte\n", progress->total);
#else
	g_print ("total size     : %lld byte\n", (long long int) progress->total);
#endif
	g_print ("speed (upload) : %.2f byte/sec\n", (double) progress->upload_speed);
	g_print ("speed          : %.2f byte/sec\n", (double) progress->download_speed);
	g_print ("percent        : %.2f%%\n", progress->percent);
	g_print ("consumed time  : %.2f sec\n", progress->consume_time);
	g_print ("remain time    : %.2f sec\n", progress->remain_time);
}

static void plugin_callback (UgPlugin* plugin, const UgMessage* msg, gpointer user_data)
{
	UgProgress*	progress;

	switch (msg->type) {
	case UG_MESSAGE_STATE:
		g_print ("--- Message State : State changed to %d\n", msg->data.v_int);
		break;

	case UG_MESSAGE_PROGRESS:
		g_print ("--- Message Progress :\n");
		progress = ug_data_new (UG_PROGRESS_I);
		if (ug_plugin_get (plugin, UG_DATA_INSTANCE, progress) == UG_RESULT_OK)
			dump_progress (progress);
		ug_data_free (progress);
		break;

	case UG_MESSAGE_ERROR:
		g_print ("--- Message Error %d: %s\n", msg->code, msg->string);
		break;

	case UG_MESSAGE_WARNING:
		g_print ("--- Message Warning %d: %s\n", msg->code, msg->string);
		break;

	case UG_MESSAGE_INFO:
		g_print ("--- Message Info %d: %s\n", msg->code, msg->string);
		break;

	case UG_MESSAGE_DATA:
		switch (msg->code) {
		case UG_MESSAGE_DATA_FILE_CHANGED:
			g_print ("--- Message Data : File changed to %s\n", msg->data.v_string);
			break;
		// HTTP message
		case UG_MESSAGE_DATA_HTTP_LOCATION:
			g_print ("--- Message Data : HTTP Location: %s\n", msg->data.v_string);
			break;
		case UG_MESSAGE_DATA_HTTP_CONTENT_LOCATION:
			g_print ("--- Message Data : HTTP Content-Location: %s\n", msg->data.v_string);
			break;
		case UG_MESSAGE_DATA_HTTP_CONTENT_DISPOSITION:
			g_print ("--- Message Data : HTTP Content-disposition: %s\n", msg->data.v_string);
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}


int main (int argc, char* argv[])
{
	const UgOptionEntry*	uoentry;
	UgDataset*		dataset;
	UgDataCommon*	common;
	UgPlugin*		plugin;

	GOptionContext*	context;
	GOptionGroup*	group;
	GError*			error = NULL;

#ifdef _WIN32	//	curl_global_init() will do this
	WSADATA WSAData;

	WSAStartup (MAKEWORD (2, 2), &WSAData);
#endif

	uglib_init ();

	// parse command-line options
	context = g_option_context_new ("[URL]");
	group	= g_option_group_new (NULL, NULL, NULL, NULL, NULL);
	uoentry = ug_option_get_main_entry ();
	g_option_group_add_entries (group, uoentry->entry);
	g_option_context_set_main_group (context, group);
	if (g_option_context_parse (context, &argc, &argv, &error) == FALSE) {
		g_print ("Option parsing failed : %s\n", error->message);
		exit (EXIT_FAILURE);		// EXIT_FAILURE == 1
	}

	// get URL from remained arguments
	if (argc == 1) {
		g_print ("%s", g_option_context_get_help (context, TRUE, NULL));
		exit (EXIT_FAILURE);		// EXIT_FAILURE == 1
	}
	dataset = ug_dataset_new ();
	ug_option_entry_get (uoentry, dataset);
	common = ug_dataset_realloc (dataset, UG_DATA_COMMON_I, 0);
	common->url = g_strdup (argv[1]);

	plugin = ug_plugin_new_by_data (dataset);
	ug_dataset_unref (dataset);
	if (plugin) {
		ug_plugin_set_state (plugin, UG_STATE_ACTIVE);
		while (ug_plugin_dispatch (plugin, plugin_callback, NULL))
			ug_plugin_delay (plugin, 1000);
		ug_plugin_unref (plugin);
	}

	uglib_finalize ();
#ifdef _WIN32
	WSACleanup ();
#endif

	return EXIT_SUCCESS;		// EXIT_SUCCESS == 0
}

