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


#ifndef UG_REGISTRY_H
#define UG_REGISTRY_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif


enum UgRegistryType
{
	UG_REG_NONE,
	UG_REG_MODULE,
	UG_REG_DATA_CLASS,
	UG_REG_OPTION_CLASS,
	UG_REG_PLUGIN_CLASS,
	UG_REG_PLUGIN_FILE_TYPE,
	UG_REG_PLUGIN_SCHEME,
	UG_REG_PLUGIN_TYPE_LAST = UG_REG_PLUGIN_SCHEME,

	UG_REG_N_TYPE,
};


void		ug_registry_insert (const gchar* key_name, enum UgRegistryType key_type, gpointer value);
void		ug_registry_remove (const gchar* key_name, enum UgRegistryType key_type);
gpointer	ug_registry_search (const gchar* key_name, enum UgRegistryType key_type);

// attachment
gboolean	ug_attachment_init (const gchar* dir);
gchar*		ug_attachment_alloc (guint* stamp);
void		ug_attachment_destroy (guint stamp);
void		ug_attachment_ref (guint stamp);
void		ug_attachment_unref (guint stamp);
void		ug_attachment_sync (void);

// ---------------------------------------------------------------------------
/*
typedef	struct	UgModule_				UgModule;

struct UgModule_
{
	// This structure base on UgClass
	const gchar*	name;
	gpointer		reserve;	// reserve for GModule-related code

	gchar*			folder;
	guint			ref_count;

	// null-terminated
	gpointer*		data_class;		// UgDataClass**
	gpointer*		plugin_class;	// UgPluginClass**
	gpointer*		option;			// UgOption**
};

void		ug_module_register        (UgModule* module);
void		ug_module_register_file   (const gchar* folder, const gchar* file);
void		ug_module_register_folder (const gchar* folder);
*/

#ifdef __cplusplus
}
#endif

#endif  // UG_REGISTRY_H

