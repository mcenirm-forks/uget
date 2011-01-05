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

#include <stdlib.h>
#include <string.h>
#include <memory.h>
// uglib
#include <UgString.h>
#include <UgRegistry.h>
#include <UgData-download.h>


// ----------------------------------------------------------------------------
// UgDataCommon
//
static void ug_data_common_init		(UgDataCommon* common);
static void ug_data_common_finalize	(UgDataCommon* common);
static void ug_data_common_assign	(UgDataCommon* common, UgDataCommon* src);

static UgDataEntry	common_data_entry[] =
{
	{"name",				G_STRUCT_OFFSET (UgDataCommon, name),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"url",					G_STRUCT_OFFSET (UgDataCommon, url),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"file",				G_STRUCT_OFFSET (UgDataCommon, file),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"folder",				G_STRUCT_OFFSET (UgDataCommon, folder),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"user",				G_STRUCT_OFFSET (UgDataCommon, user),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"password",			G_STRUCT_OFFSET (UgDataCommon, password),		UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"ConnectTimeout",		G_STRUCT_OFFSET (UgDataCommon, connect_timeout),	UG_DATA_TYPE_UINT,	NULL,	NULL},
	{"TransmitTimeout",		G_STRUCT_OFFSET (UgDataCommon, transmit_timeout),	UG_DATA_TYPE_UINT,	NULL,	NULL},
	{"RetryDelay",			G_STRUCT_OFFSET (UgDataCommon, retry_delay),		UG_DATA_TYPE_UINT,	NULL,	NULL},
	{"RetryLimit",			G_STRUCT_OFFSET (UgDataCommon, retry_limit),		UG_DATA_TYPE_UINT,	NULL,	NULL},
	{"MaxUploadSpeed",		G_STRUCT_OFFSET (UgDataCommon, max_upload_speed),	UG_DATA_TYPE_INT64,	NULL,	NULL},
	{"MaxDownloadSpeed",	G_STRUCT_OFFSET (UgDataCommon, max_download_speed),	UG_DATA_TYPE_INT64,	NULL,	NULL},
	{"SegmentsPerDownload",	G_STRUCT_OFFSET (UgDataCommon, segments_per_download),UG_DATA_TYPE_INT,	NULL,	NULL},

	// deprecate
	{"AttachedFolder",		G_STRUCT_OFFSET (UgDataCommon, attached.folder),	UG_DATA_TYPE_STRING,NULL,	NULL},
	{"AttachedStamp",		G_STRUCT_OFFSET (UgDataCommon, attached.stamp),		UG_DATA_TYPE_UINT,	NULL,	NULL},
	{NULL}		// null-terminated
};

static UgDataClass	common_data_class =
{
	"common",				// name
	NULL,					// reserve
	sizeof (UgDataCommon),	// instance_size
	common_data_entry,		// entry

	(UgInitFunc)     ug_data_common_init,
	(UgFinalizeFunc) ug_data_common_finalize,
	(UgAssignFunc)   ug_data_common_assign,
};
// extern
const	UgDataClass*		UgDataCommonClass = &common_data_class;


static void ug_data_common_init (UgDataCommon* common)
{
	common->connect_timeout  = 30;
	common->transmit_timeout = 30;
	common->retry_delay = 6;
	common->retry_limit = 99;
}

static void ug_data_common_finalize (UgDataCommon* common)
{
	g_free (common->name);
	g_free (common->url);
	g_free (common->file);
	g_free (common->folder);
	g_free (common->user);
	g_free (common->password);
}

static void ug_data_common_assign (UgDataCommon* common, UgDataCommon* src)
{
	if (common->keeping.name == FALSE)
		ug_str_set (&common->name, src->name, -1);
	if (common->keeping.url == FALSE)
		ug_str_set (&common->url, src->url, -1);
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
	// speed
	if (common->keeping.max_upload_speed == FALSE)
		common->max_upload_speed = src->max_upload_speed;
	if (common->keeping.max_download_speed == FALSE)
		common->max_download_speed = src->max_download_speed;
	// segments
	if (common->keeping.segments_per_download == FALSE)
		common->segments_per_download = src->segments_per_download;

	if (common->keeping.debug_level == FALSE)
		common->debug_level = src->debug_level;

	common->keeping = src->keeping;
}


