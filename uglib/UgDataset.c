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

#include <stdlib.h>
#include <string.h>
#include <memory.h>
// uglib
#include <UgDataset.h>
#include <UgCategory.h>
#include <UgData-download.h>


// ----------------------------------------------------------------------------
// UgDataset
//
static void	ug_dataset_init      (UgDataset* dataset);
static void	ug_dataset_finalize  (UgDataset* dataset);
static void	ug_dataset_assign    (UgDataset* dataset, UgDataset* src);
static void	ug_dataset_in_markup (UgDataset* dataset, GMarkupParseContext* context);
static void	ug_dataset_to_markup (UgDataset* dataset, UgMarkup* markup);

static UgDataEntry	dataset_entry[] =
{
	{"DataList",	0,	UG_DATA_TYPE_CUSTOM,	(UgInMarkupFunc) ug_dataset_in_markup,	(UgToMarkupFunc) ug_dataset_to_markup},
	{NULL}		// null-terminated
};

static UgDataClass	dataset_class =
{
	"dataset",				// name
	NULL,					// reserve
	sizeof (UgDataset),		// instance_size
	dataset_entry,			// entry

	(UgInitFunc)		ug_dataset_init,
	(UgFinalizeFunc)	ug_dataset_finalize,
	(UgAssignFunc)		ug_dataset_assign,
};
// extern
//const	UgDataClass*	UgDatasetClass = &dataset_class;


static void	ug_dataset_init	(UgDataset* dataset)
{
	dataset->ref_count = 1;

	// for macros
	ug_dataset_alloc_list (dataset, UgDataCommonClass);	//UG_DATASET_COMMON   0
	ug_dataset_alloc_list (dataset, UgDataProxyClass);	//UG_DATASET_PROXY    1
	ug_dataset_alloc_list (dataset, UgProgressClass);	//UG_DATASET_PROGRESS 2
	ug_dataset_alloc_list (dataset, UgRelationClass);	//UG_DATASET_RELATION 3
}

static void	ug_dataset_finalize (UgDataset* dataset)
{
	guint	index;

	if (dataset->destroy.func)
		dataset->destroy.func (dataset->destroy.data);

	for (index = 0;  index < dataset->data_len;  index += 2)
		ug_data_list_free (dataset->data[index]);
	g_free (dataset->data);
}

static void	ug_dataset_assign (UgDataset* dataset, UgDataset* src)
{
	const UgDataClass*	data_class;
	UgDataList**		data_list;
	guint				index;

	for (index = 0;  index < src->data_len;  index += 2) {
		data_class = src->key[index];
		if (data_class == NULL || data_class->assign == NULL)
			continue;
		// assign list
		data_list = ug_dataset_get_list (dataset, data_class);
		if (data_list == NULL)
			data_list = ug_dataset_alloc_list (dataset, data_class);
		*data_list = ug_data_list_assign (*data_list, src->data[index]);
	}
}

UgDataset*	ug_dataset_new (void)
{
	return ug_data_new (&dataset_class);
}

void	ug_dataset_ref   (UgDataset* dataset)
{
	dataset->ref_count++;
}

void	ug_dataset_unref (UgDataset* dataset)
{
	dataset->ref_count--;
	if (dataset->ref_count == 0)
		ug_data_free (dataset);
}

// Gets the element at the given position in a list.
gpointer	ug_dataset_get	(UgDataset* dataset, const UgDataClass* data_class, guint nth)
{
	UgDataList**	list;

	list = ug_dataset_get_list (dataset, data_class);
	if (list == NULL)
		return NULL;

	return ug_data_list_nth (*list, nth);
}

void	ug_dataset_remove (UgDataset* dataset, const UgDataClass* data_class, guint nth)
{
	UgDataList**	list;
	UgDataList*		link;

	list = ug_dataset_get_list (dataset, data_class);
	if (list == NULL || *list == NULL)
		return;

	if (nth == 0) {
		link  = *list;
		*list = link->next;
	}
	else
		link = ug_data_list_nth (*list, nth);

	if (link) {
		ug_data_list_unlink (link);
		ug_data_free (link);
	}
}

// If nth instance of data_class exist, return nth instance.
// If nth instance of data_class not exist, alloc new instance in tail and return it.
gpointer	ug_dataset_realloc (UgDataset* dataset, const UgDataClass* data_class, guint nth)
{
	UgDataList**	list;
	UgDataList*		link;

//	assert (data_class != NULL);

	list = ug_dataset_get_list (dataset, data_class);
	if (list == NULL)
		list = ug_dataset_alloc_list (dataset, data_class);

	if (*list == NULL) {
		*list = ug_data_new (data_class);
		return *list;
	}

	for (link = *list;  ;  link = link->next, nth--) {
		if (nth == 0)
			return link;
		if (link->next == NULL) {
			link->next = ug_data_new (data_class);
			link->next->prev = link;
			return link->next;
		}
	}
}

