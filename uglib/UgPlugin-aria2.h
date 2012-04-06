/*
 *
 *   Copyright (C) 2005-2012 by C.H. Huang
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

#ifndef UG_PLUGIN_ARIA2_H
#define UG_PLUGIN_ARIA2_H

#include <UgPlugin.h>
#include <UgData-download.h>
#include <UgXmlrpc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef	struct	UgPluginAria2		UgPluginAria2;

extern	const	UgPluginInterface	ug_plugin_aria2_iface;


// ----------------------------------------------------------------------------
// uGet plug-in for aria2.

// UgPlugin
// |
// `--- UgPluginAria2

struct UgPluginAria2
{
	UG_PLUGIN_MEMBERS;
//	const UgPluginInterface*	iface;
//	unsigned int				ref_count;
//	UgMessage*					messages;
//	UgState						state;
//	GMutex*						lock;

	// supported data
	UgDataCommon*	common;
	UgDataProxy*	proxy;
	UgDataHttp*		http;
	UgDataFtp*		ftp;

	// temp
	GString*		string;
	GStringChunk*	chunk;
	// torrent file or metalink file
	gchar*			local_file;
	// aria2 version
	guint			major_version;
	guint			minor_version;

	// aria2 progress and status
	gchar*			gid;
	GList*			followed;	// followedBy
	gchar*			path;
	guint			errorCode;
	guint			aria2Status;
	gint64			totalLength;
	gint64			completedLength;
	gint64			uploadLength;
	gint			downloadSpeed;
	gint			uploadSpeed;
	gdouble			consumeTime;

	UgXmlrpc		xmlrpc;
};


#ifdef __cplusplus
}
#endif

#endif  // UG_PLUGIN_ARIA2_H

