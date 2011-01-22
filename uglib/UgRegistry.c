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
#include <stdlib.h>
#include <glib.h>
//#include <gmodule.h>

#include <UgRegistry.h>
#include <UgUtils.h>

static GHashTable*	string_hash	= NULL;
static const gchar*	type_string[UG_REG_N_TYPE] =
{
	NULL,				// UG_REG_NONE
	"Module",			// UG_REG_MODULE
	"DataClass",		// UG_REG_DATA_CLASS
	"OptionClass",		// UG_REG_OPTION_CLASS
	"PluginClass",		// UG_REG_PLUGIN_CLASS
	"PluginFileType",	// UG_REG_PLUGIN_FILE_TYPE
	"PluginScheme",		// UG_REG_PLUGIN_SCHEME
};

void	ug_registry_insert (const gchar* key_name, enum UgRegistryType key_type, gpointer value)
{
	gchar*		key;

	if (key_type >= UG_REG_N_TYPE)
		return;
	if (string_hash == NULL)
		string_hash = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

	key = g_strconcat (key_name, "-", type_string [key_type], NULL);
	g_hash_table_insert (string_hash, key, value);
//	key will be freed by g_hash_table_insert() in this case.
}

void	ug_registry_remove (const gchar* key_name, enum UgRegistryType key_type)
{
	gchar*		key;

	if (string_hash == NULL || key_type >= UG_REG_N_TYPE)
		return;

	key = g_strconcat (key_name, "-", type_string [key_type], NULL);
	g_hash_table_remove (string_hash, key);
	g_free (key);
}

gpointer	ug_registry_search (const gchar* key_name, enum UgRegistryType key_type)
{
	gchar*		key;
	gpointer	value;

	if (string_hash == NULL || key_type >= UG_REG_N_TYPE)
		return NULL;

	key = g_strconcat (key_name, "-", type_string [key_type], NULL);
	value = g_hash_table_lookup (string_hash, key);
	g_free (key);
	return value;
}


// ---------------------------------------------------------------------------
// attachment
//
static GHashTable*	attachment_hash	= NULL;
static gchar*		attachment_dir  = NULL;

gboolean	ug_attachment_init (const gchar* dir)
{
	if (attachment_hash == NULL)
		attachment_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
	// directory
	if (dir)
		attachment_dir = g_strdup (dir);
	else {
		attachment_dir = g_build_filename (g_get_user_config_dir (),
				"uGet", "attachment", NULL);
	}
	if (ug_create_dir_all (attachment_dir, -1) == -1)
		return FALSE;
	else
		return TRUE;
}

gchar*	ug_attachment_alloc (guint* stamp)
{
	GString*		gstr;
	guint			length;
	guint			count;
	guint			value;

	if (attachment_dir == NULL)
		ug_attachment_init (NULL);
	// create new attachment dir
	gstr = g_string_new (attachment_dir);
	g_string_append_c (gstr, G_DIR_SEPARATOR);
	length = gstr->len;
	for (count = 0;  count < 30;  count++) {
		value = g_random_int ();
		if (value) {
			g_string_truncate (gstr, length);
			g_string_append_printf (gstr, "%x", value);
			if (ug_create_dir (gstr->str) == 0) {
				*stamp = value;
				ug_attachment_ref (value);
				return g_string_free (gstr, FALSE);
			}
		}
	}

	g_string_free (gstr, TRUE);
	return NULL;
}

void	ug_attachment_destroy (guint stamp)
{
	gchar*	dir;

	g_hash_table_remove (attachment_hash, GUINT_TO_POINTER (stamp));
	dir = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%x",
			attachment_dir, stamp);
	ug_delete_dir_recursive (dir);
	g_free (dir);
}

void	ug_attachment_ref (guint stamp)
{
	guint			count;
	gpointer		pointer;

	if (attachment_hash == NULL)
		attachment_hash = g_hash_table_new (g_direct_hash, g_direct_equal);
	if (stamp == 0)
		return;

	pointer = g_hash_table_lookup (attachment_hash, GUINT_TO_POINTER (stamp));
	count = GPOINTER_TO_UINT (pointer) + 1;
	g_hash_table_insert (attachment_hash, GUINT_TO_POINTER (stamp),
			GUINT_TO_POINTER (count));
}

