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

#include <stdlib.h>
#include <string.h>
#include <memory.h>
// uglib
#include <UgString.h>
#include <UgRegistry.h>
#include <UgetData.h>


// ----------------------------------------------------------------------------
// UgetCommon

static void uget_common_init   (UgetCommon* common);
static void uget_common_final  (UgetCommon* common);
static void uget_common_assign (UgetCommon* common, UgetCommon* src);

static const UgDataEntry	uget_common_entry[] =
{
	{"name",				G_STRUCT_OFFSET (UgetCommon, name),				UG_TYPE_STRING,	NULL,	NULL},
	{"url",					G_STRUCT_OFFSET (UgetCommon, url),				UG_TYPE_STRING,	NULL,	NULL},
	{"mirrors",				G_STRUCT_OFFSET (UgetCommon, mirrors),			UG_TYPE_STRING,	NULL,	NULL},
	{"file",				G_STRUCT_OFFSET (UgetCommon, file),				UG_TYPE_STRING,	NULL,	NULL},
	{"folder",				G_STRUCT_OFFSET (UgetCommon, folder),			UG_TYPE_STRING,	NULL,	NULL},
	{"user",				G_STRUCT_OFFSET (UgetCommon, user),				UG_TYPE_STRING,	NULL,	NULL},
	{"password",			G_STRUCT_OFFSET (UgetCommon, password),			UG_TYPE_STRING,	NULL,	NULL},
	{"ConnectTimeout",		G_STRUCT_OFFSET (UgetCommon, connect_timeout),	UG_TYPE_UINT,	NULL,	NULL},
	{"TransmitTimeout",		G_STRUCT_OFFSET (UgetCommon, transmit_timeout),	UG_TYPE_UINT,	NULL,	NULL},
	{"RetryDelay",			G_STRUCT_OFFSET (UgetCommon, retry_delay),		UG_TYPE_UINT,	NULL,	NULL},
	{"RetryLimit",			G_STRUCT_OFFSET (UgetCommon, retry_limit),		UG_TYPE_UINT,	NULL,	NULL},
	{"MaxConnections",		G_STRUCT_OFFSET (UgetCommon, max_connections),	UG_TYPE_UINT,	NULL,	NULL},
	{"MaxUploadSpeed",		G_STRUCT_OFFSET (UgetCommon, max_upload_speed),	UG_TYPE_INT64,	NULL,	NULL},
	{"MaxDownloadSpeed",	G_STRUCT_OFFSET (UgetCommon, max_download_speed),	UG_TYPE_INT64,	NULL,	NULL},
	{"RetrieveTimestamp",	G_STRUCT_OFFSET (UgetCommon, retrieve_timestamp),	UG_TYPE_INT,	NULL,	NULL},
	{NULL}		// null-terminated
};
// extern
const UgDataInterface	uget_common_iface =
{
	sizeof (UgetCommon),	// instance_size
	"common",				// name
	uget_common_entry,	// entry

	(UgInitFunc)     uget_common_init,
	(UgFinalizeFunc) uget_common_final,
	(UgAssignFunc)   uget_common_assign,
};


static void uget_common_init (UgetCommon* common)
{
	common->retrieve_timestamp = TRUE;
	common->connect_timeout  = 30;
	common->transmit_timeout = 30;
	common->retry_delay = 6;
	common->retry_limit = 99;
	common->max_connections = 1;
}

static void uget_common_final (UgetCommon* common)
{
	g_free (common->name);
	g_free (common->url);
	g_free (common->mirrors);
	g_free (common->file);
	g_free (common->folder);
	g_free (common->user);
	g_free (common->password);
}

