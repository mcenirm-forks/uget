/*
 *
 *   Copyright (C) 2005-2011 by plushuang
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <uglib.h>
#ifdef HAVE_PLUGIN_CURL
#include <UgPlugin-curl.h>
#endif
#ifdef HAVE_PLUGIN_ARIA2
#include <UgPlugin-aria2.h>
#endif

// ------------------------------------------------------------------
gboolean uglib_init (void)
{
	// data
	ug_data_interface_register (UG_DATA_COMMON_I);
	ug_data_interface_register (UG_DATA_PROXY_I);
	ug_data_interface_register (UG_PROGRESS_I);
	ug_data_interface_register (UG_DATA_HTTP_I);
	ug_data_interface_register (UG_DATA_FTP_I);
	ug_data_interface_register (UG_DATA_LOG_I);
	// category
	ug_data_interface_register (UG_CATEGORY_I);
	ug_data_interface_register (UG_RELATION_I);
	// message
	ug_data_interface_register (UG_MESSAGE_I);
	// plug-ins
#ifdef HAVE_PLUGIN_ARIA2
	// If plug-in failed to initialize, don't register it.
	if (ug_plugin_aria2_iface.global_init ())
		ug_plugin_interface_register (UG_PLUGIN_ARIA2_I);
#endif
#ifdef HAVE_PLUGIN_CURL
	if (ug_plugin_curl_iface.global_init ())
		ug_plugin_interface_register (UG_PLUGIN_CURL_I);
#endif

	return TRUE;
}

void uglib_finalize (void)
{
	// data
	ug_data_interface_unregister (UG_DATA_COMMON_I);
	ug_data_interface_unregister (UG_DATA_PROXY_I);
	ug_data_interface_unregister (UG_PROGRESS_I);
	ug_data_interface_unregister (UG_DATA_HTTP_I);
	ug_data_interface_unregister (UG_DATA_FTP_I);
	ug_data_interface_unregister (UG_DATA_LOG_I);
	// category
	ug_data_interface_unregister (UG_CATEGORY_I);
	ug_data_interface_unregister (UG_RELATION_I);
	// message
	ug_data_interface_unregister (UG_MESSAGE_I);
	// plug-ins
#ifdef HAVE_PLUGIN_ARIA2
	ug_plugin_interface_unregister (UG_PLUGIN_ARIA2_I);
	ug_plugin_aria2_iface.global_finalize ();
#endif
#ifdef HAVE_PLUGIN_CURL
	ug_plugin_interface_unregister (UG_PLUGIN_CURL_I);
	ug_plugin_curl_iface.global_finalize ();
#endif
}

