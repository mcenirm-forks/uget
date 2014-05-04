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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_PLUGIN_CURL

#include <string.h>
#include <UgPlugin-curl.h>
#include <UgetData.h>
#include <UgStdio.h>
#include <UgUtils.h>
#include <UgString.h>
#include <UgUri.h>

#ifdef HAVE_LIBPWMD
#include "pwmd.h"
static gboolean	ug_plugin_curl_set_proxy_pwmd (UgPluginCurl *plugin);
#endif  // HAVE_LIBPWMD

#define EMIT_PROGRESS_COUNT				2

enum SchemeType
{
	SCHEME_HTTP,
	SCHEME_FTP,
};

/*    defined in curl.h
typedef int (*curl_progress_callback)(void *clientp,
                                      double dltotal,
                                      double dlnow,
                                      double ultotal,
                                      double ulnow);
typedef size_t (*curl_write_callback)(char *buffer,
                                      size_t size,
                                      size_t nitems,
                                      void *outstream);
typedef size_t (*curl_read_callback)(char *buffer,
                                     size_t size,
                                     size_t nitems,
                                     void *instream);
*/

// functions for UgPluginInterface
static gboolean	ug_plugin_curl_global_init		(void);
static void		ug_plugin_curl_global_finalize	(void);

static gboolean	ug_plugin_curl_init			(UgPluginCurl* plugin, UgDataset* dataset);
static void		ug_plugin_curl_finalize		(UgPluginCurl* plugin);

static UgResult	ug_plugin_curl_set_state	(UgPluginCurl* plugin, UgState  state);
static UgResult	ug_plugin_curl_get_state	(UgPluginCurl* plugin, UgState* state);
static UgResult	ug_plugin_curl_set			(UgPluginCurl* plugin, guint parameter, gpointer data);
static UgResult	ug_plugin_curl_get			(UgPluginCurl* plugin, guint parameter, gpointer data);

// thread and setup functions
static gpointer	ug_plugin_curl_thread		(UgPluginCurl* plugin);
static gboolean	ug_plugin_curl_open_file	(UgPluginCurl* plugin, UgetCommon* common);
static void		ug_plugin_curl_set_scheme	(UgPluginCurl* plugin, CURL* curl);
static void		ug_plugin_curl_set_proxy	(UgPluginCurl* plugin, CURL* curl);

// libcurl callback functions
static int		ug_plugin_curl_progress		(UgPluginCurl* plugin, double  dltotal, double  dlnow, double  ultotal, double  ulnow);
static size_t	ug_plugin_curl_header_http	(char *buffer, size_t size, size_t nmemb, UgPluginCurl *plugin);

// static data for UgPluginInterface
static const char*	supported_schemes[] = {"http", "https", "ftp", "ftps", NULL};
// extern
const	UgPluginInterface	ug_plugin_curl_iface =
{
	sizeof (UgPluginCurl),										// instance_size
	"curl",														// name

	supported_schemes,											// schemes
	NULL,														// file_types

	(UgGlobalInitFunc)		ug_plugin_curl_global_init,			// global_init
	(UgGlobalFinalizeFunc)	ug_plugin_curl_global_finalize,		// global_finalize
	(UgGlobalSetFunc)		NULL,								// global_set
	(UgGlobalGetFunc)		NULL,								// global_get

	(UgPluginInitFunc)		ug_plugin_curl_init,				// init
	(UgFinalizeFunc)		ug_plugin_curl_finalize,			// finalize

	(UgSetStateFunc)		ug_plugin_curl_set_state,			// set_state
	(UgGetStateFunc)		ug_plugin_curl_get_state,			// get_state
	(UgSetFunc)				ug_plugin_curl_set,					// set
	(UgGetFunc)				ug_plugin_curl_get,					// get
};


// ----------------------------------------------------------------------------
// functions for UgPluginInterface
static gboolean	ug_plugin_curl_global_init (void)
{
	curl_global_init (CURL_GLOBAL_ALL);
	return TRUE;
}

static void	ug_plugin_curl_global_finalize (void)
{
	curl_global_cleanup ();
}