void	ug_attachment_unref (guint stamp)
{
	guint			count;
	gpointer		pointer;

	if (attachment_hash == NULL  ||  stamp == 0)
		return;

	pointer = g_hash_table_lookup (attachment_hash, GUINT_TO_POINTER (stamp));
	if (pointer) {
		count = GPOINTER_TO_UINT (pointer) - 1;
		if (count == 0)
			ug_attachment_destroy (stamp);
		else {
			g_hash_table_insert (attachment_hash, GUINT_TO_POINTER (stamp),
					GUINT_TO_POINTER (count));
		}
	}
}

void	ug_attachment_sync (void)
{
	GDir*		dir;
	guint		stamp;
	gchar*		path;
	gchar*		name;
	gchar*		locale_dir;

	if (g_get_filename_charsets (NULL)) {
		locale_dir = NULL;
		dir = g_dir_open (attachment_dir, 0, NULL);
	}
	else {
		locale_dir = g_filename_from_utf8 (attachment_dir, -1, NULL, NULL, NULL);
		dir = g_dir_open (locale_dir, 0, NULL);
	}

	if (dir == NULL) {
		g_free (locale_dir);
		return;
	}

	for (;;) {
		name = (gchar*) g_dir_read_name (dir);
		if (name == NULL)
			break;
		stamp = strtoul (name, NULL, 16);
		if (g_hash_table_lookup (attachment_hash, GUINT_TO_POINTER (stamp)))
			continue;
		// delete directory
		if (locale_dir == NULL)
			path = g_build_filename (attachment_dir, name, NULL);
		else {
			name = g_build_filename (locale_dir, name, NULL);
			path = g_filename_to_utf8 (name, -1, NULL, NULL, NULL);
			g_free (name);
		}
		ug_delete_dir_recursive (path);
		g_free (path);
	}

	g_free (locale_dir);
	g_dir_close (dir);
}


// ---------------------------------------------------------------------------
// register from file & folder
/*

#ifdef _WIN32
#define FILE_EXT        ".dll"
#define FILE_EXT_LEN    4
#else
#define FILE_EXT        ".so"
#define FILE_EXT_LEN    3
#endif

gboolean	ug_module_register_file	(const gchar* file_path)
{
	UgDataClass*	data_class;
	UgPluginClass*	plugin_class;
	UgOption*		option;
	GModule*		module;
	UgModuleInfo*	module_info;
	gboolean		result = FALSE;

	module = g_module_open (file_path, G_MODULE_BIND_LAZY);
	if (module == NULL) {
		g_message ("g_module_open() fail : %s.\n", g_module_error ());
		return FALSE;
	}

	if ( g_module_symbol (module, "ug_module_info", (gpointer)&module_info) ) {
		for (; obj_class; obj_class++)
			ug_class_register (obj_class);
		result = TRUE;
	}
//	g_module_close (module);

	return result;
}

guint	ug_module_register_folder	(const gchar* folder)
{
	const gchar*	filename;
	gchar*			full_path;
	guint			length;
	guint			count = 0;
	GDir*			dir;

	dir = g_dir_open (folder, 0, NULL);

	if (dir == NULL) {
		g_message ("ug_module_register_folder () : %s open fail\n", folder);
		return 0;
	}

	for (;;) {
		filename = g_dir_read_name (dir);
		if (filename == NULL)
			break;

		length = strlen (filename);
		if (length < FILE_EXT_LEN)
			continue;
		if (strncmp (filename +length -FILE_EXT_LEN, FILE_EXT, FILE_EXT_LEN) != 0)
			continue;

		full_path = g_build_filename (folder, filename, NULL);
		g_message ("ug_module_register_folder () : load %s\n", full_path);
		if (ug_module_register_file (full_path))
			count++;
	}

	return count;
}

*/
