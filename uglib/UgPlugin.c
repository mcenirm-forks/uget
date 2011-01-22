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

#include <string.h>

#include <UgPlugin.h>
#include <UgData-download.h>
#include <UgUtils.h>
#include <UgStdio.h>

#if defined(_WIN32) || defined(__MINGW32__)
#define HIDDEN_FILE_CHAR	'_'
#define HIDDEN_FILE_CHAR_S	"_"
#else
#define HIDDEN_FILE_CHAR	'.'
#define HIDDEN_FILE_CHAR_S	"."
#endif

// ---------------------------------------------------------------------------
// UgPluginClass
//
gboolean	ug_plugin_class_register (const UgPluginClass* plugin_class)
{
	const gchar**	string;
	GList*			list;

	// If plug-in failed to initialize, don't register it.
	if (plugin_class->global_init () == FALSE)
		return FALSE;

	ug_registry_insert (plugin_class->name, UG_REG_PLUGIN_CLASS, (gpointer) plugin_class);

	if (plugin_class->schemes) {
		for (string = plugin_class->schemes;  *string;  string++) {
			list = ug_registry_search (*string, UG_REG_PLUGIN_SCHEME);
			list = g_list_prepend (list, (gpointer) plugin_class);
			ug_registry_insert (*string, UG_REG_PLUGIN_SCHEME, list);
		}
	}

	if (plugin_class->file_types) {
		for (string = plugin_class->file_types;  *string;  string++) {
			list = ug_registry_search (*string, UG_REG_PLUGIN_FILE_TYPE);
			list = g_list_prepend (list, (gpointer)plugin_class);
			ug_registry_insert (*string, UG_REG_PLUGIN_FILE_TYPE, list);
		}
	}

	return TRUE;
}

void	ug_plugin_class_unregister (const UgPluginClass* plugin_class)
{
	const gchar**	string;
	GList*			list;

	// Don't unregister plug-in if it not found.
	if (ug_registry_search (plugin_class->name, UG_REG_PLUGIN_CLASS) == NULL)
		return;

	ug_registry_remove (plugin_class->name, UG_REG_PLUGIN_CLASS);

	if (plugin_class->schemes) {
		for (string = plugin_class->schemes;  *string;  string++) {
			list = ug_registry_search (*string, UG_REG_PLUGIN_SCHEME);
			list = g_list_remove (list, (gpointer) plugin_class);
			if (list)
				ug_registry_insert (*string, UG_REG_PLUGIN_SCHEME, list);
			else
				ug_registry_remove (*string, UG_REG_PLUGIN_SCHEME);
		}
	}

	if (plugin_class->file_types) {
		for (string = plugin_class->file_types;  *string;  string++) {
			list = ug_registry_search (*string, UG_REG_PLUGIN_FILE_TYPE);
			list = g_list_remove (list, (gpointer) plugin_class);
			if (list)
				ug_registry_insert (*string, UG_REG_PLUGIN_FILE_TYPE, list);
			else
				ug_registry_remove (*string, UG_REG_PLUGIN_FILE_TYPE);
		}
	}

	plugin_class->global_finalize ();
}

const UgPluginClass*	ug_plugin_class_find (const gchar* name, enum UgRegistryType type)
{
	gpointer	data;

	if (type < UG_REG_PLUGIN_CLASS || type > UG_REG_PLUGIN_TYPE_LAST)
		type = UG_REG_PLUGIN_CLASS;

	data = ug_registry_search (name, type);
	if (type == UG_REG_PLUGIN_CLASS)
		return (const UgPluginClass*) data;
	else if (data)
		return (const UgPluginClass*) ((GList*)data)->data;
	return NULL;
}


// ---------------------------------------------------------------------------
// UgPlugin : UgPlugin is a base structure for downloading.
//
UgPlugin*	ug_plugin_new	(const UgPluginClass* plugin_class, UgDataset* dataset)
{
	UgPlugin*			plugin;
	UgPluginInitFunc	init;

	plugin = g_malloc0 (plugin_class->instance_size);

	// initialize base data
	plugin->plugin_class = plugin_class;
	plugin->lock = g_mutex_new ();
//	plugin->queue = NULL;
//	plugin->state = UG_STATE_STOP;
	plugin->ref_count = 1;

	init = plugin_class->init;
	if (init) {
		// If plugin initial failed, free resource and return NULL.
		if (init (plugin, dataset) == FALSE) {
			g_mutex_free (plugin->lock);
			g_free (plugin);
			return NULL;
		}
	}

	return plugin;
}

UgPlugin*	ug_plugin_new_by_name	(const gchar* name, UgDataset* dataset)
{
	const UgPluginClass*	plugin_class;

	plugin_class = ug_plugin_class_find (name, UG_REG_PLUGIN_CLASS);
	if (plugin_class == NULL)
		return NULL;
	return ug_plugin_new (plugin_class, dataset);
}