static gboolean	ug_plugin_curl_init (UgPluginCurl* plugin, UgDataset* dataset)
{
	UgetCommon*	common;
	UgetHttp*		http;

	common = ug_dataset_get (dataset, UgetCommonInfo, 0);
	http   = ug_dataset_get (dataset, UgetHttpInfo, 0);
	// check data
	if (common == NULL  ||  common->url == NULL)
		return FALSE;
	// reset
	if (common)
		common->retry_count = 0;
	if (http)
		http->redirection_count = 0;
	// copy supported data
	plugin->common = ug_datalist_copy (ug_dataset_get (dataset, UgetCommonInfo, 0));
	plugin->proxy  = ug_datalist_copy (ug_dataset_get (dataset, UgetProxyInfo, 0));
	plugin->http   = ug_datalist_copy (ug_dataset_get (dataset, UgetHttpInfo, 0));
	plugin->ftp    = ug_datalist_copy (ug_dataset_get (dataset, UgetFtpInfo, 0));

	// default values
	plugin->resumable = TRUE;
	// initial curl
	plugin->curl = curl_easy_init ();

	return TRUE;
}

static void	ug_plugin_curl_finalize (UgPluginCurl* plugin)
{
	// free data
	ug_datalist_free (plugin->common);
	ug_datalist_free (plugin->proxy);
	ug_datalist_free (plugin->http);
	ug_datalist_free (plugin->ftp);
	// clear curl
	curl_easy_cleanup (plugin->curl);
}

static UgResult	ug_plugin_curl_set_state (UgPluginCurl* plugin, UgState  state)
{
	UgState  old_state;
	GThread* thread;

	old_state = plugin->state;
	// change plug-in status
	if (state != old_state) {
		if (state == UG_STATE_ACTIVE && old_state < UG_STATE_ACTIVE) {
			// call ug_plugin_unref () by ug_plugin_curl_thread ()
			ug_plugin_ref ((UgPlugin*) plugin);
			// create thread
			thread = g_thread_try_new ("uget-curl",
					(GThreadFunc) ug_plugin_curl_thread, plugin, NULL);
			if (thread == NULL) {
				ug_plugin_unref ((UgPlugin*) plugin);
				return UG_RESULT_ERROR;
			}
			g_thread_unref (thread);
		}
		plugin->state = state;
		ug_plugin_post ((UgPlugin*) plugin, ug_message_new_state (state));
	}

	return UG_RESULT_OK;
}

static UgResult	ug_plugin_curl_get_state (UgPluginCurl* plugin, UgState* state)
{
	if (state) {
		*state = plugin->state;
		return UG_RESULT_OK;
	}

	return UG_RESULT_ERROR;
}

static UgResult	ug_plugin_curl_set (UgPluginCurl* plugin, guint parameter, gpointer data)
{
	gint64	speed_limit;

	if (parameter != UG_TYPE_INT64)
		return UG_RESULT_UNSUPPORT;

	speed_limit = *(gint64*)data;
	curl_easy_setopt (plugin->curl, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t) speed_limit);
	curl_easy_setopt (plugin->curl, CURLOPT_MAX_SEND_SPEED_LARGE, (curl_off_t) speed_limit);

	return UG_RESULT_OK;
}

static UgResult	ug_plugin_curl_get (UgPluginCurl* plugin, guint parameter, gpointer data)
{
	UgetProgress*	progress;
	gdouble		complete;
	gdouble		total;
	gdouble		speed;

	if (parameter != UG_TYPE_INSTANCE)
		return UG_RESULT_UNSUPPORT;
	if (data == NULL || ((UgData*)data)->iface != UgetProgressInfo)
		return UG_RESULT_UNSUPPORT;

	progress = data;
	curl_easy_getinfo (plugin->curl, CURLINFO_TOTAL_TIME, &progress->consume_time);
	curl_easy_getinfo (plugin->curl, CURLINFO_SIZE_DOWNLOAD, &complete);
	curl_easy_getinfo (plugin->curl, CURLINFO_SPEED_DOWNLOAD, &speed);
	progress->download_speed = (gint64) speed;
	complete = plugin->file_beg + complete;
	total    = plugin->file_end;
	// if total size is unknown, use completed size.
	if (complete > total)
		total = complete;
	// If total size is unknown, don't calculate percent.
	if (total > 0)
		progress->percent = complete * 100 / total;
	else
		progress->percent = 0;
	// If total size and average speed is unknown, don't calculate remain time.
	if (progress->download_speed > 0 && total > 0)
		progress->remain_time = (gdouble) ((total - complete) / progress->download_speed);
	// others
	progress->complete = (gint64) complete;
	progress->total    = (gint64) total;

	return UG_RESULT_OK;
}


