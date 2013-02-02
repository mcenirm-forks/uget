/*
 *
 *   Copyright (C) 2005-2013 by C.H. Huang
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

// define UgDatalist-based structure for downloading.

// UgData
// |
// `- UgDatalist
//    |
//    +- UgetCommon
//    |
//    +- UgetProxy
//    |
//    +- UgetProgress
//    |
//    +- UgetHttp
//    |
//    +- UgetFtp
//    |
//    `- UgetLog

#ifndef UGET_DATA_H
#define UGET_DATA_H

#include <glib.h>
#include <UgData.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// interface address for UgDataset
#define	UgetCommonInfo      &uget_common_iface
#define	UgetProxyInfo       &uget_proxy_iface
#define	UgetProgressInfo    &uget_progress_iface
#define	UgetHttpInfo        &uget_http_iface
#define	UgetFtpInfo         &uget_ftp_iface
#define	UgetLogInfo         &uget_log_iface

typedef struct	UgetCommon      UgetCommon;
typedef struct	UgetProxy       UgetProxy;
typedef struct	UgetProgress    UgetProgress;
typedef struct	UgetHttp        UgetHttp;
typedef struct	UgetFtp         UgetFtp;
typedef struct	UgetLog         UgetLog;

typedef enum	UgetProxyType   UgetProxyType;

extern	const	UgDataInterface		uget_common_iface;
extern	const	UgDataInterface		uget_proxy_iface;
extern	const	UgDataInterface		uget_progress_iface;
extern	const	UgDataInterface		uget_http_iface;
extern	const	UgDataInterface		uget_ftp_iface;
extern	const	UgDataInterface		uget_log_iface;

// ----------------------------------------------------------------------------
// UgetCommon

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgetCommon

struct UgetCommon
{
	UG_DATALIST_MEMBERS (UgetCommon);
//	const UgDataInterface*	iface;
//	UgetCommon*			next;
//	UgetCommon*			prev;

	// common data
	gchar*		name;
	gchar*		url;
	gchar*		mirrors;
	gchar*		file;
	gchar*		folder;
	gchar*		user;
	gchar*		password;
	// Retrieve timestamp of the remote file if it is available.
	gboolean	retrieve_timestamp;
	// timeout
	guint		connect_timeout;	// second
	guint		transmit_timeout;	// second
	// retry
	guint		retry_delay;		// second
	guint		retry_limit;		// limit of retry_count
	guint		retry_count;		// count of UG_MESSAGE_INFO_RETRY

	guint		max_connections;	// max connections per server
	gint64		max_upload_speed;	// bytes per seconds
	gint64		max_download_speed;	// bytes per seconds

	guint		debug_level;

//	gint64		resume_offset;

	struct UgetCommonKeeping
	{
		gboolean	name:1;
		gboolean	url:1;
		gboolean	mirrors:1;
		gboolean	file:1;
		gboolean	folder:1;
		gboolean	user:1;
		gboolean	password:1;
		gboolean	timestamp:1;
		gboolean	connect_timeout:1;
		gboolean	transmit_timeout:1;
		gboolean	retry_delay:1;
		gboolean	retry_limit:1;

		gboolean	max_connections:1;
		gboolean	max_upload_speed:1;
		gboolean	max_download_speed:1;

		gboolean	debug_level:1;
	} keeping;
};


// ---------------------------------------------------------------------------
// UgetProxy

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgetProxy

enum UgetProxyType
{
	UG_DATA_PROXY_NONE,
	UG_DATA_PROXY_DEFAULT,			// Decided by plug-ins
	UG_DATA_PROXY_HTTP,
	UG_DATA_PROXY_SOCKS4,
	UG_DATA_PROXY_SOCKS5,
#ifdef HAVE_LIBPWMD
	UG_DATA_PROXY_PWMD,
#endif

	UG_DATA_PROXY_N_TYPE,
};

struct UgetProxy
{
	UG_DATALIST_MEMBERS (UgetProxy);
//	const UgDataInterface*	iface;
//	UgetProxy*			next;
//	UgetProxy*			prev;

	gchar*				host;
	guint				port;
	UgetProxyType		type;

	gchar*				user;
	gchar*				password;

	struct UgetProxyKeeping
	{
		gboolean	host:1;
		gboolean	port:1;
		gboolean	type:1;

		gboolean	user:1;
		gboolean	password:1;
	} keeping;

#ifdef HAVE_LIBPWMD
	struct UgetProxyPwmd
	{
		gchar*		socket;
		gchar*		file;
		gchar*		element;

		struct UgetProxyPwmdKeeping
		{
			gboolean	socket:1;
			gboolean	file:1;
			gboolean	element:1;
		} keeping;
	} pwmd;
#endif	// HAVE_LIBPWMD
};


// ---------------------------------------------------------------------------
// UgetProgress

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgetProgress

struct UgetProgress
{
	UG_DATALIST_MEMBERS (UgetProgress);
//	const UgDataInterface*	iface;
//	UgetProgress*				next;
//	UgetProgress*				prev;

	gint64		complete;
	gint64		total;
	gdouble		percent;
	gdouble		consume_time;		// Elapsed	(seconds)
	gdouble		remain_time;		// Left		(seconds)

	gint64		download_speed;
	gint64		upload_speed;		// torrent
	gint64		uploaded;			// torrent
	gdouble		ratio;				// torrent

//	guint		n_segments;			// 1 segment = 1 offset + 1 length
//	gint64*		segments;			// offset1, length1, offset2, length2
};


// ---------------------------------------------------------------------------
// UgetHttp

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgetHttp

struct UgetHttp
{
	UG_DATALIST_MEMBERS (UgetHttp);
//	const UgDataInterface*	iface;
//	UgetHttp*				next;
//	UgetHttp*				prev;

	gchar*		user;
	gchar*		password;
	gchar*		referrer;
	gchar*		user_agent;

	gchar*		post_data;
	gchar*		post_file;
	gchar*		cookie_data;
	gchar*		cookie_file;

	guint		redirection_limit;	// limit of redirection_count
	guint		redirection_count;	// count of UG_MESSAGE_DATA_HTTP_LOCATION

	struct UgetHttpKeeping
	{
		gboolean	user:1;
		gboolean	password:1;
		gboolean	referrer:1;
		gboolean	user_agent:1;

		gboolean	post_data:1;
		gboolean	post_file:1;
		gboolean	cookie_data:1;
		gboolean	cookie_file:1;
		gboolean	redirection_limit:1;
	} keeping;
};


// ---------------------------------------------------------------------------
// UgetFtp

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgetFtp

struct UgetFtp
{
	UG_DATALIST_MEMBERS (UgetFtp);
//	const UgDataInterface*	iface;
//	UgetFtp*				next;
//	UgetFtp*				prev;

	gchar*		user;
	gchar*		password;

	gboolean	active_mode;

	struct UgetFtpKeeping
	{
		gboolean	user:1;
		gboolean	password:1;
		gboolean	active_mode:1;
	} keeping;
};


// ---------------------------------------------------------------------------
// UgetLog

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgetLog

struct UgetLog
{
	UG_DATALIST_MEMBERS (UgetLog);
//	const UgDataInterface*	iface;
//	UgetLog*				next;
//	UgetLog*				prev;

	gchar*		added_on;		// "date time" string, e.g. "1990-01-01 23:00"
	gchar*		completed_on;	// "date time" string
};


#ifdef __cplusplus
}
#endif

#endif  // UGET_DATA_H

