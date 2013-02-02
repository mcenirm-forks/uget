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

#include <stdlib.h>
#include <string.h>
#include <UgetData.h>
#include <UgCategory-cmd.h>


// ----------------------------------------------------------------------------
// UgCategoryCmd: additional data for UgCategory and command-line.
//
// UgCategory::user.category used for getting UgCategoruCmd
// UgetRelation::user.category used for getting UgCategoruCmd
// UgetRelation::user.storage  used for getting GQueue in UgCategoryCmd
// UgetRelation::user.position used for getting link in GQueue.
//
static UgCategoryCmd*	ug_category_cmd_new  (void);
static void				ug_category_cmd_free (UgCategoryCmd* ccmd);

const UgCategoryFuncs	ccmd_funcs =
{
	ug_category_cmd_add,
	ug_category_cmd_get_all,
	ug_category_cmd_get_tasks,
	ug_category_cmd_changed,
};

static UgCategoryCmd*	ug_category_cmd_new (void)
{
	UgCategoryCmd*	ccmd;

	ccmd = g_malloc (sizeof (UgCategoryCmd));
	g_queue_init (&ccmd->active);
	g_queue_init (&ccmd->queuing);
	g_queue_init (&ccmd->finished);
	g_queue_init (&ccmd->recycled);
	return ccmd;
}

static void	ug_category_cmd_free (UgCategoryCmd* ccmd)
{
	g_queue_foreach (&ccmd->queuing,  (GFunc) ug_dataset_unref, NULL);
	g_queue_foreach (&ccmd->finished, (GFunc) ug_dataset_unref, NULL);
	g_queue_foreach (&ccmd->recycled, (GFunc) ug_dataset_unref, NULL);
	g_queue_clear (&ccmd->queuing);
	g_queue_clear (&ccmd->finished);
	g_queue_clear (&ccmd->recycled);
	g_free (ccmd);
}

UgCategory*	ug_category_new_with_cmd (void)
{
	UgCategory*	category;

	category = ug_category_new ();
	ug_category_use_cmd (category);
	return category;
}

void	ug_category_use_cmd (UgCategory* category)
{
	if (category->user.category == NULL) {
		category->user.category	= ug_category_cmd_new ();
		// destroy notify
		category->destroy.func  = (UgNotifyFunc) ug_category_cmd_free;
		category->destroy.data  = category->user.category;
		// functions
		category->funcs = &ccmd_funcs;
	}
}

// add dataset to category and increase reference count of dataset.
void	ug_category_cmd_add (UgCategory* category, UgDataset* dataset)
{
	UgetRelation*			relation;
	UgCategoryCmd*		ccmd;

	ccmd = category->user.category;
	// add and set UgetRelation to dataset
	ug_dataset_ref (dataset);
	relation = UG_DATASET_RELATION (dataset);
	if (relation == NULL)
		relation = ug_dataset_alloc_front (dataset, UgetRelationInfo);
	relation->category = category;
	relation->user.category = ccmd;

	// select queue & add task to it
	if (relation->hints & UG_HINT_RECYCLED) {
		g_queue_push_head (&ccmd->recycled, dataset);
		relation->user.storage  = &ccmd->recycled;
		relation->user.position =  ccmd->recycled.head;
	}
	else if (relation->hints & UG_HINT_FINISHED) {
		g_queue_push_head (&ccmd->finished, dataset);
		relation->user.storage  = &ccmd->finished;
		relation->user.position =  ccmd->finished.head;
	}
	else {
		g_queue_push_tail (&ccmd->queuing, dataset);
		relation->user.storage  = &ccmd->queuing;
		relation->user.position =  ccmd->queuing.tail;
	}
}

// get all tasks(UgDataset) in this category.
// To free the returned value, use g_list_free (list).
GList*	ug_category_cmd_get_all (UgCategory* category)
{
	UgCategoryCmd*	ccmd;
	GList*			link;
	GList*			list;

	ccmd = category->user.category;
	list = NULL;
	for (link = ccmd->recycled.tail;  link;  link = link->prev)
		list = g_list_prepend (list, link->data);

	for (link = ccmd->finished.tail;  link;  link = link->prev)
		list = g_list_prepend (list, link->data);

	for (link = ccmd->queuing.tail;  link;  link = link->prev)
		list = g_list_prepend (list, link->data);

	return list;
}

