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

#include <UgRunning.h>
#include <UgString.h>
#include <UgCategory.h>
#include <UgData-download.h>

#include <glib/gi18n.h>

// ----------------------------------------------------------------------------
// UgRunning
//
UgRunning*	ug_running_new (void)
{
	UgRunning*	running;

	running = g_malloc0 (sizeof (UgRunning));
	g_queue_init (&running->group);
	return running;
}

void	ug_running_free (UgRunning* running)
{
	ug_running_clear (running);
	g_queue_clear (&running->group);
	g_free (running);
}

gboolean	ug_running_add (UgRunning* running, UgDataset* dataset)
{
	UgRelation*	relation;
	UgCategory*	category;

	// is this dataset in group?
	if (g_queue_find (&running->group, dataset))
		return TRUE;
	// create UgPlugin
	relation = UG_DATASET_RELATION (dataset);
	category = relation->category;
	if (relation->plugin == NULL) {
		relation->plugin = ug_plugin_new_by_data (dataset);
		if (relation->plugin == NULL) {
			// set relation->hints
			relation->hints |=  UG_HINT_ERROR;
			g_free (relation->message.string);
			relation->message.type = UG_MESSAGE_ERROR;
			relation->message.string = g_strdup (_("No plug-in support this scheme or file."));
			// notify
			if (category)
				ug_category_changed (category, dataset);
			// If this job can't activate, return FALSE.
			return FALSE;
		}
	}
	// change relation->hints and notify category
	relation->hints &= ~UG_HINT_INACTIVE;
	relation->hints |=  UG_HINT_DOWNLOADING;
	if (category)
		ug_category_changed (category, dataset);
	// add to group
	ug_dataset_ref (dataset);
	g_queue_push_tail (&running->group, dataset);
	// activate job
	ug_plugin_set_state (relation->plugin, UG_STATE_ACTIVE);
	ug_running_do_speed_limit (running);
	// If this job can activate, return TRUE.
	return TRUE;
}

void	ug_running_remove (UgRunning* running, UgDataset* dataset)
{
	UgRelation*	relation;
	UgCategory*	category;
	GList*		link;

	// is this dataset in group?
	link = g_queue_find (&running->group, dataset);
	if (link == NULL)
		return;
	// stop job
	relation = UG_DATASET_RELATION (dataset);
	if (relation->plugin) {
		ug_plugin_set_state (relation->plugin, UG_STATE_NULL);
		ug_plugin_unref (relation->plugin);
		relation->plugin = NULL;
	}
	// check relation->hints to avoid repeated notifications
	if (relation->hints & UG_HINT_ACTIVE) {
		// change relation->hints
		relation->hints &= ~UG_HINT_ACTIVE;
		// notify category
		category = relation->category;
		if (category)
			ug_category_changed (category, dataset);
	}
	// remove from group
	g_queue_delete_link(&running->group, link);
	ug_dataset_unref (dataset);
}

gboolean	ug_running_add_jobs (UgRunning* running, GList* list)
{
	gboolean	result = FALSE;

	for (;  list;  list = list->next) {
		if (ug_running_add (running, list->data))
			result = TRUE;
	}

	return result;
}

void	ug_running_remove_jobs (UgRunning* running, GList* list)
{
	for (;  list;  list = list->next)
		ug_running_remove (running, list->data);
}

void	ug_running_clear (UgRunning* running)
{
	GList*	list;

	list = g_list_copy (running->group.head);
	ug_running_remove_jobs (running, list);
	g_list_free (list);
}

GList*	ug_running_get_inactive (UgRunning* running)
{
	UgRelation*	relation;
	GList*		list;
	GList*		link;
	UgState		state;

	list = NULL;
	for (link = running->group.head;  link;  link = link->next) {
		relation = UG_DATASET_RELATION ((UgDataset*)link->data);
		ug_plugin_get_state (relation->plugin, &state);
		if (state == UG_STATE_ACTIVE || relation->hints & UG_HINT_ACTIVE)
			continue;
		// inactive list
		list = g_list_prepend (list, link->data);
	}

	return list;
}

guint	ug_running_get_n_jobs (UgRunning* running)
{
	return running->group.length;
}

gdouble		ug_running_get_speed (UgRunning* running)
{
	UgProgress*		progress;
	GList*			link;
	gdouble			speed = 0.0;

	for (link = running->group.head;  link;  link = link->next) {
		progress = UG_DATASET_PROGRESS ((UgDataset*)link->data);
		if (progress == NULL)
			continue;
		speed += progress->download_speed;
	}
	return speed;
}

void	ug_running_set_speed (UgRunning* running, guint64 speed_limit)
{
	running->speed_limit = speed_limit;
}

