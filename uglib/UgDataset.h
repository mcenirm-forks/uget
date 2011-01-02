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

// UgDataset		collection of all UgDataList-based instance
//
// UgData
// |
// `- UgDataset
//
#ifndef UG_DATASET_H
#define UG_DATASET_H

#include <glib.h>
#include <UgData.h>

#ifdef __cplusplus
extern "C" {
#endif

// These macro is for internal use only.
// use ug_dataset_get(dataset, UgDataCommonClass, 0) to instead UG_DATASET_COMMON(dataset)
#define	UG_DATASET_COMMON(dataset)			( (UgDataCommon*)((dataset)->data[0]) )
#define	UG_DATASET_PROXY(dataset)			( (UgDataProxy*) ((dataset)->data[2]) )
#define	UG_DATASET_PROGRESS(dataset)		( (UgProgress*)  ((dataset)->data[4]) )
#define	UG_DATASET_RELATION(dataset)		( (UgRelation*)  ((dataset)->data[6]) )

typedef struct	UgDataset_			UgDataset;		// collection of all UgDataList-based instance


// ----------------------------------------------------------------------------
// UgDataset : collection of all UgDataList-based instance.
//
struct UgDataset_
{
	UG_DATA_MEMBERS;
//	const UgDataClass*	data_class;		// for UgMarkup parse/write

	UgDataList**		data;
	const UgDataClass**	key;
	guint				data_len;
	guint				alloc_len;

	guint				ref_count;

	// call destroy.func(destroy.data) when destroying.
	struct
	{
		UgNotifyFunc	func;
		gpointer		data;
	} destroy;

	struct
	{
		gpointer		pointer;
		gpointer		data;
	} user;
};

UgDataset*	ug_dataset_new   (void);
void		ug_dataset_ref   (UgDataset* dataset);
void		ug_dataset_unref (UgDataset* dataset);

// Gets the element at the given position in a list.
gpointer	ug_dataset_get (UgDataset* dataset, const UgDataClass* data_class, guint nth);

// Removes the element at the given position in a list.
void		ug_dataset_remove (UgDataset* dataset, const UgDataClass* data_class, guint nth);

// If nth instance of data_class exist, return nth instance.
// If nth instance of data_class not exist, alloc new instance in tail and return it.
gpointer	ug_dataset_realloc (UgDataset* dataset, const UgDataClass* data_class, guint nth);

gpointer	ug_dataset_alloc_front (UgDataset* dataset, const UgDataClass* data_class);
gpointer	ug_dataset_alloc_back  (UgDataset* dataset, const UgDataClass* data_class);

// ------------------------------------
// UgDataset list functions
guint			ug_dataset_list_length (UgDataset* dataset, const UgDataClass* data_class);

UgDataList**	ug_dataset_alloc_list (UgDataset* dataset, const UgDataClass* data_class);

UgDataList**	ug_dataset_get_list (UgDataset* dataset, const UgDataClass* data_class);

// free old list in dataset and set list with new_list.
void			ug_dataset_set_list (UgDataset* dataset, const UgDataClass* data_class, gpointer new_list);

// Cuts the element at the given position in a list.
//UgDataList*	ug_dataset_cut_list (UgDataset* dataset, const UgDataClass* data_class, guint nth);
gpointer		ug_dataset_cut_list (UgDataset* dataset, const UgDataClass* data_class, guint nth);


#ifdef __cplusplus
}
#endif

#endif  // UG_DATASET_H