// ----------------------------------------------------------------------------
// UgDataProxy
//
static void	ug_data_proxy_finalize	(UgDataProxy* proxy);
static void	ug_data_proxy_assign	(UgDataProxy* proxy, UgDataProxy* src);

static UgDataEntry	proxy_data_entry[] =
{
	{"host",		G_STRUCT_OFFSET (UgDataProxy, host),		UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"port",		G_STRUCT_OFFSET (UgDataProxy, port),		UG_DATA_TYPE_UINT,		NULL,	NULL},
	{"type",		G_STRUCT_OFFSET (UgDataProxy, type),		UG_DATA_TYPE_UINT,		NULL,	NULL},
	{"user",		G_STRUCT_OFFSET (UgDataProxy, user),		UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"password",	G_STRUCT_OFFSET (UgDataProxy, password),	UG_DATA_TYPE_STRING,	NULL,	NULL},
#ifdef HAVE_LIBPWMD
	{"pwmd.socket",		G_STRUCT_OFFSET (UgDataProxy, pwmd.socket),	UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"pwmd.file",		G_STRUCT_OFFSET (UgDataProxy, pwmd.file),	UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"pwmd.element",	G_STRUCT_OFFSET (UgDataProxy, pwmd.element),UG_DATA_TYPE_STRING,	NULL,	NULL},
#endif
	{NULL},		// null-terminated
};

static UgDataClass	proxy_data_class =
{
	"proxy",				// name
	NULL,					// reserve
	sizeof (UgDataProxy),	// instance_size
	proxy_data_entry,		// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	ug_data_proxy_finalize,
	(UgAssignFunc)		ug_data_proxy_assign,
};
// extern
const	UgDataClass*	UgDataProxyClass = &proxy_data_class;


static void	ug_data_proxy_finalize	(UgDataProxy* proxy)
{
	g_free (proxy->host);
	g_free (proxy->user);
	g_free (proxy->password);

#ifdef HAVE_LIBPWMD
	g_free(proxy->pwmd.socket);
	g_free(proxy->pwmd.file);
	g_free(proxy->pwmd.element);
#endif	// HAVE_LIBPWMD
}

static void	ug_data_proxy_assign (UgDataProxy* proxy, UgDataProxy* src)
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
	if (proxy->pwmd.keeping.file == FALSE)
		ug_str_set (&proxy->pwmd.file, src->pwmd.file, -1);
	if (proxy->pwmd.keeping.element == FALSE)
		ug_str_set (&proxy->pwmd.element, src->pwmd.element, -1);
#endif	// HAVE_LIBPWMD

	proxy->keeping = src->keeping;
}


// ----------------------------------------------------------------------------
// UgProgress
//
static void ug_progress_assign		(UgProgress* progress, UgProgress* src);

static UgDataEntry	progress_data_entry[] =
{
	{"complete",	G_STRUCT_OFFSET (UgProgress, complete),		UG_DATA_TYPE_INT64,		NULL,	NULL},
	{"total",		G_STRUCT_OFFSET (UgProgress, total),		UG_DATA_TYPE_INT64,		NULL,	NULL},
	{"percent",		G_STRUCT_OFFSET (UgProgress, percent),		UG_DATA_TYPE_DOUBLE,	NULL,	NULL},
	{"elapsed",		G_STRUCT_OFFSET (UgProgress, consume_time),	UG_DATA_TYPE_DOUBLE,	NULL,	NULL},
	{NULL},		// null-terminated
};

static UgDataClass	progress_data_class =
{
	"progress",				// name
	NULL,					// reserve
	sizeof (UgProgress),	// instance_size
	progress_data_entry,	// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	NULL,
	(UgAssignFunc)		ug_progress_assign,
};
// extern
const UgDataClass*	UgProgressClass = &progress_data_class;

static void ug_progress_assign (UgProgress* progress, UgProgress* src)
{
	// copy without private member
	memcpy ( ((guint8*) progress) + sizeof (UgDataList),
	         ((guint8*) src)      + sizeof (UgDataList),
	         sizeof (UgProgress)  - sizeof (UgDataList) );
}