UgPlugin*	ug_plugin_new_by_data	(UgDataset* dataset)
{
	const UgPluginClass*	plugin_class;
	UgDataCommon*			common;
	gchar*					string;

	common = UG_DATASET_COMMON (dataset);
	if (common == NULL)
		return NULL;
	plugin_class = NULL;

	if (common->url) {
		string = g_uri_parse_scheme (common->url);
		if (string) {
			plugin_class = ug_plugin_class_find (string,
					UG_REG_PLUGIN_SCHEME);
			g_free (string);
		}
	}
	else if (common->file) {
		// filename maybe contain folder
		string = strrchr (common->file, G_DIR_SEPARATOR);
		if (string == NULL)
			string = common->file;
		// get filename extension
		string = strrchr (string, '.');
		if (string) {
			plugin_class = ug_plugin_class_find (string+1,
					UG_REG_PLUGIN_FILE_TYPE);
		}
	}
	else
		return NULL;

	if (plugin_class)
		return ug_plugin_new (plugin_class, dataset);
	else
		return NULL;
}

void	ug_plugin_ref	(UgPlugin* plugin)
{
	plugin->ref_count++;
}

void	ug_plugin_unref	(UgPlugin* plugin)
{
	UgFinalizeFunc	finalize;

	plugin->ref_count--;
	if (plugin->ref_count == 0) {
		finalize = plugin->plugin_class->finalize;
		if (finalize)
			finalize (plugin);

		// finalize base data
		g_mutex_free (plugin->lock);
		ug_data_list_free (plugin->messages);
		g_free (plugin);
	}
}

void	ug_plugin_lock	(UgPlugin* plugin)
{
	g_mutex_lock (plugin->lock);
}

void	ug_plugin_unlock	(UgPlugin* plugin)
{
	g_mutex_unlock (plugin->lock);
}

void	ug_plugin_delay (UgPlugin* plugin, guint millisecond)
{
	gulong u_second = millisecond * 1000;

	while (plugin->state == UG_STATE_ACTIVE) {
		if (u_second < 250000) {
			g_usleep (u_second);
			return;
		}
		g_usleep (250000);
		u_second -= 250000;
	}
}

void	ug_plugin_post (UgPlugin* plugin, UgMessage* message)
{
	UgMessage*	link;

	message->src = plugin;

	g_mutex_lock (plugin->lock);
	// Remove repeated progress message from queue
	link = plugin->messages;
	if (message->type == UG_MESSAGE_PROGRESS && link && link->type == UG_MESSAGE_PROGRESS)
		plugin->messages = link->next;
	else
		link = NULL;
	// prepend new message
	plugin->messages = ug_data_list_prepend (plugin->messages, message);
	g_mutex_unlock (plugin->lock);

	// free repeated progress message here to reduce lock time
	if (link)
		ug_data_free (link);
}

UgMessage*	ug_plugin_pop_all (UgPlugin* plugin)
{
	UgMessage*	messages;

	g_mutex_lock (plugin->lock);
	messages = plugin->messages;
	plugin->messages = NULL;
	g_mutex_unlock (plugin->lock);

	return ug_data_list_reverse (messages);
}

gboolean	ug_plugin_dispatch (UgPlugin* plugin, UgPluginCallback callback, gpointer user_data)
{
	UgMessage*	messages;
	UgMessage*	cur;

	messages = ug_plugin_pop_all (plugin);
	// dispatch messages
	if (callback) {
		for (cur = messages;  cur;  cur = cur->next)
			callback (plugin, cur, user_data);
	}
	// free messages
	ug_data_list_free (messages);
	// return TRUE if plug-in active.
	return (plugin->state == UG_STATE_ACTIVE);
}