// ----------------------------------------------------------------------------
// thread and setup functions
static gpointer	ug_plugin_curl_thread (UgPluginCurl* plugin)
{
	CURL*			curl;
	CURLcode		curl_code;
	long			curl_time;
	long			response_code;
	UgetCommon*	common;
	UgetProxy*	proxy;
	UgetHttp*		http;
	UgetFtp*		ftp;

	// supported data
	common = plugin->common;
	proxy = plugin->proxy;
	http = plugin->http;
	ftp = plugin->ftp;
	// curl
	curl = plugin->curl;
	curl_code = CURLE_FAILED_INIT;

	// Common option ----------------------------------------------------------
	curl_easy_setopt (curl, CURLOPT_URL, common->url);
	curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1L);
	// setup scheme and it's related data (user & password)
	ug_plugin_curl_set_scheme (plugin, curl);
	// check filename
	if (common->file)
		plugin->keep_filename = TRUE;
	else {
		common->file = ug_uri_get_filename (common->url);
		if (common->file == NULL)
			common->file = g_strdup ("index.htm");
		ug_plugin_post ((UgPlugin*) plugin,
				ug_message_new_data (UG_MESSAGE_DATA_FILE_CHANGED, common->file));
		plugin->keep_filename = FALSE;
	}
	if (common->max_download_speed) {
		curl_easy_setopt (curl, CURLOPT_MAX_SEND_SPEED_LARGE,
				(curl_off_t) common->max_upload_speed);
		curl_easy_setopt (curl, CURLOPT_MAX_RECV_SPEED_LARGE,
				(curl_off_t) common->max_download_speed);
	}

	// Proxy option -----------------------------------------------------------
	if (proxy) {
		// proxy type
		switch (proxy->type) {
		case UGET_PROXY_NONE:
			curl_easy_setopt (curl, CURLOPT_PROXYTYPE, 0);
			break;

		default:
		case UGET_PROXY_DEFAULT:
		case UGET_PROXY_HTTP:
			curl_easy_setopt (curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			ug_plugin_curl_set_proxy (plugin, curl);
			break;

		case UGET_PROXY_SOCKS4:
			curl_easy_setopt (curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			ug_plugin_curl_set_proxy (plugin, curl);
			break;

		case UGET_PROXY_SOCKS5:
			curl_easy_setopt (curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
			ug_plugin_curl_set_proxy (plugin, curl);
			break;

#ifdef HAVE_LIBPWMD
		case UGET_PROXY_PWMD:
			if (ug_plugin_curl_set_proxy_pwmd (plugin) == FALSE)
				goto exit;
			break;
#endif
		}
	}

	// HTTP option ------------------------------------------------------------
	// use libcurl's redirection.
  	curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt (curl, CURLOPT_AUTOREFERER, 1L);
	curl_easy_setopt (curl, CURLOPT_MAXREDIRS, 30L);

	if (http) {
		if (http->referrer == NULL)
			http->referrer = g_strndup (common->url, ug_uri_referrer_len (common->url));
		curl_easy_setopt (curl, CURLOPT_REFERER, http->referrer);
//		if (http->redirection_limit)
		curl_easy_setopt (curl, CURLOPT_MAXREDIRS, http->redirection_limit);

		if (http->user_agent)
			curl_easy_setopt (curl, CURLOPT_USERAGENT, http->user_agent);
//		else {
//			curl_easy_setopt (curl, CURLOPT_USERAGENT,
//					"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1)");
//			curl_easy_setopt (curl, CURLOPT_USERAGENT,
//					"Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729)");
//		}

		// cookie
		if (http->cookie_data)
			curl_easy_setopt (curl, CURLOPT_COOKIE, http->cookie_data);
		else if (http->cookie_file)
			curl_easy_setopt (curl, CURLOPT_COOKIEFILE, http->cookie_file);
		else
			curl_easy_setopt (curl, CURLOPT_COOKIEFILE, "");

		if (http->post_data) {
			curl_easy_setopt (curl, CURLOPT_POST, 1L);
			curl_easy_setopt (curl, CURLOPT_POSTFIELDS, http->post_data);
			curl_easy_setopt (curl, CURLOPT_POSTFIELDSIZE, strlen (http->post_data));
		}
		else if (http->post_file) {
			plugin->file_stream_in = ug_fopen (http->post_file, "r");
			if (plugin->file_stream_in) {
				curl_easy_setopt (curl, CURLOPT_POST, 1L);
				curl_easy_setopt (curl, CURLOPT_READDATA, plugin->file_stream_in);
#if defined(_WIN32) && !defined(__MINGW32__)	// for MS VC only
				curl_easy_setopt (curl, CURLOPT_READFUNCTION , fread);
#endif
			}
			else {
				ug_plugin_post ((UgPlugin*) plugin,
						ug_message_new_error (UG_MESSAGE_ERROR_HTTP_OPEN_POSTED_FILE_FAILED, NULL));
				goto exit;
			}
		}
	}

	// FTP option -------------------------------------------------------------
	if (ftp && ftp->active_mode) {
		// FTP Active Mode
		// use EPRT and then LPRT before using PORT
		curl_easy_setopt (curl, CURLOPT_FTP_USE_EPRT, TRUE);
		// '-' symbol to let the library use your system's default IP address.
		curl_easy_setopt (curl, CURLOPT_FTPPORT, "-");
	}
	else {
		// FTP Passive Mode
		// use PASV command in IPv4 server. (Passive Mode)
		// don't use EPSV command. (Extended Passive Mode)
		curl_easy_setopt (curl, CURLOPT_FTP_USE_EPSV, FALSE);
		// don't use EPRT and LPRT command.
		curl_easy_setopt (curl, CURLOPT_FTP_USE_EPRT, FALSE);
		// don't use PORT command.
		curl_easy_setopt (curl, CURLOPT_FTPPORT, NULL);
	}

	// Progress  --------------------------------------------------------------
	curl_easy_setopt (curl, CURLOPT_PROGRESSFUNCTION, (curl_progress_callback) ug_plugin_curl_progress);
	curl_easy_setopt (curl, CURLOPT_PROGRESSDATA, plugin);
	curl_easy_setopt (curl, CURLOPT_NOPROGRESS, FALSE);

	// Others -----------------------------------------------------------------
	curl_easy_setopt (curl, CURLOPT_FILETIME, 1L);	// use with CURLINFO_FILETIME
	curl_easy_setopt (curl, CURLOPT_SSL_VERIFYPEER, FALSE);	// disable peer SSL certificate verification
	curl_easy_setopt (curl, CURLOPT_ERRORBUFFER, plugin->curl_error_string);
	curl_easy_setopt (curl, CURLOPT_LOW_SPEED_LIMIT, 100);
	curl_easy_setopt (curl, CURLOPT_LOW_SPEED_TIME, 60);

	while (plugin->state == UG_STATE_ACTIVE) {
		// set/reset progress count.
		// This will emit progress message when callback is called first time.
		plugin->progress_count = EMIT_PROGRESS_COUNT - 1;
		// open and create output file
		if (ug_plugin_curl_open_file (plugin, common) == FALSE)
			break;
		plugin->file_beg = (gdouble) plugin->file_offset;

		// perform
		curl_code = curl_easy_perform (curl);

		// get size of output file
		ug_fseek (plugin->file_stream, 0, SEEK_END);
		plugin->file_offset_old = plugin->file_offset;
		plugin->file_offset = ug_ftell (plugin->file_stream);

		// server response code
		response_code = 0;
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &response_code);
		// HTTP response code
		// 4xx Client Error
		// 5xx Server Error
		if (plugin->scheme == SCHEME_HTTP  &&  response_code >= 400) {
			gchar*	string;

			string = g_strdup_printf ("Server response code : %ld", response_code);
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_CUSTOM, string));
			g_free (string);
			// discard data if error
			plugin->file_offset = plugin->file_offset_old;
			ug_fseek (plugin->file_stream, plugin->file_offset, SEEK_SET);
			ug_fs_truncate (plugin->file_stream, plugin->file_offset);
			// Don't post completed message.
			curl_code = CURLE_ABORTED_BY_CALLBACK;
			break;
		}

		// curl_easy_perform() returned code.
		switch (curl_code) {
		case CURLE_OK:
			ug_plugin_post ((UgPlugin*) plugin, ug_message_new_progress ());
			goto exit;

		// can resume (retry)
		case CURLE_RECV_ERROR:
		case CURLE_PARTIAL_FILE:
		case CURLE_OPERATION_TIMEDOUT:
			// update status
			plugin->resumable = TRUE;
			// send message
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_info (UG_MESSAGE_INFO_RESUMABLE, NULL));
			break;

		// can't resume (retry)
		case CURLE_RANGE_ERROR:
		case CURLE_BAD_DOWNLOAD_RESUME:
			// update status
			plugin->resumable = FALSE;
			// discard data
			plugin->file_offset = 0;
			ug_fseek (plugin->file_stream, 0, SEEK_SET);
			ug_fs_truncate (plugin->file_stream, 0);
			// send message
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_info (UG_MESSAGE_INFO_NOT_RESUMABLE, NULL));
			break;

