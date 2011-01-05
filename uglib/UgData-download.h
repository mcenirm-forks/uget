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


// define UgDataList-based structure for downloading.
//
// UgData
// |
// +- UgDataList
//    |
//    +- UgDataCommon
//    |
//    +- UgDataProxy
//    |
//    +- UgProgress
//    |
//    +- UgDataHttp
//    |
//    +- UgDataFtp
//


#ifndef UG_DATA_DOWNLOAD_H
#define UG_DATA_DOWNLOAD_H

#include <glib.h>
#include <UgData.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgDataCommon_		UgDataCommon;
typedef struct	UgDataProxy_		UgDataProxy;
typedef struct	UgProgress_			UgProgress;
typedef struct	UgDataHttp_			UgDataHttp;
typedef struct	UgDataFtp_			UgDataFtp;
typedef struct	UgDataLog_			UgDataLog;

typedef enum	UgDataProxyType_	UgDataProxyType;

extern	const	UgDataClass*		UgDataCommonClass;
extern	const	UgDataClass*		UgDataProxyClass;
extern	const	UgDataClass*		UgProgressClass;
extern	const	UgDataClass*		UgDataHttpClass;
extern	const	UgDataClass*		UgDataFtpClass;
extern	const	UgDataClass*		UgDataLogClass;

// ----------------------------------------------------------------------------
// UgDataCommon
//
//  UgData
//  |
//  `- UgDataList
//     |
//     `- UgDataCommon
//
struct UgDataCommon_
{
	UG_DATA_LIST_MEMBERS (UgDataCommon);
//	const UgDataClass*	data_class;
//	UgDataCommon*		next;
//	UgDataCommon*		prev;

	// common data
	gchar*		name;
	gchar*		url;
	gchar*		file;
	gchar*		folder;
	gchar*		user;
	gchar*		password;
	// timeout
	guint		connect_timeout;	// second
	guint		transmit_timeout;	// second
	// retry
	guint		retry_delay;		// second
	guint		retry_limit;		// limit of retry_count
	guint		retry_count;		// count of UG_MESSAGE_INFO_RETRY

	gint64		max_upload_speed;	// bytes per seconds
	gint64		max_download_speed;	// bytes per seconds
	guint		segments_per_download;	// number of segments per download

	guint		debug_level;

//	gint64		resume_offset;

	struct UgDataCommonKeeping
	{
		gboolean	name:1;
		gboolean	url:1;
		gboolean	file:1;
		gboolean	folder:1;
		gboolean	user:1;
		gboolean	password:1;
		gboolean	connect_timeout:1;
		gboolean	transmit_timeout:1;
		gboolean	retry_delay:1;
		gboolean	retry_limit:1;

		gboolean	max_upload_speed:1;
		gboolean	max_download_speed:1;
		gboolean	segments_per_download:1;

		gboolean	debug_level:1;
	} keeping;

	// deprecate
	struct
	{
		gchar*	folder;
		guint	stamp;
	} attached;
};


// ---------------------------------------------------------------------------
// UgDataProxy
//
//  UgData
//  |
//  `- UgDataList
//     |
//     `- UgDataProxy
//
enum UgDataProxyType_
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

struct UgDataProxy_
{
	UG_DATA_LIST_MEMBERS (UgDataProxy);
//	const UgDataClass*	data_class;
//	UgDataProxy*		next;
//	UgDataProxy*		prev;

	gchar*				host;
	guint				port;
	UgDataProxyType		type;

	gchar*				user;
	gchar*				password;

	struct UgDataProxyKeeping
	{
		gboolean	host:1;
		gboolean	port:1;
		gboolean	type:1;

		gboolean	user:1;
		gboolean	password:1;
	} keeping;

#ifdef HAVE_LIBPWMD
	struct UgDataProxyPwmd
	{
		gchar*		socket;
		gchar*		file;
		gchar*		element;

		struct UgDataProxyPwmdKeeping
		{
			gboolean	socket:1;
			gboolean	file:1;
			gboolean	element:1;
		} keeping;
	} pwmd;
#endif	// HAVE_LIBPWMD
};


// ---------------------------------------------------------------------------
// UgProgress
//
//  UgData
//  |
//  `- UgDataList
//     |
//     `- UgProgress
//
struct UgProgress_
{
	UG_DATA_LIST_MEMBERS (UgProgress);
//	const UgDataClass*	data_class;
//	UgDataProgress*		next;
//	UgDataProgress*		prev;

	gint64		complete;
	gint64		total;
	gdouble		percent;
	gdouble		upload_speed;
	gdouble		download_speed;
	gdouble		consume_time;		// Elapsed	(seconds)
	gdouble		remain_time;		// Left		(seconds)

//	guint		n_segments;			// 1 segment = 1 offset + 1 length
//	gint64*		segments;			// offset1, length1, offset2, length2
};


// ---------------------------------------------------------------------------
// UgDataHttp
//
//  UgData
//  |
//  `- UgDataList
//     |
//     `- UgDataHttp
//
struct UgDataHttp_
{
	UG_DATA_LIST_MEMBERS (UgDataHttp);
//	const UgDataClass*	data_class;
//	UgDataHttp*			next;
//	UgDataHttp*			prev;

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

	struct UgDataHttpKeeping
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
// UgDataFtp
//
//  UgData
//  |
//  `- UgDataList
//     |
//     `- UgDataFtp
//
struct UgDataFtp_
{
	UG_DATA_LIST_MEMBERS (UgDataFtp);
//	const UgDataClass*	data_class;
//	UgDataFtp*			next;
//	UgDataFtp*			prev;

	gchar*		user;
	gchar*		password;

	gboolean	active_mode;

	struct UgDataFtpKeeping
	{
		gboolean	user:1;
		gboolean	password:1;
		gboolean	active_mode:1;
	} keeping;
};


// ---------------------------------------------------------------------------
// UgDataLog
//
//  UgData
//  |
//  `- UgDataList
//     |
//     `- UgDataLog
//
struct UgDataLog_
{
	UG_DATA_LIST_MEMBERS (UgDataLog);
//	const UgDataClass*	data_class;
//	UgDataLog*			next;
//	UgDataLog*			prev;

	gchar*		added_on;		// "date time" string, e.g. "1990-01-01 23:00"
	gchar*		completed_on;	// "date time" string
};


#ifdef __cplusplus
}
#endif

#endif  // UG_DATA_DOWNLOAD_H

