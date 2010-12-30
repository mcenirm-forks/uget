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

#include <string.h>
#include <UgData-download.h>
#include <UgetGtk-setting.h>

static void	ug_string_list_in_markup (GList** string_list, GMarkupParseContext* context);
static void	ug_string_list_to_markup (GList** string_list, UgMarkup* markup);


// ----------------------------------------------------------------------------
// UgDataEntry
//
// UgetGtkSetting
static UgDataEntry	uget_setting_data_entry[] =
{
	{"LaunchApp",		G_STRUCT_OFFSET (UgetGtkSetting, launch.active),	UG_DATA_TYPE_INT,		NULL,		NULL},
	{"LaunchAppTypes",	G_STRUCT_OFFSET (UgetGtkSetting, launch.types),		UG_DATA_TYPE_STRING,	NULL,		NULL},
	{"AutoSave",		G_STRUCT_OFFSET (UgetGtkSetting, auto_save.active),		UG_DATA_TYPE_INT,	NULL,		NULL},
	{"AutoSaveInterval",G_STRUCT_OFFSET (UgetGtkSetting, auto_save.interval),	UG_DATA_TYPE_INT,	NULL,		NULL},
	{"DownloadColumn",	G_STRUCT_OFFSET (UgetGtkSetting, download_column),	UG_DATA_TYPE_CUSTOM,	(UgInMarkupFunc) ug_data_in_markup,	(UgToMarkupFunc) ug_data_to_markup},
	{"Summary",			G_STRUCT_OFFSET (UgetGtkSetting, summary),			UG_DATA_TYPE_CUSTOM,	(UgInMarkupFunc) ug_data_in_markup,	(UgToMarkupFunc) ug_data_to_markup},
	{"Window",			G_STRUCT_OFFSET (UgetGtkSetting, window),			UG_DATA_TYPE_CUSTOM,	(UgInMarkupFunc) ug_data_in_markup,	(UgToMarkupFunc) ug_data_to_markup},
	{"UserInterface",	G_STRUCT_OFFSET (UgetGtkSetting, ui),				UG_DATA_TYPE_CUSTOM,	(UgInMarkupFunc) ug_data_in_markup,	(UgToMarkupFunc) ug_data_to_markup},
	{"Clipboard",		G_STRUCT_OFFSET (UgetGtkSetting, clipboard),		UG_DATA_TYPE_CUSTOM,	(UgInMarkupFunc) ug_data_in_markup,	(UgToMarkupFunc) ug_data_to_markup},
//	{"OfflineMode",		G_STRUCT_OFFSET (UgetGtkSetting, offline_mode),		UG_DATA_TYPE_INT,		NULL,		NULL},
//	{"Shutdown",		G_STRUCT_OFFSET (UgetGtkSetting, shutdown),			UG_DATA_TYPE_INT,		NULL,		NULL},
	{"CategoryDefault",	G_STRUCT_OFFSET (UgetGtkSetting, category),			UG_DATA_TYPE_CUSTOM,	(UgInMarkupFunc) ug_data_in_markup,	(UgToMarkupFunc) ug_data_to_markup},
	{"FolderList",		G_STRUCT_OFFSET (UgetGtkSetting, folder_list),		UG_DATA_TYPE_CUSTOM,	(UgInMarkupFunc) ug_string_list_in_markup,	(UgToMarkupFunc) ug_string_list_to_markup},
	{NULL},				// null-terminated
};
// UgDownloadColumnSetting
static UgDataEntry	download_column_data_entry[] =
{
	{"completed",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, completed),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{"total",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, total),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"percent",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, percent),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"elapsed",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, elapsed),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"left",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, left),			UG_DATA_TYPE_INT,	NULL,	NULL},
	{"speed",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, speed),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"retry",		G_STRUCT_OFFSET (struct UgDownloadColumnSetting, retry),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"category",	G_STRUCT_OFFSET (struct UgDownloadColumnSetting, category),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"URL",			G_STRUCT_OFFSET (struct UgDownloadColumnSetting, url),			UG_DATA_TYPE_INT,	NULL,	NULL},
	{NULL}			// null-terminated
};
// UgSummarySetting
static UgDataEntry	summary_setting_data_entry[] =
{
	{"name",		G_STRUCT_OFFSET (struct UgSummarySetting, name),			UG_DATA_TYPE_INT,	NULL,	NULL},
	{"folder",		G_STRUCT_OFFSET (struct UgSummarySetting, folder),			UG_DATA_TYPE_INT,	NULL,	NULL},
	{"category",	G_STRUCT_OFFSET (struct UgSummarySetting, category),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"URL",			G_STRUCT_OFFSET (struct UgSummarySetting, url),				UG_DATA_TYPE_INT,	NULL,	NULL},
	{"message",		G_STRUCT_OFFSET (struct UgSummarySetting, message),			UG_DATA_TYPE_INT,	NULL,	NULL},
	{NULL}			// null-terminated
};
// UgWindowSetting
static UgDataEntry	window_setting_data_entry[] =
{
	{"Toolbar",		G_STRUCT_OFFSET (struct UgWindowSetting, toolbar),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"Statusbar",	G_STRUCT_OFFSET (struct UgWindowSetting, statusbar),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{"Category",	G_STRUCT_OFFSET (struct UgWindowSetting, category),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"Summary",		G_STRUCT_OFFSET (struct UgWindowSetting, summary),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"x",			G_STRUCT_OFFSET (struct UgWindowSetting, x)	,			UG_DATA_TYPE_INT,	NULL,	NULL},
	{"y",			G_STRUCT_OFFSET (struct UgWindowSetting, y),			UG_DATA_TYPE_INT,	NULL,	NULL},
	{"width",		G_STRUCT_OFFSET (struct UgWindowSetting, width),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"height",		G_STRUCT_OFFSET (struct UgWindowSetting, height),		UG_DATA_TYPE_INT,	NULL,	NULL},
	{"maximized",	G_STRUCT_OFFSET (struct UgWindowSetting, maximized),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{NULL},			// null-terminated
};
// UgUserInterfaceSetting
static UgDataEntry	ui_setting_data_entry[] =
{
	{"CloseConfirmation",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting, close_confirmation),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{"CloseAction",			G_STRUCT_OFFSET (struct UgUserInterfaceSetting, close_action),			UG_DATA_TYPE_INT,	NULL,	NULL},
	{"DeleteConfirmation",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	delete_confirmation),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{"StartInTray",			G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	start_in_tray),			UG_DATA_TYPE_INT,	NULL,	NULL},
	{"StartInOfflineMode",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	start_in_offline_mode),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{"StartNotification",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	start_notification),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{"SoundNotification",	G_STRUCT_OFFSET (struct UgUserInterfaceSetting,	sound_notification),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{NULL},			// null-terminated
};
// UgClipboardSetting
static UgDataEntry	clipboard_setting_data_entry[] =
{
	{"pattern",		G_STRUCT_OFFSET (struct UgClipboardSetting, pattern),	UG_DATA_TYPE_STRING,	NULL,	NULL},
	{"monitor",		G_STRUCT_OFFSET (struct UgClipboardSetting, monitor),	UG_DATA_TYPE_INT,		NULL,	NULL},
	{"quiet",		G_STRUCT_OFFSET (struct UgClipboardSetting, quiet),		UG_DATA_TYPE_INT,		NULL,	NULL},
	{"NthCategory",	G_STRUCT_OFFSET (struct UgClipboardSetting, nth_category),	UG_DATA_TYPE_INT,	NULL,	NULL},
	{NULL},			// null-terminated
};


// ----------------------------------------------------------------------------
// UgDataClass
//
// UgetGtkSetting
static UgDataClass	uget_setting_data_class =
{
	"UgetGtkSetting",			// name
	NULL,						// reserve
	sizeof (UgetGtkSetting),	// instance_size
	uget_setting_data_entry,	// entry
	NULL, NULL, NULL,
};
// UgDownloadColumnSetting
static UgDataClass	download_column_data_class =
{
	"DownloadColumnSetting",
	NULL,
	sizeof (struct UgDownloadColumnSetting),
	download_column_data_entry,
	NULL, NULL, NULL,
};
// UgSummarySetting
static UgDataClass	summary_setting_data_class =
{
	"SummarySetting",
	NULL,
	sizeof (struct UgSummarySetting),
	summary_setting_data_entry,
	NULL, NULL, NULL,
};
// UgWindowSetting
static UgDataClass	window_setting_data_class =
{
	"WindowSetting",
	NULL,
	sizeof (struct UgWindowSetting),
	window_setting_data_entry,
	NULL, NULL, NULL,
};
// UgUserInterfaceSetting
static UgDataClass	ui_setting_data_class =
{
	"UserInterfaceSetting",
	NULL,
	sizeof (struct UgUserInterfaceSetting),
	ui_setting_data_entry,
	NULL, NULL, NULL,
};
// UgClipboardSetting
static UgDataClass	clipboard_setting_data_class =
{
	"ClipboardSetting",
	NULL,
	sizeof (struct UgClipboardSetting),
	clipboard_setting_data_entry,
	NULL, NULL, NULL,
};


// ----------------------------------------------------------------------------
// "FolderList" UgMarkup functions
//
static void ug_string_list_start_element (GMarkupParseContext*	context,
                                          const gchar*		element_name,
                                          const gchar**		attr_names,
                                          const gchar**		attr_values,
                                          GList**			string_list,
                                          GError**			error)
{
	guint	index;

	for (index=0; attr_names[index]; index++) {
		if (strcmp (attr_names[index], "value") == 0)
			*string_list = g_list_prepend (*string_list, g_strdup (attr_values[index]));
	}

	// skip end_element() one times.
	g_markup_parse_context_push (context, &ug_markup_skip_parser, NULL);
}

// GList**  user_data
static GMarkupParser	ug_string_list_parser =
{
	(gpointer) ug_string_list_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL, NULL, NULL
};

static void	ug_string_list_in_markup (GList** string_list, GMarkupParseContext* context)
{
	g_markup_parse_context_push (context, &ug_string_list_parser, string_list);
}

static void	ug_string_list_to_markup (GList** string_list, UgMarkup* markup)
{
	GList*	link;

	for (link = g_list_last (*string_list);  link;  link = link->prev) {
		ug_markup_write_element_start	(markup, "string value='%s'", link->data);
		ug_markup_write_element_end	(markup, "string");
	}
}

// ----------------------------------------------------------------------------
// "UgetGtkSetting" UgMarkup functions
//
static void uget_setting_start_element (GMarkupParseContext*	context,
                                        const gchar*			element_name,
                                        const gchar**			attr_names,
                                        const gchar**			attr_values,
                                        UgetGtkSetting*			setting,
                                        GError**				error)
{
	guint	index;

	if (strcmp (element_name, "UgetGtkSetting") != 0) {
		g_set_error (error, G_MARKUP_ERROR,
				G_MARKUP_ERROR_UNKNOWN_ELEMENT, "Unknown element");
		return;
	}

	for (index=0; attr_names[index]; index++) {
		if (strcmp (attr_names[index], "version") != 0)
			continue;
		if (strcmp (attr_values[index], "1") == 0) {
			g_markup_parse_context_push (context,
					&ug_data_parser, setting);
			return;
		}
	}

	g_set_error (error, G_MARKUP_ERROR,
			G_MARKUP_ERROR_UNKNOWN_ELEMENT, "Unknown element");
}

// UgetGtkSetting*  user_data
static GMarkupParser	uget_gtk_setting_parser =
{
	(gpointer) uget_setting_start_element,
	(gpointer) g_markup_parse_context_pop,
	NULL, NULL, NULL
};

// ----------------------------------------------------------------------------
// "UgetGtkSetting" functions
//
void	uget_gtk_setting_init (UgetGtkSetting* setting)
{
	setting->data_class = &uget_setting_data_class;

	// "SummarySetting"
	setting->summary.data_class = &summary_setting_data_class;
	// "DownloadColumnSetting"
	setting->download_column.data_class = &download_column_data_class;
	setting->download_column.changed_count = 1;
	// "WindowSetting"
	setting->window.data_class = &window_setting_data_class;
	// "UserInterfaceSetting"
	setting->ui.data_class = &ui_setting_data_class;
	// "ClipboardSetting"
	setting->clipboard.data_class = &clipboard_setting_data_class;
	// "CategoryDefault"
	setting->category.data_class = UgCategoryClass;
	ug_category_init (&setting->category);
	// "FolderList"
	setting->folder_list = NULL;
}

void	uget_gtk_setting_reset (UgetGtkSetting* setting)
{
	UgDataCommon*	common;

	// "SummarySetting"
	setting->summary.name     = TRUE;
	setting->summary.folder   = TRUE;
	setting->summary.category = FALSE;
	setting->summary.url      = FALSE;
	setting->summary.message  = TRUE;

	// "DownloadColumnSetting"
	setting->download_column.completed = TRUE;
	setting->download_column.total     = TRUE;
	setting->download_column.percent   = TRUE;
	setting->download_column.elapsed   = TRUE;
	setting->download_column.left      = TRUE;
	setting->download_column.speed     = TRUE;
	setting->download_column.retry     = TRUE;
	setting->download_column.category  = TRUE;
	setting->download_column.url       = TRUE;

	// "WindowSetting"
	setting->window.toolbar   = TRUE;
	setting->window.statusbar = TRUE;
	setting->window.category  = TRUE;
	setting->window.summary   = TRUE;
	setting->window.x = 0;
	setting->window.y = 0;
	setting->window.width = 0;
	setting->window.height = 0;
	setting->window.maximized = FALSE;

	// "UserInterfaceSetting"
	setting->ui.close_confirmation = TRUE;
	setting->ui.close_action = 0;
	setting->ui.delete_confirmation = TRUE;
	setting->ui.start_in_tray = FALSE;
	setting->ui.start_in_offline_mode = FALSE;
	setting->ui.start_notification = TRUE;
	setting->ui.sound_notification = TRUE;

	// "ClipboardSetting"
	g_free (setting->clipboard.pattern);
	setting->clipboard.pattern = g_strdup (UGET_GTK_CLIPBOARD_PATTERN);
	setting->clipboard.monitor = TRUE;
	setting->clipboard.quiet = FALSE;
	setting->clipboard.nth_category = 0;

	setting->offline_mode = FALSE;
	setting->shutdown = 0;

	// "CategoryDefault"
	common = ug_dataset_realloc (setting->category.defaults,
			UgDataCommonClass, 0);
	g_free (common->folder);
	common->folder = g_strdup (g_get_home_dir ());

	// "FolderList"
	g_list_foreach (setting->folder_list, (GFunc) g_free, NULL);
	g_list_free (setting->folder_list);
	setting->folder_list = NULL;

	// Others
	setting->launch.active = TRUE;
	setting->launch.types = g_strdup (UGET_GTK_LAUNCH_APP_TYPES);
	setting->auto_save.active = TRUE;
	setting->auto_save.interval = 3;
}

gboolean	uget_gtk_setting_save (UgetGtkSetting* setting, const gchar* file)
{
	UgMarkup*	markup;

	markup = ug_markup_new ();
	if (ug_markup_write_start (markup, file, TRUE)) {
		ug_markup_write_element_start	(markup, "UgetGtkSetting version='1'");
		ug_data_to_markup ((UgData*) setting, markup);
		ug_markup_write_element_end	(markup, "UgetGtkSetting");
		ug_markup_write_end (markup);
		return TRUE;
	}
	return FALSE;
}

gboolean	uget_gtk_setting_load (UgetGtkSetting* setting, const gchar* file)
{
	return ug_markup_parse (file, &uget_gtk_setting_parser, setting);
}