#if 0
		// can't connect (retry)
		case CURLE_COULDNT_CONNECT:
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_CONNECT_FAILED, plugin->curl_error_string));
			break;
#else
		case CURLE_COULDNT_CONNECT:
		case CURLE_COULDNT_RESOLVE_HOST:
#endif
		// retry
		case CURLE_SEND_ERROR:
		case CURLE_GOT_NOTHING:
		case CURLE_BAD_CONTENT_ENCODING:
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_CUSTOM, plugin->curl_error_string));
			break;

		// too many redirection (exit)
		case CURLE_TOO_MANY_REDIRECTS:
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_HTTP_TOO_MANY_REDIRECTIONS, NULL));
			goto exit;

		// abort by user (exit)
		case CURLE_ABORTED_BY_CALLBACK:
			goto exit;

		// out of resource (exit)
		case CURLE_OUT_OF_MEMORY:
		case CURLE_WRITE_ERROR:
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_OUT_OF_RESOURCE, plugin->curl_error_string));
			goto exit;

		// exit
		case CURLE_UNSUPPORTED_PROTOCOL:
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_UNSUPPORTED_SCHEME, plugin->curl_error_string));
			goto exit;

		// other error (exit)
#if 0
		case CURLE_COULDNT_RESOLVE_HOST:
