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


#ifndef UG_CATEGORY_CMD_H
#define UG_CATEGORY_CMD_H

#include <UgCategory.h>

#ifdef __cplusplus
extern "C" {
#endif


#define	UG_CATEGORY_CMD(category)		((UgCategoryCmd*) (category)->user.category)
#define	UG_RELATION_CMD_QUEUE(relation)	((GQueue*)        (relation)->user.pointer)
#define	UG_RELATION_CMD_QLINK(relation)	((GList*)         (relation)->user.position)


// ----------------------------------------------------------------------------
// UgCategoryCmd: additional data for UgCategory and command-line
//
typedef struct	UgCategoryCmd_			UgCategoryCmd;

struct UgCategoryCmd_
{
	GQueue			active;
	GQueue			queuing;
	GQueue			finished;
	GQueue			recycled;
};

// create UgCategory and set additional data and functions.
UgCategory*		ug_category_new_with_cmd (void);

// set additional data and functions to UgCategory.
void	ug_category_use_cmd		(UgCategory* category);

// functions will be used by UgCategoryFuncs (UgCategory::funcs)
void	ug_category_cmd_add		(UgCategory* category, UgDataset* dataset);
GList*	ug_category_cmd_get_all	(UgCategory* category);
GList*	ug_category_cmd_get_jobs(UgCategory* category);
void	ug_category_cmd_changed	(UgCategory* category, UgDataset* dataset);

// other functions used by uget-cmd
void	ug_category_cmd_remove	(UgCategory* category, UgDataset* dataset);
void	ug_category_cmd_clear	(UgCategory* category, UgCategoryHints hint, guint from_nth);
void	ug_category_cmd_move_to	(UgCategory* category, UgDataset* dataset, UgCategory* dest);


#ifdef __cplusplus
}
#endif

#endif  // UG_CATEGORY_CMD_H