// concatenate folder and file to new path and try create folder and empty file.
// If folder create failed, post UG_MESSAGE_ERROR_FOLDER_CREATE_FAILED.
// If file exist, it will change filename and post UG_MESSAGE_DATA_FILE_CHANGED.
// if function succeeded, return new path and it's folder length.
gchar*	ug_plugin_create_file (UgPlugin* plugin, const gchar* folder, const gchar* file, guint* folder_len)
{
	GString*		gstr;
	const gchar*	tail_str;
	guint			head_len;
	guint			location_len;
	guint			count;
	int				fd;

	if (folder == NULL && file == NULL) {
		ug_plugin_post (plugin, ug_message_new_error (UG_MESSAGE_ERROR_FILE_CREATE_FAILED, NULL));
		return NULL;
	}
	// concatenate full path
	gstr = g_string_sized_new (80);
	if (folder) {
		g_string_append (gstr, folder);
		if (gstr->len  &&  gstr->str [gstr->len -1] != G_DIR_SEPARATOR)
			g_string_append_c (gstr, G_DIR_SEPARATOR);
	}
	if (file)
		g_string_append (gstr, file);
	// get location_len (length of path exclude filename).
	tail_str = strrchr (gstr->str, G_DIR_SEPARATOR);
	if (tail_str == NULL)
		location_len = 0;
	else
		location_len = tail_str - gstr->str + 1;	// + G_DIR_SEPARATOR
	// create folder
	if (location_len && ug_create_dir_all (gstr->str, location_len) == -1) {
		ug_plugin_post (plugin, ug_message_new_error (UG_MESSAGE_ERROR_FOLDER_CREATE_FAILED, NULL));
		return g_string_free (gstr, TRUE);
	}
	// get head_len (length of folder + primary filename)
	// and tail_str ('.' + filename extension)
	tail_str = strrchr (gstr->str + location_len, '.');
	if (tail_str == NULL)
		tail_str = gstr->str + gstr->len;
	head_len = tail_str - gstr->str;
	tail_str = g_strdup (tail_str);
	// create file
	for (count=0;  count < 100;  count++) {
//		fd = open (gstr->str, O_CREAT | O_EXCL | O_RDWR,
//		           S_IREAD | S_IWRITE | S_IRGRP | S_IROTH);
		fd = ug_fd_open (gstr->str, UG_FD_O_CREATE | UG_FD_O_EXCL | UG_FD_O_RDWR,
		                 UG_FD_S_IREAD | UG_FD_S_IWRITE | UG_FD_S_IRGRP | UG_FD_S_IROTH);
		if (fd != -1) {
//			close (fd);
			ug_fd_close (fd);
			break;
		}
		g_string_truncate (gstr, head_len);
		g_string_append_printf (gstr, "(%d)", count);
		g_string_append (gstr, tail_str);
	}
	// free tail_str
	g_free ((gpointer) tail_str);
	// result
	if (count == 100) {
		ug_plugin_post (plugin,
				ug_message_new_error (UG_MESSAGE_ERROR_FILE_CREATE_FAILED, NULL));
		return g_string_free (gstr, TRUE);
	}
	if (count > 0) {
		ug_plugin_post (plugin,
				ug_message_new_data (UG_MESSAGE_DATA_FILE_CHANGED, gstr->str + location_len));
	}
	if (folder_len)
		*folder_len = location_len;

	return g_string_free (gstr, FALSE);
}

// rename file from old_utf8 to new_utf8.
// If file rename fail, it will post UG_MESSAGE_WARNING_FILE_RENAME_FAILED and return FALSE.
gboolean	ug_plugin_rename_file (UgPlugin* plugin, const gchar* old_utf8, const gchar* new_utf8)
{
	if (ug_rename_file (old_utf8, new_utf8) == 0)
		return TRUE;
	// file rename failed.
	ug_plugin_post (plugin,
			ug_message_new_warning (UG_MESSAGE_WARNING_FILE_RENAME_FAILED, NULL));
	return FALSE;
}

gchar*	ug_plugin_create_and_hide (UgPlugin* plugin, const gchar* folder, const gchar* file, guint* folder_len)
{
	gchar*	hidden_file;
	gchar*	path;

	hidden_file = g_strconcat (HIDDEN_FILE_CHAR_S, file, NULL);
	path = ug_plugin_create_file (plugin, folder, hidden_file, folder_len);
	g_free (hidden_file);
	return path;
}

gboolean	ug_plugin_rename_and_unhide (UgPlugin* plugin, const gchar* old_utf8, const gchar* new_utf8)
{
	gchar*	folder;
	gchar*	file;
	gchar*	path;

	folder = g_path_get_dirname (new_utf8);
	file = g_path_get_basename (new_utf8);
	path = ug_plugin_create_file (plugin, folder, file + 1, NULL);
	g_free (file);
	g_free (folder);
	if (path  &&  ug_rename_file (old_utf8, path) == 0) {
		g_free (path);
		return TRUE;
	}
	// file rename failed.
	g_free (path);
	ug_plugin_post (plugin,
			ug_message_new_warning (UG_MESSAGE_WARNING_FILE_RENAME_FAILED, NULL));
	return FALSE;
}

// --- virtual functions ---
UgResult ug_plugin_set_state (UgPlugin* plugin, UgState  state)
{
	UgSetStateFunc  set_state = plugin->plugin_class->set_state;

	if (set_state)
		return set_state (plugin, state);
	return UG_RESULT_UNSUPPORT;
}

UgResult ug_plugin_get_state (UgPlugin* plugin, UgState* state)
{
	UgGetStateFunc  get_state = plugin->plugin_class->get_state;

	if (get_state)
		return get_state (plugin, state);
	return UG_RESULT_UNSUPPORT;
}

UgResult ug_plugin_get (UgPlugin* plugin, guint parameter, gpointer data)
{
	UgGetFunc  get = plugin->plugin_class->get;

	if (get)
		return get (plugin, parameter, data);
	return UG_RESULT_UNSUPPORT;
}

UgResult ug_plugin_set (UgPlugin* plugin, guint parameter, gpointer data)
{
	UgSetFunc  set = plugin->plugin_class->set;

	if (set)
		return set (plugin, parameter, data);
	return UG_RESULT_UNSUPPORT;
}