#endif
		case CURLE_COULDNT_RESOLVE_PROXY:
		case CURLE_FAILED_INIT:
		case CURLE_URL_MALFORMAT:
		case CURLE_FTP_WEIRD_SERVER_REPLY:
		case CURLE_REMOTE_ACCESS_DENIED:
		default:
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_CUSTOM, plugin->curl_error_string));
			goto exit;
		}

		// retry
		if (common->retry_count < common->retry_limit || common->retry_limit == 0) {
			common->retry_count++;
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_info (UG_MESSAGE_INFO_RETRY, NULL));
			ug_plugin_delay ((UgPlugin*) plugin, common->retry_delay * 1000);
		}
		else {
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_TOO_MANY_RETRIES, NULL));
			break;
		}
	}

exit:
	if (plugin->file_stream) {
		fclose (plugin->file_stream);
		plugin->file_stream = NULL;
	}
	if (plugin->file_stream_in) {
		fclose (plugin->file_stream_in);
		plugin->file_stream_in = NULL;
	}

	if (curl_code == CURLE_OK) {
#ifdef HIDE_TEMPORARY_FILE
		ug_plugin_rename_and_unhide ((UgPlugin*) plugin, plugin->path_tmp, plugin->path);
		ug_delete_file (plugin->path);
#else
		ug_plugin_rename_file ((UgPlugin*) plugin, plugin->path_tmp, plugin->path);
#endif
		if (common->retrieve_timestamp) {
			curl_easy_getinfo (curl, CURLINFO_FILETIME, &curl_time);
			if (curl_time != -1)
				ug_modify_file_time (plugin->path, (time_t) curl_time);
		}
		ug_plugin_post ((UgPlugin*) plugin,
				ug_message_new_info (UG_MESSAGE_INFO_COMPLETE, NULL));
		ug_plugin_post ((UgPlugin*) plugin,
				ug_message_new_info (UG_MESSAGE_INFO_FINISH, NULL));
	}
	else if (plugin->path) {
		// delete empty downloaded file.
		if (plugin->file_offset == 0)
			ug_unlink (plugin->path_tmp);
		ug_unlink (plugin->path);
	}
	// free string
	g_free (plugin->path_tmp);
	g_free (plugin->path);
	plugin->path_tmp = NULL;
	plugin->path     = NULL;

	if (plugin->state == UG_STATE_ACTIVE)
		ug_plugin_curl_set_state (plugin, UG_STATE_READY);
	// call ug_plugin_ref () by ug_plugin_curl_set_state ()
	ug_plugin_unref ((UgPlugin*) plugin);
	return NULL;
}