static void uget_common_assign (UgetCommon* common, UgetCommon* src)
{
	if (common->keeping.name == FALSE)
		ug_str_set (&common->name, src->name, -1);
	if (common->keeping.url == FALSE)
		ug_str_set (&common->url, src->url, -1);
	if (common->keeping.mirrors == FALSE)
		ug_str_set (&common->mirrors, src->mirrors, -1);
	if (common->keeping.file == FALSE)
		ug_str_set (&common->file, src->file, -1);
	if (common->keeping.folder == FALSE)
		ug_str_set (&common->folder, src->folder, -1);
	if (common->keeping.user == FALSE)
		ug_str_set (&common->user, src->user, -1);
	if (common->keeping.password == FALSE)
		ug_str_set (&common->password, src->password, -1);
	// timeout
	if (common->keeping.connect_timeout == FALSE)
		common->connect_timeout  = src->connect_timeout;
	if (common->keeping.transmit_timeout == FALSE)
		common->transmit_timeout = src->transmit_timeout;
	// retry
	if (common->keeping.retry_delay == FALSE)
		common->retry_delay = src->retry_delay;
	if (common->keeping.retry_limit == FALSE)
		common->retry_limit = src->retry_limit;
	// max connections
	if (common->keeping.max_connections == FALSE)
		common->max_connections = src->max_connections;
	// speed
	if (common->keeping.max_upload_speed == FALSE)
		common->max_upload_speed = src->max_upload_speed;
	if (common->keeping.max_download_speed == FALSE)
		common->max_download_speed = src->max_download_speed;
	// Retrieve timestamp
	if (common->keeping.retrieve_timestamp == FALSE)
		common->retrieve_timestamp = src->retrieve_timestamp;

	if (common->keeping.debug_level == FALSE)
		common->debug_level = src->debug_level;

	common->keeping = src->keeping;
}


// ----------------------------------------------------------------------------
// UgetProxy

static void	uget_proxy_final  (UgetProxy* proxy);
static void	uget_proxy_assign (UgetProxy* proxy, UgetProxy* src);

static const UgDataEntry	uget_proxy_entry[] =
{
	{"host",		G_STRUCT_OFFSET (UgetProxy, host),		UG_TYPE_STRING,	NULL,	NULL},
	{"port",		G_STRUCT_OFFSET (UgetProxy, port),		UG_TYPE_UINT,	NULL,	NULL},
	{"type",		G_STRUCT_OFFSET (UgetProxy, type),		UG_TYPE_UINT,	NULL,	NULL},
	{"user",		G_STRUCT_OFFSET (UgetProxy, user),		UG_TYPE_STRING,	NULL,	NULL},
	{"password",	G_STRUCT_OFFSET (UgetProxy, password),	UG_TYPE_STRING,	NULL,	NULL},
#ifdef HAVE_LIBPWMD
	{"pwmd-socket",	G_STRUCT_OFFSET (UgetProxy, pwmd.socket),	UG_TYPE_STRING,	NULL,	NULL},
	{"pwmd-socket-args",	G_STRUCT_OFFSET (UgetProxy, pwmd.socket_args),	UG_TYPE_STRING,	NULL,	NULL},
	{"pwmd-file",	G_STRUCT_OFFSET (UgetProxy, pwmd.file),	UG_TYPE_STRING,	NULL,	NULL},
	{"pwmd-element",G_STRUCT_OFFSET (UgetProxy, pwmd.element),UG_TYPE_STRING,	NULL,	NULL},
#endif
	{NULL},		// null-terminated
};
// extern
const UgDataInterface	uget_proxy_iface =
{
	sizeof (UgetProxy),	// instance_size
	"proxy",			// name
	uget_proxy_entry,	// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	uget_proxy_final,
	(UgAssignFunc)		uget_proxy_assign,
};


static void	uget_proxy_final	(UgetProxy* proxy)
{
	g_free (proxy->host);
	g_free (proxy->user);
	g_free (proxy->password);

#ifdef HAVE_LIBPWMD
	g_free(proxy->pwmd.socket);
	g_free(proxy->pwmd.socket_args);
	g_free(proxy->pwmd.file);
	g_free(proxy->pwmd.element);
#endif	// HAVE_LIBPWMD
}