// get queuing tasks(UgDataset) in this category/
// This function should be noticed UgCategory::active_limit, because
// application will try to activate all returned dataset.
// To free the returned value, use g_list_free (list).
GList*	ug_category_cmd_get_tasks (UgCategory* category)
{
	UgCategoryCmd*	ccmd;
	UgetRelation*		relation;
	GList*			link;
	GList*			tasks;
	guint			tasks_len;

	ccmd = category->user.category;
	if (category->active_limit <= ccmd->active.length)
		return NULL;

	tasks = NULL;
	tasks_len = 0;

	for (link = ccmd->queuing.head;  link;  link = link->next) {
		relation = UG_DATASET_RELATION ((UgDataset*) link->data);
		if (relation->hints & UG_HINT_UNRUNNABLE)
			continue;

		tasks = g_list_prepend (tasks, link->data);
		tasks_len++;
		if (tasks_len >= (category->active_limit - ccmd->active.length))
			break;
	}

	return g_list_reverse (tasks);
}

// used to notify category that it's dataset was changed.
// It may change hints and switch dataset to another internal queue of category.
void	ug_category_cmd_changed (UgCategory* category, UgDataset* dataset)
{
	UgCategoryCmd*	ccmd;
	UgetRelation*		relation;
	GQueue*			queue;

	ccmd = category->user.category;
	relation = UG_DATASET_RELATION (dataset);
	// select queue
	if (relation->hints & UG_HINT_RECYCLED) {
		queue = &ccmd->recycled;
		relation->hints &= ~UG_HINT_FINISHED;
	}
	else if (relation->hints & UG_HINT_FINISHED) {
		queue = &ccmd->finished;
		relation->hints &= ~UG_HINT_RECYCLED;
	}
	else if (relation->hints & UG_HINT_ACTIVE)
		queue = &ccmd->active;
	else
		queue = &ccmd->queuing;

	// Don't switch task to the same queue.
	if (queue != relation->user.storage) {
		// switch task to new queue
		g_queue_delete_link (relation->user.storage, relation->user.position);
		if (queue == &ccmd->finished || queue == &ccmd->recycled) {
			g_queue_push_head (queue, dataset);
			relation->user.storage  = queue;
			relation->user.position = queue->head;
		}
		else {
			g_queue_push_tail (queue, dataset);
			relation->user.storage  = queue;
			relation->user.position = queue->tail;
		}
	}
}

void	ug_category_cmd_remove (UgCategory* category, UgDataset* dataset)
{
	UgetRelation*		relation;

	relation = UG_DATASET_RELATION (dataset);
	relation->category = NULL;
	relation->user.category = NULL;
	g_queue_delete_link (relation->user.storage, relation->user.position);
	// delete data and files
	ug_download_delete_temp (dataset);
	ug_dataset_unref (dataset);
}

void	ug_category_cmd_clear  (UgCategory* category, UgCategoryHints hint, guint from_nth)
{
	UgCategoryCmd*	ccmd;
	GQueue*			queue;

	ccmd = category->user.category;
	// select queue
	if (hint & UG_HINT_RECYCLED)
		queue = &ccmd->recycled;
	else if (hint & UG_HINT_FINISHED)
		queue = &ccmd->finished;
	else
		queue = &ccmd->queuing;

	while (queue->length > from_nth)
		ug_category_cmd_remove (category, queue->tail->data);
}

void	ug_category_cmd_move_to (UgCategory* category, UgDataset* dataset, UgCategory* dest)
{
	UgetRelation*		relation;

	relation = UG_DATASET_RELATION (dataset);
	if (relation->category == category)
		return;
	// move
	ug_dataset_ref (dataset);
	ug_category_cmd_remove (category, dataset);
	ug_category_cmd_add (category, dataset);
	ug_dataset_unref (dataset);
}

