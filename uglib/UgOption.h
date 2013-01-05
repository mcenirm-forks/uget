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

// collect result of GOptionContext to UgDataset

#ifndef UG_OPTION_H
#define UG_OPTION_H

#include <glib.h>
#include <UgDataset.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct	UgOption			UgOption;
typedef struct	UgOptionEntry		UgOptionEntry;
typedef struct	UgOptionMainData	UgOptionMainData;

typedef gboolean	(*UgOptionEntryGet)	(const UgOptionEntry* oentry, UgDataset* dataset);


// ----------------------------------------------------------------------------
// UgOption: integrate GOptionContext, GOptionGroup, and UgOptionEntry
//
struct	UgOption
{
	GOptionContext*		context;
	GOptionGroup*		group;		// main group
	UgOptionMainData*	data;		// main data
	GList*				list;		// list of UgOption
};

void	ug_option_init  (UgOption* uopt);

// if group is NULL, option->entry will add to main group.
void	ug_option_add   (UgOption* uopt, const UgOptionEntry* option, GOptionGroup* group);

// show help message and exit
void	ug_option_help  (UgOption* uopt, const char* progname, const char* help_option);

// This parse arguments and return list of newly-created UgDataset.
// To free the returned value, use:
//	g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
//	g_list_free (list);
GList*	ug_option_parse (UgOption* uopt, GPtrArray* args);

const UgOptionEntry*	ug_option_get_main_entry (void);


// ----------------------------------------------------------------------------
// UgOptionEntry
//
struct UgOptionEntry
{
	const gchar*		name;
	gpointer			reserve;			// reserve for GModule-related code

	GOptionEntry*		entry;				// for  g_option_group_add_entries()
	gpointer			data;				// argument data
	guint				data_size;			// size of argument data

	UgInitFunc			init;				// initialize UgOption::data
	UgOptionEntryGet	get;				// get argument data and set it to UgDataset
};

// These functions can use with g_list_foreach().
// g_list_foreach (UgOption_list, (GFunc) ug_option_entry_get, dataset);
// g_list_foreach (UgOption_list, (GFunc) ug_option_entry_clear, NULL);
void	ug_option_entry_get   (const UgOptionEntry* oentry, UgDataset* dataset);
void	ug_option_entry_clear (const UgOptionEntry* oentry);


// ---------------------------------------------------------------------------
// UgOptionMainData: main argument data
//
struct UgOptionMainData
{
	// option_entry->arg_data place below
	gboolean	version;
	gint		offline;

	gboolean	quiet;
	gint		category_index;
	gchar*		input_file;

	gchar*		folder;
	gchar*		file;

	gchar*		user;
	gchar*		password;

	struct
	{
		gint		type;
		gchar*		host;
		guint		port;
		gchar*		user;
		gchar*		password;
	} proxy;

	struct
	{
		gchar*		user;
		gchar*		password;
		gchar*		referrer;
		gchar*		user_agent;
		gchar*		cookie_data;
		gchar*		cookie_file;
		gchar*		post_data;
		gchar*		post_file;
	} http;

	struct
	{
		gchar*		user;
		gchar*		password;
	} ftp;
};


// ------------------------------------
// argument functions, free or copy GPtrArray from ug_ipc_peek() and ug_ipc_pop()
//
GPtrArray*	ug_arg_new  (int argc, char** argv, gboolean copy_strings);
void		ug_arg_free (GPtrArray* args, gboolean free_strings);
GPtrArray*	ug_arg_copy (GPtrArray* args, gboolean copy_strings);

// find -?, -h, --help, or --help- in command-line options and return it.
char*		ug_arg_find_help (int argc, char** argv);
// find -V, --version
char*		ug_arg_find_version (int argc, char** argv);


#ifdef __cplusplus
}
#endif

#endif  // UG_OPTION_H