static void	uget_proxy_assign (UgetProxy* proxy, UgetProxy* src)
{
	if (proxy->keeping.host == FALSE)
		ug_str_set (&proxy->host, src->host, -1);
	if (proxy->keeping.port == FALSE)
		proxy->port = src->port;
	if (proxy->keeping.type == FALSE)
		proxy->type = src->type;

	if (proxy->keeping.user == FALSE)
		ug_str_set (&proxy->user, src->user, -1);
	if (proxy->keeping.password == FALSE)
		ug_str_set (&proxy->password, src->password, -1);

#ifdef HAVE_LIBPWMD
	if (proxy->pwmd.keeping.socket == FALSE)
		ug_str_set (&proxy->pwmd.socket, src->pwmd.socket, -1);
	if (proxy->pwmd.keeping.socket_args == FALSE)
		ug_str_set (&proxy->pwmd.socket_args, src->pwmd.socket_args, -1);
	if (proxy->pwmd.keeping.file == FALSE)
		ug_str_set (&proxy->pwmd.file, src->pwmd.file, -1);
	if (proxy->pwmd.keeping.element == FALSE)
		ug_str_set (&proxy->pwmd.element, src->pwmd.element, -1);
#endif	// HAVE_LIBPWMD

	proxy->keeping = src->keeping;
}


// ----------------------------------------------------------------------------
// UgetProgress

static void uget_progress_assign (UgetProgress* progress, UgetProgress* src);

static const UgDataEntry	uget_progress_entry[] =
{
	{"complete",	G_STRUCT_OFFSET (UgetProgress, complete),		UG_TYPE_INT64,	NULL,	NULL},
	{"total",		G_STRUCT_OFFSET (UgetProgress, total),		UG_TYPE_INT64,	NULL,	NULL},
	{"percent",		G_STRUCT_OFFSET (UgetProgress, percent),		UG_TYPE_DOUBLE,	NULL,	NULL},
	{"elapsed",		G_STRUCT_OFFSET (UgetProgress, consume_time),	UG_TYPE_DOUBLE,	NULL,	NULL},
	{"uploaded",	G_STRUCT_OFFSET (UgetProgress, uploaded),		UG_TYPE_INT64,	NULL,	NULL},
	{"ratio",		G_STRUCT_OFFSET (UgetProgress, ratio),		UG_TYPE_DOUBLE,	NULL,	NULL},
	{NULL},		// null-terminated
};
// extern
const UgDataInterface	uget_progress_iface =
{
	sizeof (UgetProgress),	// instance_size
	"progress",				// name
	uget_progress_entry,		// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	NULL,
	(UgAssignFunc)		uget_progress_assign,
};


static void uget_progress_assign (UgetProgress* progress, UgetProgress* src)
{
	// copy without private member
	memcpy ( ((guint8*) progress) + sizeof (UgDatalist),
	         ((guint8*) src)      + sizeof (UgDatalist),
	         sizeof (UgetProgress)  - sizeof (UgDatalist) );
}


// ---------------------------------------------------------------------------
// UgetHttp

static void	uget_http_init   (UgetHttp* http);
static void	uget_http_final  (UgetHttp* http);
static void	uget_http_assign (UgetHttp* http, UgetHttp* src);

static const UgDataEntry	uget_http_entry[] =
{
	{"user",				G_STRUCT_OFFSET (UgetHttp, user),				UG_TYPE_STRING,	NULL,	NULL},
	{"password",			G_STRUCT_OFFSET (UgetHttp, password),			UG_TYPE_STRING,	NULL,	NULL},
	{"referrer",			G_STRUCT_OFFSET (UgetHttp, referrer),			UG_TYPE_STRING,	NULL,	NULL},
	{"UserAgent",			G_STRUCT_OFFSET (UgetHttp, user_agent),			UG_TYPE_STRING,	NULL,	NULL},
	{"PostData",			G_STRUCT_OFFSET (UgetHttp, post_data),			UG_TYPE_STRING,	NULL,	NULL},
	{"PostFile",			G_STRUCT_OFFSET (UgetHttp, post_file),			UG_TYPE_STRING,	NULL,	NULL},
	{"CookieData",			G_STRUCT_OFFSET (UgetHttp, cookie_data),		UG_TYPE_STRING,	NULL,	NULL},
	{"CookieFile",			G_STRUCT_OFFSET (UgetHttp, cookie_file),		UG_TYPE_STRING,	NULL,	NULL},
	{"RedirectionLimit",	G_STRUCT_OFFSET (UgetHttp, redirection_limit),	UG_TYPE_UINT,	NULL,	NULL},
	{"RedirectionCount",	G_STRUCT_OFFSET (UgetHttp, redirection_count),	UG_TYPE_UINT,	NULL,	NULL},
	{NULL},		// null-terminated
};
// extern
const UgDataInterface	uget_http_iface =
{
	sizeof (UgetHttp),	// instance_size
	"http",					// name
	uget_http_entry,		// entry

	(UgInitFunc)		uget_http_init,
	(UgFinalizeFunc)	uget_http_final,
	(UgAssignFunc)		uget_http_assign,
};


