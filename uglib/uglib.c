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
	ug_data_interface_register (UgDataCommonIface);
	ug_data_interface_register (UgDataProxyIface);
	ug_data_interface_register (UgProgressIface);
	ug_data_interface_register (UgDataHttpIface);
	ug_data_interface_register (UgDataFtpIface);
	ug_data_interface_register (UgDataLogIface);
	// category
	ug_data_interface_register (UgCategoryIface);
	ug_data_interface_register (UgRelationIface);
	// message
	ug_data_interface_register (UgMessageIface);
	// plug-ins
#ifdef HAVE_PLUGIN_ARIA2
	// If plug-in failed to initialize, don't register it.
	if (UgPluginAria2Iface->global_init ())
		ug_plugin_interface_register (UgPluginAria2Iface);
#endif
#ifdef HAVE_PLUGIN_CURL
	if (UgPluginCurlIface->global_init ())
		ug_plugin_interface_register (UgPluginCurlIface);
#endif

	return TRUE;
}

void uglib_finalize (void)
{
	// data
	ug_data_interface_unregister (UgDataCommonIface);
	ug_data_interface_unregister (UgDataProxyIface);
	ug_data_interface_unregister (UgProgressIface);
	ug_data_interface_unregister (UgDataHttpIface);
	ug_data_interface_unregister (UgDataFtpIface);
	ug_data_interface_unregister (UgDataLogIface);
	// category
	ug_data_interface_unregister (UgCategoryIface);
	ug_data_interface_unregister (UgRelationIface);
	// message
	ug_data_interface_unregister (UgMessageIface);
	// plug-ins
#ifdef HAVE_PLUGIN_ARIA2
	ug_plugin_interface_unregister (UgPluginAria2Iface);
	UgPluginAria2Iface->global_finalize ();
#endif
#ifdef HAVE_PLUGIN_CURL
	ug_plugin_interface_unregister (UgPluginCurlIface);
	UgPluginCurlIface->global_finalize ();
#endif
}

