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

#ifdef _WIN32
#include <windows.h>
#endif

#include <UgetCmd.h>


// ----------------------------------------------------------------------------
// UgetCmd
//
static gboolean	uget_cmd_timer_ipc (UgetCmd* ugcmd);
static gboolean	uget_cmd_timer_queuing (UgetCmd* ugcmd);

void	uget_cmd_run (UgetCmd* ugcmd)
{
	uget_cmd_load (ugcmd);

	if (ugcmd->category_list == NULL) {
		ugcmd->category_list = g_list_append (ugcmd->category_list, ug_category_new_with_cmd());
		ugcmd->category_list = g_list_append (ugcmd->category_list, ug_category_new_with_cmd());
	}

	ugcmd->running = ug_running_new ();

	ugcmd->main_loop = g_main_loop_new (NULL, FALSE);

	// 0.5 seconds
	g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 500,
						(GSourceFunc) uget_cmd_timer_ipc, ugcmd, NULL);
	// 0.5 seconds
	g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 500,
						(GSourceFunc) ug_running_dispatch, ugcmd->running, NULL);
	// 1 seconds
	g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 1000,
						(GSourceFunc) uget_cmd_timer_queuing, ugcmd, NULL);

	g_main_loop_run (ugcmd->main_loop);
}

void	uget_cmd_save (UgetCmd* ugcmd)
{
	GList*	link;
	GList*	list;
	gchar*	file;

	// get all of UgDataset from all UgCategory
	list = NULL;
	for (link = ugcmd->category_list;  link;  link = link->next) {
		list = g_list_concat (list, ug_category_cmd_get_all ((UgCategory*) link->data));
	}

	// save all UgDataset
	file = g_build_filename (g_get_user_config_dir (),
	                         UGET_CMD_DIR, UGET_CMD_DOWNLOAD_FILE,
	                         NULL);
	ug_download_list_save (list, file);
	g_list_free (list);
	g_free (file);

	// save all UgCategory after calling ug_download_list_save()
	file = g_build_filename (g_get_user_config_dir (),
	                         UGET_CMD_DIR, UGET_CMD_CATEGORY_FILE,
	                         NULL);
	ug_category_list_save (ugcmd->category_list, file);
	g_free (file);
}

void	uget_cmd_load (UgetCmd* ugcmd)
{
	GList*			download_list;
	GList*			category_list;
	gchar*			file;

	// load all UgDataset from file
	file = g_build_filename (g_get_user_config_dir (),
	                         UGET_CMD_DIR, UGET_CMD_DOWNLOAD_FILE,
	                         NULL);
	download_list = ug_download_list_load (file);
	g_free (file);

	// load all UgCategory
	file = g_build_filename (g_get_user_config_dir (),
	                         UGET_CMD_DIR, UGET_CMD_CATEGORY_FILE,
	                         NULL);
	category_list = ug_category_list_load (file);
	g_free (file);

	// UgCategory will use data and functions from UgCategoryCmd.
	g_list_foreach (category_list, (GFunc) ug_category_use_cmd, NULL);

	// link and add UgDataset to UgCategory
	ug_category_list_link (category_list, download_list);
	ugcmd->category_list = category_list;

	// free unused UgDataset and list
	g_list_foreach (download_list, (GFunc) ug_dataset_unref, NULL);
	g_list_free (download_list);
}

static gboolean uget_cmd_timer_ipc (UgetCmd* ugcmd)
{
	UgCategory*		category;
	GPtrArray*		args;
	GList*			list;
	GList*			link;

	// get arguments from IPC
	args = ug_ipc_pop (&ugcmd->ipc);
	if (args == NULL)
		return TRUE;
	// parse arguments
	list = ug_option_parse (&ugcmd->option, args);
	ug_arg_free (args, TRUE);
	if (list == NULL)
		return TRUE;
	// create attachment (backup cookie & post file)
//	if (ug_download_create_attachment (list->data, FALSE) == TRUE)
//		g_list_foreach (list->next, (GFunc) ug_download_assign_attachment, list->data);
	// select category
	g_print ("uget-cmd: Get %d downloads.\n", g_list_length (list));
	if (ugcmd->option.data->category_index < 0 || ugcmd->option.data->category_index > 1)
		ugcmd->option.data->category_index = 0;
	category = g_list_nth_data (ugcmd->category_list, ugcmd->option.data->category_index);
	// add UgDataset to category
	for (link = list;  link;  link = link->next) {
		ug_data_assign (link->data, category->defaults);
		ug_category_cmd_add (category, link->data);
	}
	g_list_foreach (list, (GFunc) ug_dataset_unref, NULL);
	g_list_free (list);

	return TRUE;
}

static gboolean	uget_cmd_timer_queuing (UgetCmd* ugcmd)
{
	GList*			jobs;
	GList*			link;
	gdouble			speed;

	// do something for inactive jobs
	jobs = ug_running_get_inactive (ugcmd->running);
	for (link = jobs;  link;  link = link->next) {
		// remove inactive jobs from group
		ug_running_remove (ugcmd->running, link->data);
	}
	g_list_free (jobs);

	// get queuing jobs from categories and activate them
	for (link = ugcmd->category_list;  link;  link = link->next) {
		jobs = ug_category_cmd_get_jobs (link->data);
		ug_running_add_jobs (ugcmd->running, jobs);
		g_list_free (jobs);
	}

	// display number of jobs and speed
	if (ug_running_get_n_jobs (ugcmd->running) > 0) {
		speed = ug_running_get_speed (ugcmd->running);
		g_print ("%u jobs, %.2f KiB\n",
				ug_running_get_n_jobs (ugcmd->running),
				speed / 1024.0);
	}

	return TRUE;
}