static gboolean	ug_plugin_curl_open_file (UgPluginCurl* plugin, UgetCommon* common)
{
	if (plugin->file_stream == NULL) {
		g_free (plugin->path);
#ifdef HIDE_TEMPORARY_FILE
		plugin->path = ug_plugin_create_and_hide ((UgPlugin*) plugin,
				common->folder, common->file, &plugin->path_folder_len);
#else
		plugin->path = ug_plugin_create_file ((UgPlugin*) plugin,
				common->folder, common->file, &plugin->path_folder_len);
#endif
		if (plugin->path == NULL)
			return FALSE;

		g_free (plugin->path_tmp);
		plugin->path_tmp = g_strconcat (plugin->path, ".ug_", NULL);
//		if (plugin->resumable)
			plugin->file_stream = ug_fopen (plugin->path_tmp, "ab+");
//		else
//			plugin->file_stream = ug_fopen (plugin->path_tmp, "wb");

		// if file open failed
		if (plugin->file_stream == NULL) {
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_error (UG_MESSAGE_ERROR_FILE_OPEN_FAILED, NULL));
			ug_unlink (plugin->path);
			return FALSE;
		}

//		if (plugin->resumable) {
			ug_fseek (plugin->file_stream, 0, SEEK_END);
			plugin->file_offset = ug_ftell (plugin->file_stream);
//		} else
//			plugin->file_offset = 0;
	}

#if defined(_WIN32) && !defined(__MINGW32__)	// for MS VC only
	curl_easy_setopt (plugin->curl, CURLOPT_WRITEFUNCTION , fwrite);
#endif
	curl_easy_setopt (plugin->curl, CURLOPT_WRITEDATA , plugin->file_stream);
	curl_easy_setopt (plugin->curl, CURLOPT_RESUME_FROM_LARGE, plugin->file_offset);
	return TRUE;
}

// setup scheme and it's related data (user & password)
static void	ug_plugin_curl_set_scheme (UgPluginCurl* plugin, CURL* curl)
{
	UgetCommon*	common;
	UgetHttp*		http;
	UgetFtp*		ftp;

	common = plugin->common;
	http   = plugin->http;
	ftp    = plugin->ftp;

	// clear header callback
	curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, NULL);
	curl_easy_setopt (curl, CURLOPT_HEADERDATA, NULL);