gpointer	ug_dataset_alloc_front (UgDataset* dataset, const UgDataClass* data_class)
{
	UgDataList**	list;
	UgDataList*		link;

//	assert (data_class != NULL);

	list = ug_dataset_get_list (dataset, data_class);
	if (list == NULL)
		list = ug_dataset_alloc_list (dataset, data_class);

	link = ug_data_new (data_class);
	*list = ug_data_list_prepend (*list, link);

	return link;
}

gpointer	ug_dataset_alloc_back  (UgDataset* dataset, const UgDataClass* data_class)
{
	UgDataList**	list;
	UgDataList*		link;

//	assert (data_class != NULL);

	list = ug_dataset_get_list (dataset, data_class);
	if (list == NULL)
		list = ug_dataset_alloc_list (dataset, data_class);

	link  = ug_data_new (data_class);
	*list = ug_data_list_append (*list, link);

	return link;
}


// ----------------------------------------------
// UgDataset list functions
guint	ug_dataset_list_length (UgDataset* dataset, const UgDataClass* data_class)
{
	UgDataList**	list;

	list = ug_dataset_get_list (dataset, data_class);
	return ug_data_list_length (*list);
}

UgDataList**	ug_dataset_alloc_list (UgDataset* dataset, const UgDataClass* data_class)
{
	guint	data_len = dataset->data_len;

	if (dataset->alloc_len == data_len) {
		dataset->alloc_len += 8 * 2;
		dataset->data = g_realloc (dataset->data, sizeof (gpointer) * dataset->alloc_len);
		dataset->key  = (const UgDataClass**) dataset->data + 1;
	}
	dataset->key[data_len]  = data_class;
	dataset->data[data_len] = NULL;
	dataset->data_len += 2;

	return &dataset->data[data_len];
}

UgDataList**	ug_dataset_get_list (UgDataset* dataset, const UgDataClass* data_class)
{
	guint	index;

	for (index = 0;  index < dataset->data_len;  index += 2) {
		if (dataset->key[index] == data_class)
			return &dataset->data[index];
	}

	return NULL;
}

// free old list in dataset and set list with new_list.
void	ug_dataset_set_list (UgDataset* dataset, const UgDataClass* data_class, gpointer new_list)
{
	UgDataList**	list;
	UgDataList*		old_list;

	list = ug_dataset_get_list (dataset, data_class);
	if (list == NULL)
		list = ug_dataset_alloc_list (dataset, data_class);

	old_list = *list;
	*list = new_list;
	ug_data_list_free (old_list);
}

// Cuts the element at the given position in a list.
gpointer	ug_dataset_cut_list (UgDataset* dataset, const UgDataClass* data_class, guint nth)
{
	UgDataList**	list;
	UgDataList*		link;

	list = ug_dataset_get_list (dataset, data_class);
	if (list == NULL)
		return NULL;

	if (nth == 0) {
		link = *list;
		*list = NULL;
	}
	else {
		// nth > 0
		link = ug_data_list_nth (*list, nth);
		if (link) {
			UG_DATA_LIST_CAST (link)->prev->next = NULL;
			UG_DATA_LIST_CAST (link)->prev = NULL;
		}
	}

	return link;
}

// ----------------------------------------------------------------------------
// UgMarkup parse/write
static void ug_dataset_parser_start_element (GMarkupParseContext*	context,
                                              const gchar*		element_name,
                                              const gchar**		attr_names,
                                              const gchar**		attr_values,
                                              UgDataset*		dataset,
                                              GError**			error)
{
	const UgDataClass*	data_class;
	UgDataList*			datalist;
	guint				index;

	if (strcmp (element_name, "DataClass") != 0) {
		g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
		return;
	}

	for (index=0; attr_names[index]; index++) {
		if (strcmp (attr_names[index], "name") != 0)
			continue;

		// find registered data class (UgDataClass)
		data_class = ug_data_class_find (attr_values[index]);
		if (data_class) {
			// Create new instance by UgDataClass and prepend it to list.
			datalist = ug_dataset_alloc_front (dataset, data_class);
			g_markup_parse_context_push (context, &ug_data_parser, datalist);
		}
		else {
			// Skip unregistered class, don't parse anything.
			g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
		}
		break;
	}
}

static GMarkupParser	ug_dataset_parser =
{
	(gpointer) ug_dataset_parser_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL,
	NULL,
	NULL
};

static void	ug_dataset_in_markup (UgDataset* dataset, GMarkupParseContext* context)
{
	g_markup_parse_context_push (context, &ug_dataset_parser, dataset);
}

static void	ug_dataset_to_markup (UgDataset* dataset, UgMarkup* markup)
{
	const UgDataClass*	data_class;
	UgDataList*			datalist;
	guint				index;

	for (index = 0;  index < dataset->data_len;  index += 2) {
		// output from tail to head
		datalist = ug_data_list_last (dataset->data[index]);
		for (;  datalist;  datalist = datalist->prev) {
			data_class = datalist->data_class;
			if (data_class->entry == NULL)
				continue;
			ug_markup_write_element_start (markup, "DataClass name='%s'", data_class->name);
			ug_data_to_markup ((UgData*)datalist, markup);
			ug_markup_write_element_end   (markup, "DataClass");
		}
	}
}