// ---------------------------------------------------------------------------
// UgDataHttp
//
static void	ug_data_http_init		(UgDataHttp* http);
static void	ug_data_http_finalize	(UgDataHttp* http);
static void	ug_data_http_assign		(UgDataHttp* http, UgDataHttp* src);

static UgDataEntry	http_data_entry[] =
{
	{"user",				G_STRUCT_OFFSET (UgDataHttp, user),					UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"password",			G_STRUCT_OFFSET (UgDataHttp, password),				UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"referrer",			G_STRUCT_OFFSET (UgDataHttp, referrer),				UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"UserAgent",			G_STRUCT_OFFSET (UgDataHttp, user_agent),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"PostData",			G_STRUCT_OFFSET (UgDataHttp, post_data),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"PostFile",			G_STRUCT_OFFSET (UgDataHttp, post_file),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"CookieData",			G_STRUCT_OFFSET (UgDataHttp, cookie_data),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"CookieFile",			G_STRUCT_OFFSET (UgDataHttp, cookie_file),			UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"RedirectionLimit",	G_STRUCT_OFFSET (UgDataHttp, redirection_limit),	UG_DATA_TYPE_UINT,		NULL,	NULL},
	{"RedirectionCount",	G_STRUCT_OFFSET (UgDataHttp, redirection_count),	UG_DATA_TYPE_UINT,		NULL,	NULL},
	{NULL},		// null-terminated
};

static UgDataClass	http_data_class =
{
	"http",					// name
	NULL,					// reserve
	sizeof (UgDataHttp),	// instance_size
	http_data_entry,		// entry

	(UgInitFunc)		ug_data_http_init,
	(UgFinalizeFunc)	ug_data_http_finalize,
	(UgAssignFunc)		ug_data_http_assign,
};
// extern
const	UgDataClass*	UgDataHttpClass = &http_data_class;

static void	ug_data_http_init (UgDataHttp* http)
{
	http->redirection_limit = 30;
}

static void	ug_data_http_finalize (UgDataHttp* http)
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

static void	ug_data_http_assign (UgDataHttp* http, UgDataHttp* src)
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
// UgDataFtp
//
static void	ug_data_ftp_finalize	(UgDataFtp* ftp);
static void	ug_data_ftp_assign		(UgDataFtp* ftp, UgDataFtp* src);

static UgDataEntry	ftp_data_entry[] =
{
	{"user",		G_STRUCT_OFFSET (UgDataFtp, user),		UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"password",	G_STRUCT_OFFSET (UgDataFtp, password),	UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"ActiveMode",	G_STRUCT_OFFSET (UgDataFtp, active_mode),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{NULL},		// null-terminated
};

static UgDataClass	ftp_data_class =
{
	"ftp",				// name
	NULL,				// reserve
	sizeof (UgDataFtp),	// instance_size
	ftp_data_entry,		// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	ug_data_ftp_finalize,
	(UgAssignFunc)		ug_data_ftp_assign,
};
// extern
const	UgDataClass*	UgDataFtpClass = &ftp_data_class;


static void	ug_data_ftp_finalize	(UgDataFtp* ftp)
{
	g_free (ftp->user);
	g_free (ftp->password);
}

static void	ug_data_ftp_assign	(UgDataFtp* ftp, UgDataFtp* src)
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
// UgDataLog
//
static void	ug_data_log_finalize	(UgDataLog* log);

static UgDataEntry	log_data_entry[] =
{
	{"AddedOn",		G_STRUCT_OFFSET (UgDataLog, added_on),		UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"CompletedOn",	G_STRUCT_OFFSET (UgDataLog, completed_on),	UG_DATA_TYPE_STRING,	NULL,	NULL},
	{NULL},		// null-terminated
};

static UgDataClass	log_data_class =
{
	"log",				// name
	NULL,				// reserve
	sizeof (UgDataLog),	// instance_size
	log_data_entry,		// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	ug_data_log_finalize,
	(UgAssignFunc)		NULL,
};
// extern
const	UgDataClass*	UgDataLogClass = &log_data_class;


static void	ug_data_log_finalize	(UgDataLog* log)
{
	g_free (log->added_on);
	g_free (log->completed_on);
}

