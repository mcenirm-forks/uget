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

#include <UgPlugin-aria2.h>

#ifdef HAVE_LIBPWMD
#include <stdlib.h>
#include <libpwmd.h>
#endif  // HAVE_LIBPWMD

#define	ARIA2_SOCKET_PORT		6800

// functions for UgPluginClass
static gboolean	ug_plugin_aria2_global_init		(void);
static void		ug_plugin_aria2_global_finalize	(void);

static gboolean	ug_plugin_aria2_init		(UgPluginAria2* plugin, UgDataset* dataset);
static void		ug_plugin_aria2_finalize	(UgPluginAria2* plugin);

static UgResult	ug_plugin_aria2_set_state	(UgPluginAria2* plugin, UgState  state);
static UgResult	ug_plugin_aria2_get_state	(UgPluginAria2* plugin, UgState* state);
static UgResult	ug_plugin_aria2_set			(UgPluginAria2* plugin, guint parameter, gpointer data);
static UgResult	ug_plugin_aria2_get			(UgPluginAria2* plugin, guint parameter, gpointer data);

// thread function
static gpointer	ug_plugin_aria2_thread		(UgPluginAria2* plugin);

// static data for UgPluginClass
static const char*	supported_schemes[]   = {"http", "https", "ftp", "ftps", NULL};
static const char*	supported_filetypes[] = {"torrent", "metalink", NULL};

static const	UgPluginClass	plugin_class_aria2 =
{
	"aria2",													// name
	NULL,														// reserve
	sizeof (UgPluginAria2),										// instance_size
	supported_schemes,											// schemes
	supported_filetypes,										// file_types

	(UgGlobalInitFunc)		ug_plugin_aria2_global_init,		// global_init
	(UgGlobalFinalizeFunc)	ug_plugin_aria2_global_finalize,	// global_finalize

	(UgPluginInitFunc)		ug_plugin_aria2_init,				// init
	(UgFinalizeFunc)		ug_plugin_aria2_finalize,			// finalize

	(UgSetStateFunc)		ug_plugin_aria2_set_state,			// set_state
	(UgGetStateFunc)		ug_plugin_aria2_get_state,			// get_state
	(UgSetFunc)				ug_plugin_aria2_set,				// set
	(UgGetFunc)				ug_plugin_aria2_get,				// get
};

// extern
const UgPluginClass*	UgPluginAria2Class = &plugin_class_aria2;

// ----------------------------------------------------------------------------
// functions for UgPluginClass
//
static gboolean	ug_plugin_aria2_global_init (void)
{
	return FALSE;
}

static void	ug_plugin_aria2_global_finalize (void)
{
}

static gboolean	ug_plugin_aria2_init (UgPluginAria2* plugin, UgDataset* dataset)
{
	UgDataCommon*	common;
	UgDataHttp*		http;

	common = ug_dataset_get (dataset, UgDataCommonClass, 0);
	http   = ug_dataset_get (dataset, UgDataHttpClass, 0);
	// check data
	if (common == NULL  ||  common->url == NULL)
		return FALSE;
	// reset
	if (common)
		common->retry_count = 0;
	if (http)
		http->redirection_count = 0;
	// copy supported data
	plugin->common = ug_data_list_copy (ug_dataset_get (dataset, UgDataCommonClass, 0));
	plugin->proxy  = ug_data_list_copy (ug_dataset_get (dataset, UgDataProxyClass, 0));
	plugin->http   = ug_data_list_copy (ug_dataset_get (dataset, UgDataHttpClass, 0));
	plugin->ftp    = ug_data_list_copy (ug_dataset_get (dataset, UgDataFtpClass, 0));

	return TRUE;
}

static void	ug_plugin_aria2_finalize (UgPluginAria2* plugin)
{
	// free data
	ug_data_list_free (plugin->common);
	ug_data_list_free (plugin->proxy);
	ug_data_list_free (plugin->http);
	ug_data_list_free (plugin->ftp);
}

static UgResult	ug_plugin_aria2_set_state (UgPluginAria2* plugin, UgState  state)
{
	UgState		old_state;

	old_state		= plugin->state;
	plugin->state	= state;
	// change plug-in status
	if (state != old_state) {
		if (state == UG_STATE_ACTIVE && old_state < UG_STATE_ACTIVE) {
			// call ug_plugin_unref () by ug_plugin_aria2_thread ()
			ug_plugin_ref ((UgPlugin*) plugin);
			g_thread_create ((GThreadFunc) ug_plugin_aria2_thread, plugin, FALSE, NULL);
		}

		ug_plugin_post ((UgPlugin*) plugin, ug_message_new_state (state));
	}

	return UG_RESULT_OK;
}

static UgResult	ug_plugin_aria2_get_state (UgPluginAria2* plugin, UgState* state)
{
	if (state) {
		*state = plugin->state;
		return UG_RESULT_OK;
	}

	return UG_RESULT_ERROR;
}

UgResult	ug_plugin_aria2_set (UgPluginAria2* plugin, guint parameter, gpointer data)
{
	return UG_RESULT_UNSUPPORT;
}

static UgResult	ug_plugin_aria2_get (UgPluginAria2* plugin, guint parameter, gpointer data)
{
	UgProgress*	progress;

	if (parameter != UG_DATA_TYPE_INSTANCE)
		return UG_RESULT_UNSUPPORT;
	if (data == NULL || ((UgData*)data)->data_class != UgProgressClass)
		return UG_RESULT_UNSUPPORT;

	progress = data;

	return UG_RESULT_OK;
}

// ----------------------------------------------------------------------------
// thread function
//
static gpointer	ug_plugin_aria2_thread (UgPluginAria2* plugin)
{
	return NULL;
}

