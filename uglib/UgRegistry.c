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

#include <string.h>
#include <stdlib.h>
#include <glib.h>
//#include <gmodule.h>

#include <UgRegistry.h>
#include <UgUtils.h>

static GHashTable*		registry_hash	= NULL;
//static GStaticMutex	registry_mutex	= G_STATIC_MUTEX_INIT;

void	ug_registry_insert (const char* key, const void* value)
{
	GList*	list;

	if (registry_hash == NULL)
		registry_hash = g_hash_table_new (g_str_hash, g_str_equal);

	list = g_hash_table_lookup (registry_hash, key);
	// if key doesn't exist in registry_hash, duplicate it.
	if (list == NULL)
		key = g_strdup (key);
	// add value to list and update list in registry_hash.
	list = g_list_prepend (list, (gpointer) value);
	g_hash_table_insert (registry_hash, (gpointer) key, list);
}

void	ug_registry_remove (const char* key, const void* value)
{
	GList*		list;
	gpointer	orig_key;

	if (registry_hash == NULL)
		return;

	list = g_hash_table_lookup (registry_hash, key);
	if (list) {
		// remove specified value from list
		list = g_list_remove (list, value);
		// if list has data, use new list instead of old one.
		// otherwise key and value must be removed.
		if (list)
			g_hash_table_insert (registry_hash, (gpointer) key, list);
		else {
			// the original key must be freed.
			g_hash_table_lookup_extended (registry_hash, key, &orig_key, NULL);
			g_hash_table_remove (registry_hash, key);
			g_free (orig_key);
		}
	}
}

int		ug_registry_exist  (const char* key, const void* value)
{
	GList*	list;

	if (registry_hash) {
		list = g_hash_table_lookup (registry_hash, key);
		if (g_list_find (list, value))
			return TRUE;
	}

	return FALSE;
}

void*	ug_registry_find (const char* key)
{
	GList*	list;

	if (registry_hash) {
		list = g_hash_table_lookup (registry_hash, key);
		if (list)
			return list->data;
	}

	return NULL;
}


// ---------------------------------------------------------------------------
// counting

static GHashTable*		counting_hash	= NULL;
//static GStaticMutex	counting_mutex	= G_STATIC_MUTEX_INIT;

unsigned int	ug_counting_current  (const void* key)
{
	if (counting_hash)
		return GPOINTER_TO_UINT (g_hash_table_lookup (counting_hash, key));

	return 0;
}

unsigned int	ug_counting_increase (const void* key)
{
	unsigned int	value;

	if (counting_hash == NULL)
		counting_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	value = GPOINTER_TO_UINT (g_hash_table_lookup (counting_hash, key)) + 1;
	g_hash_table_insert (counting_hash, (gpointer) key, GUINT_TO_POINTER (value));

	return value;
}

unsigned int	ug_counting_decrease (const void* key)
{
	unsigned int	value;

	if (counting_hash) {
		value = GPOINTER_TO_UINT (g_hash_table_lookup (counting_hash, key));
		if (value) {
			value-- ;
			g_hash_table_insert (counting_hash, (gpointer) key, GUINT_TO_POINTER (value));
		}
		return value;
	}
	return 0;
}


// ---------------------------------------------------------------------------
// attachment
static GHashTable*	attachment_hash	= NULL;
static char*		attachment_dir  = NULL;

int		ug_attachment_init (const char* dir)
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

char*	ug_attachment_alloc (unsigned int* stamp)
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

void	ug_attachment_destroy (unsigned int stamp)
{
	gchar*	dir;

	g_hash_table_remove (attachment_hash, GUINT_TO_POINTER (stamp));
	dir = g_strdup_printf ("%s" G_DIR_SEPARATOR_S "%x",
			attachment_dir, stamp);
	ug_delete_dir_recursive (dir);
	g_free (dir);
}

void	ug_attachment_ref (unsigned int stamp)
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

void	ug_attachment_unref (unsigned int stamp)
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

