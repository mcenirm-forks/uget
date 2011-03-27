/*
 *
 *   Copyright (C) 2005-2011 by C.H. Huang
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

// a group contains all running jobs


#ifndef UG_RUNNING_H
#define UG_RUNNING_H

#include <UgDataset.h>
#include <UgMessage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgRunning		UgRunning;

typedef gboolean (*UgWatchFunc)	(gpointer instance, UgMessage* message, UgDataset* dataset, gpointer data);


// ----------------------------------------------------------------------------
// UgRunning: a group for active jobs (UgDataset)
//
struct UgRunning
{
	GQueue		group;

	guint64		speed_limit;
	// global watch function
	struct {
		UgWatchFunc		func;
		gpointer		data;
	} watch;
};

void		ug_running_init (UgRunning* running);
void		ug_running_finalize (UgRunning* running);

UgRunning*	ug_running_new (void);
void		ug_running_free (UgRunning* running);

gboolean	ug_running_add (UgRunning* running, UgDataset* dataset);
void		ug_running_remove (UgRunning* running, UgDataset* dataset);

gboolean	ug_running_add_jobs (UgRunning* running, GList* list);
void		ug_running_remove_jobs (UgRunning* running, GList* list);
void		ug_running_clear (UgRunning* running);

GList*		ug_running_get_inactive (UgRunning* running);

guint		ug_running_get_n_jobs (UgRunning* running);

gint64		ug_running_get_speed (UgRunning* running);
void		ug_running_set_speed (UgRunning* running, guint64 speed_limit);

// This is a GSourceFunc, you can use it with GSource.
// It can adjust speed of all job.
gboolean	ug_running_do_speed_limit (UgRunning* running);

// This is a GSourceFunc, you can use it with GSource.
// It can dispatch all messages from all jobs.
gboolean	ug_running_dispatch (UgRunning* running);

// It only dispatch messages from one of jobs.
void		ug_running_dispatch_1 (UgRunning* running, UgDataset* dataset);

//void		ug_running_foreach(UgRunning* running, GFunc func, gpointer data);
#define		ug_running_foreach(running, gfunc, data)		g_queue_foreach (&running->group, gfunc, data)


#ifdef __cplusplus
}
#endif

#endif	// UG_RUNNING_H

