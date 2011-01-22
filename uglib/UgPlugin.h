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

// define plug-ins struct and interface


#ifndef UG_PLUGIN_H
#define UG_PLUGIN_H

#include <glib.h>
#include <UgRegistry.h>
#include <UgDataset.h>
#include <UgMessage.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UG_PLUGIN_CAST(instance)			((UgPlugin*)(instance))

#define UG_PLUGIN_MEMBERS					\
	const UgPluginClass*	plugin_class;	\
	guint					ref_count;		\
	GMutex*					lock;			\
	UgMessage*				messages;		\
	UgState					state


typedef struct	UgPlugin_			UgPlugin;
typedef struct	UgPluginClass_		UgPluginClass;
typedef enum	UgResult_			UgResult;
typedef	enum	UgState_			UgState;

typedef gboolean (*UgPluginInitFunc)		(UgPlugin* plugin, UgDataset* dataset);
typedef gboolean (*UgGlobalInitFunc)		(void);
typedef void     (*UgGlobalFinalizeFunc)	(void);
typedef void     (*UgPluginCallback)		(UgPlugin* plugin, const UgMessage* message, gpointer user_data);

typedef UgResult (*UgSetStateFunc)	(gpointer instance, UgState  state);
typedef UgResult (*UgGetStateFunc)	(gpointer instance, UgState* state);
typedef UgResult (*UgSetFunc)		(gpointer instance, guint parameter, gpointer data);
typedef UgResult (*UgGetFunc)		(gpointer instance, guint parameter, gpointer data);
typedef void     (*UgOutputFunc)	(gpointer instance, const char* buffer, gint length, gint64 offset);

enum UgResult_
{
	UG_RESULT_ERROR,
	UG_RESULT_REFUSED,
	UG_RESULT_OK,
//	UG_RESULT_PENDING,
	UG_RESULT_UNSUPPORT,
};

enum	UgState_
{
	UG_STATE_NULL,			// file and memory resource is NULL
	UG_STATE_READY,			// file and memory resource ready, but stop.
	UG_STATE_PAUSED,
	UG_STATE_ACTIVE,
};

// ---------------------------------------------------------------------------
// UgPluginClass
//
struct UgPluginClass_
{
	// This structure base on UgClass
	const gchar*			name;
	gpointer				reserve;	// reserve for GModule-related code

	guint					instance_size;
	const char**			schemes;
	const char**			file_types;

	UgGlobalInitFunc		global_init;
	UgGlobalFinalizeFunc	global_finalize;

	UgPluginInitFunc		init;
	UgFinalizeFunc			finalize;

	UgSetStateFunc			set_state;
	UgGetStateFunc			get_state;
	UgSetFunc				set;
	UgGetFunc				get;
};

gboolean	ug_plugin_class_register	(const UgPluginClass* plugin_class);
void		ug_plugin_class_unregister	(const UgPluginClass* plugin_class);

// if type==NULL, type will be UG_REG_PLUGIN_CLASS.
const UgPluginClass*	ug_plugin_class_find	(const gchar* name,	enum UgRegistryType type);


// ---------------------------------------------------------------------------
// UgPlugin : UgPlugin is a base structure for downloading.
//
struct UgPlugin_
{
	UG_PLUGIN_MEMBERS;
//	const UgPluginClass*	plugin_class;
//	guint					ref_count;
//	GMutex*					lock;
//	UgMessage*				messages;
//	UgState					state;
};

UgPlugin*	ug_plugin_new			(const UgPluginClass* plugin_class, UgDataset* dataset);
UgPlugin*	ug_plugin_new_by_name	(const gchar* name, UgDataset* dataset);
UgPlugin*	ug_plugin_new_by_data	(UgDataset* dataset);

void	ug_plugin_ref		(UgPlugin* plugin);
void	ug_plugin_unref		(UgPlugin* plugin);

void	ug_plugin_lock		(UgPlugin* plugin);
void	ug_plugin_unlock	(UgPlugin* plugin);

// for retry delay
// delay millisecond. If plug-in state changed, this function will return.
void	ug_plugin_delay		(UgPlugin* plugin, guint millisecond);

// post one message to queue/pop all messages from queue.
void		ug_plugin_post		(UgPlugin* plugin, UgMessage* message);
UgMessage*	ug_plugin_pop_all	(UgPlugin* plugin);

// dispatch message for simple program.
// return TRUE if plug-in is running.
// Example :
//	while (ug_plugin_dispatch (plugin, NULL, NULL))
//		ug_plugin_delay (plugin, 1000);
gboolean	ug_plugin_dispatch	(UgPlugin* plugin, UgPluginCallback callback, gpointer user_data);

// concatenate folder and file to new path and try create folder and empty file.
// If folder create failed, post UG_MESSAGE_ERROR_FOLDER_CREATE_FAILED.
// If file exist, it will change filename and post UG_MESSAGE_DATA_FILE_CHANGED.
// if function succeeded, return new path and it's folder length.
//
// folder_utf8 : UTF-8 encoded folder name, it can set NULL.
// file_utf8   : UTF-8 encoded file name, it can set NULL.
// folder_len : folder length of new path. set NULL to ignore.
// Return     : concatenated path include folder and file. call g_free() to free it.
//
// e.g.
//
//	gchar*	path;
//	gchar*	path_folder;
//	guint	path_folder_len;
//	gchar*	path_file;
//
//	// below 3 line has the same result.
//	path = ug_plugin_create_file (plugin, "var", "temp/filename.ext",     &path_folder_len);
//	path = ug_plugin_create_file (plugin, NULL,  "var/temp/filename.ext", &path_folder_len);
//	path = ug_plugin_create_file (plugin, "var/temp/filename.ext", NULL,  &path_folder_len);
//
//	if (path) {
//		path_folder = g_strndup (path, path_folder_len);
//		path_file   = path + path_folder_len;
//	}
//
// result:
//	If path "var/temp/filename.ext" exist, result list below.
//	path        = "var/temp/filename(0).ext"
//	path_folder = "var/temp/"
//	path_file   = "filename(0).ext"
//
//
gchar*		ug_plugin_create_file (UgPlugin* plugin, const gchar* folder_utf8, const gchar* file_utf8, guint* folder_len);

// rename file from old_utf8 to new_utf8.
// If file rename fail, it will post UG_MESSAGE_WARNING_FILE_RENAME_FAILED and return FALSE.
gboolean	ug_plugin_rename_file (UgPlugin* plugin, const gchar* old_utf8, const gchar* new_utf8);

gchar*		ug_plugin_create_and_hide (UgPlugin* plugin, const gchar* folder, const gchar* file, guint* folder_len);
gboolean	ug_plugin_rename_and_unhide (UgPlugin* plugin, const gchar* old_utf8, const gchar* new_utf8);

// --- virtual functions ---
UgResult	ug_plugin_set_state	(UgPlugin* plugin, UgState  state);
UgResult	ug_plugin_get_state	(UgPlugin* plugin, UgState* state);
UgResult	ug_plugin_set (UgPlugin* plugin, guint parameter, gpointer data);
UgResult	ug_plugin_get (UgPlugin* plugin, guint parameter, gpointer data);


#ifdef __cplusplus
}
#endif

#endif  // UG_PLUGIN_H