//	curl_easy_setopt (curl, CURLOPT_WRITEHEADER, NULL);		// CURLOPT_WRITEHEADER == CURLOPT_HEADERDATA

	if (common->user || common->password) {
		// set user & password by common data
		curl_easy_setopt (curl, CURLOPT_USERNAME, (common->user)     ? common->user     : "");
		curl_easy_setopt (curl, CURLOPT_PASSWORD, (common->password) ? common->password : "");
		curl_easy_setopt (curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
	}
	else {
		// clear user & password
		curl_easy_setopt (curl, CURLOPT_USERNAME, NULL);
		curl_easy_setopt (curl, CURLOPT_PASSWORD, NULL);
		curl_easy_setopt (curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	}

	if (g_ascii_strncasecmp (common->url, "http", 4) == 0) {
		plugin->scheme = SCHEME_HTTP;
		// set HTTP user & password
		if (http && (http->user || http->password) ) {
			curl_easy_setopt (curl, CURLOPT_USERNAME, (http->user)     ? http->user     : "");
			curl_easy_setopt (curl, CURLOPT_PASSWORD, (http->password) ? http->password : "");
			curl_easy_setopt (curl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
		}
		// set header callback for HTTP
		curl_easy_setopt (curl, CURLOPT_HEADERFUNCTION, (curl_write_callback) ug_plugin_curl_header_http);
		curl_easy_setopt (curl, CURLOPT_HEADERDATA, plugin);
//		curl_easy_setopt (curl, CURLOPT_WRITEHEADER, plugin);	// CURLOPT_WRITEHEADER == CURLOPT_HEADERDATA
	}
	else if (g_ascii_strncasecmp (common->url, "ftp", 3) == 0) {
		plugin->scheme = SCHEME_FTP;
		// set FTP user & password
		if (ftp && (ftp->user || ftp->password)) {
			curl_easy_setopt (curl, CURLOPT_USERNAME, (ftp->user)     ? ftp->user     : "");
			curl_easy_setopt (curl, CURLOPT_PASSWORD, (ftp->password) ? ftp->password : "");
		}
	}
}

static void	ug_plugin_curl_set_proxy (UgPluginCurl* plugin, CURL* curl)
{
	UgetProxy*	proxy;

	proxy = plugin->proxy;
	curl_easy_setopt (curl, CURLOPT_PROXY, proxy->host);

	if (proxy->port)
		curl_easy_setopt (curl, CURLOPT_PROXYPORT, proxy->port);
	else
		curl_easy_setopt (curl, CURLOPT_PROXYPORT, 80);
	// proxy user and password
	if (proxy->user || proxy->password ||
		proxy->type == UGET_PROXY_SOCKS4 ||
		proxy->type == UGET_PROXY_SOCKS5)
	{
		curl_easy_setopt (curl, CURLOPT_PROXYUSERNAME,
				(proxy->user)     ? proxy->user     : "");
		curl_easy_setopt (curl, CURLOPT_PROXYPASSWORD,
				(proxy->password) ? proxy->password : "");
	}
	else {
		curl_easy_setopt (curl, CURLOPT_PROXYUSERNAME, NULL);
		curl_easy_setopt (curl, CURLOPT_PROXYPASSWORD, NULL);
	}
}


// ----------------------------------------------------------------------------
// libcurl callback functions
static int  ug_plugin_curl_progress (UgPluginCurl* plugin,
                                     double  dltotal, double  dlnow,
                                     double  ultotal, double  ulnow)
{
	// get file size
	plugin->file_end = plugin->file_beg + dltotal;
	// return if status changed
	if (plugin->state < UG_STATE_ACTIVE) {
		ug_plugin_post ((UgPlugin*) plugin, ug_message_new_progress ());
		return TRUE;    // Return TRUE if user aborted
	}
	// progress count
	plugin->progress_count++;
	if (plugin->progress_count == EMIT_PROGRESS_COUNT) {
		plugin->progress_count = 0;
		ug_plugin_post ((UgPlugin*) plugin, ug_message_new_progress ());
	}

	return 0;
}

// The HTTP method is supposed to be case-sensitive, but
// the HTTP headers are case-insensitive supposed to be according to RFC 2616.
static size_t	ug_plugin_curl_header_http (char *buffer, size_t size, size_t nmemb, UgPluginCurl *plugin)
{
	UgetCommon*	common;
	gchar*			file;
	gchar*			temp;

	common = plugin->common;
	file   = NULL;
#ifndef NDEBUG
	g_print ("%s", buffer);
#endif

	// handle HTTP header "Accept-Ranges:"
	if (g_ascii_strncasecmp (buffer, "Accept-Ranges: ", 15) == 0) {
		buffer += 15;
		if (strncmp (buffer, "none", 4) == 0) {
			plugin->resumable = FALSE;
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_info (UG_MESSAGE_INFO_NOT_RESUMABLE, NULL));
		}
		else {
			plugin->resumable = TRUE;
			ug_plugin_post ((UgPlugin*) plugin,
					ug_message_new_info (UG_MESSAGE_INFO_RESUMABLE, NULL));
		}
	}
	// handle HTTP header "Location:"
	else if (g_ascii_strncasecmp (buffer, "Location: ", 10) == 0) {
		// exclude header and character '\r', '\n'
		buffer += 10;
		g_free (common->url);
		common->url = g_strndup (buffer, strcspn (buffer, "\r\n"));
		ug_plugin_post ((UgPlugin*) plugin,
				ug_message_new_data (UG_MESSAGE_DATA_HTTP_LOCATION, common->url));
		// setup scheme and it's related data (user & password)
		ug_plugin_curl_set_scheme (plugin, plugin->curl);
		if (plugin->keep_filename == FALSE)
			file = ug_uri_get_filename (common->url);
	}
	// handle HTTP header "Content-Location:"
	else if (g_ascii_strncasecmp (buffer, "Content-Location: ", 18) == 0) {
		// exclude header and character '\r', '\n'
		buffer += 18;
		temp = g_strndup (buffer, strcspn (buffer, "\r\n"));
		ug_plugin_post ((UgPlugin*) plugin,
				ug_message_new_data (UG_MESSAGE_DATA_HTTP_CONTENT_LOCATION, temp));
		if (plugin->keep_filename == FALSE)
			file = ug_uri_get_filename (temp);
		g_free (temp);
	}
	// handle HTTP header "Content-Disposition:"
	else if (g_ascii_strncasecmp (buffer, "Content-Disposition: ", 21) == 0) {
		// exclude header and character '\r', '\n'
		buffer += 21;
		buffer = g_strndup (buffer, strcspn (buffer, "\r\n"));
		ug_plugin_post ((UgPlugin*) plugin,
				ug_message_new_data (UG_MESSAGE_DATA_HTTP_CONTENT_DISPOSITION, buffer));
		// grab filename
		if (plugin->keep_filename == FALSE) {
			file = strstr (buffer, "filename=");
			if (file) {
				file += 9;	// value of "filename="
				if (file[0] == '\"') {
					file += 1;
					temp = g_strndup (file, strcspn (file, "\""));
				}
				else
					temp = g_strndup (file, strcspn (file, ";"));
//				if (common->keeping.file == FALSE)
				file = ug_uri_get_filename (temp);
				g_free (temp);
			}
		}
		g_free (buffer);
	}

	if (file  &&  strcmp (file, common->file) != 0) {
		// change filename
		ug_plugin_post ((UgPlugin*) plugin,
				ug_message_new_data (UG_MESSAGE_DATA_FILE_CHANGED, file));
		ug_str_set (&common->folder, plugin->path, plugin->path_folder_len);
		ug_str_set (&common->file, file, -1);
		// delete old file
		fclose (plugin->file_stream);
		plugin->file_stream = NULL;
		ug_unlink (plugin->path_tmp);
		ug_unlink (plugin->path);
		// re-open file
		if (ug_plugin_curl_open_file (plugin, common) == FALSE)
			return 0;	// curl_easy_perform() will return CURLE_ABORTED_BY_CALLBACK
	}
	g_free (file);

	return nmemb;
}

// ----------------------------------------------------------------------------
// PWMD
//
#ifdef HAVE_LIBPWMD
static gboolean	ug_plugin_curl_set_proxy_pwmd (UgPluginCurl *plugin)
{
       UgMessage *message;
       struct pwmd_proxy_s pwmd;
       gpg_error_t rc;

       memset(&pwmd, 0, sizeof(pwmd));
       rc = ug_set_pwmd_proxy_options(&pwmd, plugin->proxy);

       if (rc)
               goto fail;

       // proxy host and port
       curl_easy_setopt (plugin->curl, CURLOPT_PROXY, pwmd.hostname);
       curl_easy_setopt (plugin->curl, CURLOPT_PROXYPORT, pwmd.port);

       // proxy user and password
       if (pwmd.username || pwmd.password || !strcasecmp(pwmd.type, "socks4") || !strcasecmp(pwmd.type, "socks5")) {
               curl_easy_setopt (plugin->curl, CURLOPT_PROXYUSERNAME,
                               (pwmd.username) ? pwmd.username : "");
               curl_easy_setopt (plugin->curl, CURLOPT_PROXYPASSWORD,
                               (pwmd.password) ? pwmd.password : "");
       }
       else {
               curl_easy_setopt (plugin->curl, CURLOPT_PROXYUSERNAME, NULL);
               curl_easy_setopt (plugin->curl, CURLOPT_PROXYPASSWORD, NULL);
       }

       if (!strcasecmp(pwmd.type, "socks4"))
               curl_easy_setopt (plugin->curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
       else if (!strcasecmp(pwmd.type, "socks5"))
               curl_easy_setopt (plugin->curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

       ug_close_pwmd(&pwmd);
       return TRUE;

fail:
       ug_close_pwmd(&pwmd);
       gchar *e = g_strdup_printf("Pwmd ERR %i: %s", rc, gpg_strerror(rc));
       message = ug_message_new_error (UG_MESSAGE_ERROR_CUSTOM, e);
       ug_plugin_post ((UgPlugin*) plugin, message);
       fprintf(stderr, "%s\n", e);
       g_free(e);
       return FALSE;
}

#endif	// HAVE_LIBPWMD

#endif	// HAVE_PLUGIN_CURL

