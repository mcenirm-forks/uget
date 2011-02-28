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

#ifndef UG_DATA_H
#define UG_DATA_H

#include <glib.h>
#include <UgMarkup.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct	UgDataEntry			UgDataEntry;
typedef struct	UgDataInterface		UgDataInterface;
typedef struct	UgData				UgData;
typedef struct	UgDataList			UgDataList;

typedef enum	UgDataType			UgDataType;

// UgDataInterface
typedef void	(*UgInitFunc)		(gpointer instance);
typedef void	(*UgFinalizeFunc)	(gpointer instance);
typedef void	(*UgAssignFunc)		(gpointer instance, gpointer src);	// UgOverwriteFunc
typedef void	(*UgNotifyFunc)		(gpointer user_data);

// UgInMarkupFunc : how to parse data in markup.
// UgToMarkupFunc : how to write data to markup.
typedef void	(*UgInMarkupFunc)	(gpointer data, GMarkupParseContext* context);
typedef void	(*UgToMarkupFunc)	(gpointer data, UgMarkup* markup);

enum	UgDataType
{
	UG_DATA_NONE,
	UG_DATA_STRING,
	UG_DATA_INT,
	UG_DATA_UINT,
	UG_DATA_INT64,
	UG_DATA_DOUBLE,

	UG_DATA_INSTANCE,	// UgData-based pointer

	// User defined type.    (e.g. UserType  user_struct)
	// You must use it with UgInMarkupFunc & UgToMarkupFunc in UgDataEntry.
	UG_DATA_CUSTOM,
};


// ----------------------------------------------------------------------------
// UgDataEntry: defines a single XML element and it's offset of data structure.

//	typedef struct
//	{
//		gchar*		user;
//		gchar*		pass;
//	} Foo;
//
//	static UgDataEntry foo_tags[] =
//	{
//		{ "user",	G_STRUCT_OFFSET (Foo, user),	UG_DATA_STRING,	NULL,	NULL},
//		{ "pass",	G_STRUCT_OFFSET (Foo, pass),	UG_DATA_STRING,	NULL,	NULL},
//		{ NULL }
//	};
//
//	<user value='guest3' />
//	<pass value='unknown' />

struct UgDataEntry
{
	char*			name;			// tag name
	int				offset;
	UgDataType		type;

	UgInMarkupFunc	in_markup;		// how to parse  data in markup.
	UgToMarkupFunc	to_markup;		// how to output data to markup.
};


// ----------------------------------------------------------------------------
// UgDataInterface: All UgData-based structure must use it to save and load from XML file.

struct UgDataInterface
{
	unsigned int		instance_size;
	const char*			name;

	const UgDataEntry*	entry;		// To disable file parse/write, set entry = NULL.

	UgInitFunc			init;
	UgFinalizeFunc		finalize;

	UgAssignFunc		assign;		// overwrite
};

void		ug_data_interface_register	(const UgDataInterface*	iface);
void		ug_data_interface_unregister(const UgDataInterface*	iface);
const UgDataInterface*	ug_data_interface_find	(const gchar* name);


// ----------------------------------------------------------------------------
// UgData : UgData is a base structure.
//          It can save and load from XML file by UgDataEntry in UgDataInterface.

#define UG_DATA_MEMBERS				\
	const UgDataInterface*	iface

struct UgData
{
	UG_DATA_MEMBERS;
//	const UgDataInterface*	iface;
};

// ------------------------------------
// UgData*	ug_data_new		(const UgDataInterface* iface);
// void		ug_data_free	(UgData*	data);
gpointer	ug_data_new		(const UgDataInterface* iface);
void		ug_data_free	(gpointer	data);

// UgData*	ug_data_copy	(UgData*	data);
//void		ug_data_assign	(UgData*	data, UgData*	src);
gpointer	ug_data_copy	(gpointer	data);
void		ug_data_assign	(gpointer	data, gpointer	src);	// overwrite (or merge)

// ------------------------------------
// XML input and output
extern		GMarkupParser		ug_data_parser;		// UgData*   user_data
void		ug_data_in_markup	(UgData* data, GMarkupParseContext* context);
void		ug_data_to_markup	(UgData* data, UgMarkup* markup);


// ----------------------------------------------------------------------------
// UgDataList : UgDataList is a UgData structure include Doubly-Linked Lists.
//              All UgDataList-base structure can store in UgDataset.

//  UgData
//  |
//	`- UgDataList

#define UG_DATA_LIST_MEMBERS(Type)	\
	const UgDataInterface*	iface;	\
	Type*					next;	\
	Type*					prev

struct UgDataList
{
	UG_DATA_LIST_MEMBERS (UgDataList);
//	const UgDataInterface*	iface;
//	UgDataList*				next;
//	UgDataList*				prev;
};

// --- UgDataList functions are similar to GList functions.
//void		ug_data_list_free		(UgDataList* datalist);
void		ug_data_list_free		(gpointer datalist);

guint		ug_data_list_length		(gpointer datalist);
gpointer	ug_data_list_first		(gpointer datalist);
gpointer	ug_data_list_last		(gpointer datalist);
gpointer	ug_data_list_nth		(gpointer datalist, guint nth);

gpointer	ug_data_list_prepend	(gpointer datalist, gpointer datalink);
gpointer	ug_data_list_append		(gpointer datalist, gpointer datalink);
gpointer	ug_data_list_reverse	(gpointer datalist);

void		ug_data_list_unlink		(gpointer datalink);

// --- copy UgDataList to a new UgDataList if UgDataInterface::assign exist.
gpointer	ug_data_list_copy		(gpointer datalist);
gpointer	ug_data_list_assign		(gpointer datalist, gpointer src);


#ifdef __cplusplus
}
#endif

#endif  // UG_DATA_H