static void	uget_http_init (UgetHttp* http)
{
	http->redirection_limit = 30;
}

static void	uget_http_final (UgetHttp* http)
{
	g_free (http->user);
	g_free (http->password);
	g_free (http->referrer);
	g_free (http->user_agent);
	g_free (http->post_data);
	g_free (http->post_file);
	g_free (http->cookie_data);
	g_free (http->cookie_file);
}

static void	uget_http_assign (UgetHttp* http, UgetHttp* src)
{
	if (http->keeping.user == FALSE)
		ug_str_set (&http->user, src->user, -1);
	if (http->keeping.password == FALSE)
		ug_str_set (&http->password, src->password, -1);
	if (http->keeping.referrer == FALSE)
		ug_str_set (&http->referrer, src->referrer, -1);
	if (http->keeping.user_agent == FALSE)
		ug_str_set (&http->user_agent, src->user_agent, -1);

	if (http->keeping.post_data == FALSE)
		ug_str_set (&http->post_data, src->post_data, -1);
	if (http->keeping.post_file == FALSE)
		ug_str_set (&http->post_file, src->post_file, -1);
	if (http->keeping.cookie_data == FALSE)
		ug_str_set (&http->cookie_data, src->cookie_data, -1);
	if (http->keeping.cookie_file == FALSE)
		ug_str_set (&http->cookie_file, src->cookie_file, -1);

	if (http->keeping.redirection_limit == FALSE)
		http->redirection_limit = src->redirection_limit;

	http->keeping = src->keeping;
}


// ---------------------------------------------------------------------------
// UgetFtp
//
static void	uget_ftp_final  (UgetFtp* ftp);
static void	uget_ftp_assign (UgetFtp* ftp, UgetFtp* src);

static const UgDataEntry  uget_ftp_entry[] =
{
	{"user",		G_STRUCT_OFFSET (UgetFtp, user),		UG_TYPE_STRING,	NULL,	NULL},
	{"password",	G_STRUCT_OFFSET (UgetFtp, password),	UG_TYPE_STRING,	NULL,	NULL},
	{"ActiveMode",	G_STRUCT_OFFSET (UgetFtp, active_mode),	UG_TYPE_INT,NULL,	NULL},
	{NULL},		// null-terminated
};
// extern
const UgDataInterface  uget_ftp_iface =
{
	sizeof (UgetFtp),	// instance_size
	"ftp",				// name
	uget_ftp_entry,	// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	uget_ftp_final,
	(UgAssignFunc)		uget_ftp_assign,
};


static void	uget_ftp_final	(UgetFtp* ftp)
{
	g_free (ftp->user);
	g_free (ftp->password);
}

static void	uget_ftp_assign	(UgetFtp* ftp, UgetFtp* src)
{
	if (ftp->keeping.user == FALSE)
		ug_str_set (&ftp->user, src->user, -1);
	if (ftp->keeping.password == FALSE)
		ug_str_set (&ftp->password, src->password, -1);

	if (ftp->keeping.active_mode == FALSE)
		ftp->active_mode = src->active_mode;

	ftp->keeping = src->keeping;
}


// ---------------------------------------------------------------------------
// UgetLog
//
static void	uget_log_final (UgetLog* log);

static const UgDataEntry  uget_log_entry[] =
{
	{"AddedOn",		G_STRUCT_OFFSET (UgetLog, added_on),		UG_TYPE_STRING,	NULL,	NULL},
	{"CompletedOn",	G_STRUCT_OFFSET (UgetLog, completed_on),	UG_TYPE_STRING,	NULL,	NULL},
	{NULL},		// null-terminated
};
// extern
const UgDataInterface  uget_log_iface =
{
	sizeof (UgetLog),	// instance_size
	"log",				// name
	uget_log_entry,	// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	uget_log_final,
	(UgAssignFunc)		NULL,
};


static void	uget_log_final (UgetLog* log)
{
	g_free (log->added_on);
	g_free (log->completed_on);
}