// This is a GSourceFunc, you can use it with GSource.
// It can adjust speed of all job.
gboolean	ug_running_do_speed_limit (UgRunning* running)
{
	UgRelation*	relation;
	gint64		average;
	GList*		link;

	if (running->speed_limit == 0)
		return TRUE;

	average = running->speed_limit / running->group.length;
	for (link = running->group.head;  link;  link = link->next) {
		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		ug_plugin_set (relation->plugin, UG_DATA_TYPE_INT64, &average);
	}

	// return FALSE if the source should be removed.
	return TRUE;
}

// This is a GSourceFunc, you can use it with GSource.
// It can dispatch all messages from all jobs.
gboolean	ug_running_dispatch (UgRunning* running)
{
	GList*	link;

	for (link = running->group.head;  link;  link = link->next)
		ug_running_dispatch_1 (running, link->data);
	// adjust speed limit after dispatching messages
	ug_running_do_speed_limit (running);

	// return FALSE if the source should be removed.
	return TRUE;
}

// It only dispatch messages from one of jobs.
void	ug_running_dispatch_1 (UgRunning* running, UgDataset* dataset)
{
	UgCategory*		category;
	UgRelation*		relation;
	UgMessage*		msg_list;
	UgMessage*		msg;
	union {
		UgDataCommon*	common;
		UgProgress*		progress;
		UgDataHttp*		http;
	} temp;


	relation = UG_DATASET_RELATION (dataset);
	// if category == NULL, it will not notify category that dataset was changed.
	category = relation->category;
	// if no plug-in, return.
	if (relation->plugin == NULL)
		return;
	// dispatch messages
	msg_list = ug_plugin_pop_all (relation->plugin);
	for (msg = msg_list;  msg;  msg = msg->next) {
		// call watch functions
		if (running->watch.func) {
			if (running->watch.func (running, msg, dataset, running->watch.data) == TRUE)
				continue;
		}

		// default message handler
		switch (msg->type) {
		case UG_MESSAGE_STATE:
			if (msg->data.v_int != UG_STATE_ACTIVE)
				relation->hints &= ~UG_HINT_ACTIVE;
			break;

		case UG_MESSAGE_PROGRESS:
			temp.progress = UG_DATASET_PROGRESS (dataset);
			if (temp.progress == NULL)
				temp.progress = ug_dataset_alloc_front (dataset, UgProgressClass);
			ug_plugin_get (relation->plugin, UG_DATA_TYPE_INSTANCE, temp.progress);
			break;

		case UG_MESSAGE_ERROR:
			relation->hints |= UG_HINT_ERROR;
			g_free (relation->message.string);
			relation->message.type = UG_MESSAGE_ERROR;
			relation->message.string = g_strdup (msg->string);
			break;

		case UG_MESSAGE_INFO:
			switch (msg->code) {
			case UG_MESSAGE_INFO_RETRY:
				temp.common = UG_DATASET_COMMON (dataset);
				temp.common->retry_count++;
				break;

			case UG_MESSAGE_INFO_COMPLETE:
				relation->hints |= UG_HINT_COMPLETED;
				break;

			case UG_MESSAGE_INFO_FINISH:
				relation->hints |= UG_HINT_FINISHED;
				break;

			case UG_MESSAGE_INFO_RESUMABLE:
			case UG_MESSAGE_INFO_NOT_RESUMABLE:
				g_free (relation->message.string);
				relation->message.type = UG_MESSAGE_INFO;
				relation->message.string = g_strdup (msg->string);
				break;

			default:
				break;
			}
			// End of switch (msg->code)
			break;
		// End of UG_MESSAGE_INFO

		case UG_MESSAGE_DATA:
			switch (msg->code) {
			case UG_MESSAGE_DATA_FILE_CHANGED:
				if (msg->data.v_string) {
					temp.common = UG_DATASET_COMMON (dataset);
					ug_str_set (&temp.common->file, msg->data.v_string, -1);
				}
				break;

			case UG_MESSAGE_DATA_URL_CHANGED:
			// HTTP message
			case UG_MESSAGE_DATA_HTTP_LOCATION:		// redirection
				if (msg->data.v_string) {
					temp.common = UG_DATASET_COMMON (dataset);
					ug_str_set (&temp.common->url, msg->data.v_string, -1);
					temp.http = ug_dataset_realloc (dataset, UgDataHttpClass, 0);
					temp.http->redirection_count++;
				}
				break;

			default:
				break;
			}
			// End of switch (msg->code)
			break;
		// End of UG_MESSAGE_DATA

		default:
			// if category == NULL, it will not notify category that dataset was changed.
			category = NULL;
			break;
		}
		// End of switch (msg->type)
	}
	// free message list
	ug_data_list_free (msg_list);

	// if category == NULL, it will not notify category that dataset was changed.
	if (category)
		ug_category_changed (category, dataset);
}

