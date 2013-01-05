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

#include <memory.h>
#include <string.h>
// uglib
#include <UgUtils.h>
#include <UgString.h>
#include <UgOption.h>
#include <UgData-download.h>

static const	UgOptionEntry	ug_option_main_entry;	// defined below
static GList*	ug_arg_get_rest (GPtrArray* args);		// used by ug_option_parse()

void	ug_option_init  (UgOption* uopt)
{
	uopt->context = g_option_context_new ("[URL 1] [URL 2] ... [URL n]");
	uopt->group   = g_option_group_new (NULL, NULL, NULL, NULL, NULL);
	uopt->list    = g_list_prepend (NULL, (gpointer) &ug_option_main_entry);
	uopt->data    = ug_option_main_entry.data;

	g_option_group_add_entries (uopt->group, ug_option_main_entry.entry);
	g_option_context_set_main_group (uopt->context, uopt->group);
}

void	ug_option_add (UgOption* uopt, const UgOptionEntry* oentry, GOptionGroup* group)
{
	if (group == NULL)
		group = uopt->group;
	else if (group != uopt->group)
		g_option_context_add_group (uopt->context, group);

	if (oentry) {
		if (g_list_find (uopt->list, oentry))
			return;
		uopt->list = g_list_prepend (uopt->list, (gpointer) oentry);
		g_option_group_add_entries (group, oentry->entry);
	}
}

void	ug_option_help  (UgOption* uopt, const char* progname, const char* help_option)
{
	gint		argc;
	gchar*		args[2];
	gchar**		argv;

	if (help_option == NULL)
		help_option = "-h";
	args[0] = (gchar*) progname;
	args[1] = (gchar*) help_option;
	argv = args;
	argc = 2;

	g_option_context_set_help_enabled (uopt->context, TRUE);
	g_option_context_parse (uopt->context, &argc, &argv, NULL);
}

GList*	ug_option_parse (UgOption* uopt, GPtrArray* args)
{
	GPtrArray*		temp_args;
	UgDataset*		dataset;
	UgDataCommon*	common;
	GList*			downloads;
	GList*			strings;

	// setup
	g_option_context_set_help_enabled (uopt->context, FALSE);
	g_option_context_set_ignore_unknown_options (uopt->context, TRUE);
	// clear argument data
	g_list_foreach (uopt->list, (GFunc) ug_option_entry_clear, NULL);

	temp_args = ug_arg_copy (args, FALSE);
	g_option_context_parse (uopt->context,
	                        (gint*)    &temp_args->len,
	                        (gchar***) &temp_args->pdata,
	                        NULL);
	// get URLs from file or command-line
	if (uopt->data->input_file)
		strings = ug_text_file_get_uris (uopt->data->input_file, NULL);
	else
		strings = NULL;
	strings = g_list_concat (strings, ug_arg_get_rest (temp_args));
	ug_arg_free (temp_args, FALSE);

	// get argument data and create tasks
	downloads = NULL;
	for (strings = g_list_last (strings);  strings;  strings = strings->prev) {
		dataset = ug_dataset_new ();
		common = ug_dataset_alloc_front (dataset, UG_DATA_COMMON_I);
		common->url = strings->data;
		common->keeping.url = TRUE;		// keep common->url no change
		g_list_foreach (uopt->list, (GFunc) ug_option_entry_get, dataset);
		downloads = g_list_prepend (downloads, dataset);
		// if previous link is first one...
		if (strings->prev == NULL)
			break;
	}
	// free string list
	g_list_free (strings);

	return downloads;
}

const UgOptionEntry*	ug_option_get_main_entry (void)
{
	return &ug_option_main_entry;
}


// ----------------------------------------------------------------------------
// UgOptionEntry
//
void ug_option_entry_get (const UgOptionEntry* oentry, UgDataset* dataset)
{
	oentry->get (oentry, dataset);
}

void ug_option_entry_clear (const UgOptionEntry* oentry)
{
	memset (oentry->data, 0, oentry->data_size);
	if (oentry->init)
		oentry->init (oentry->data);
}


// ---------------------------------------------------------------------------
// UgOptionMainData
//
static UgOptionMainData		option_main_data;
static GOptionEntry			option_main_entry[] =
{
	{"version",		  'V',	0,	G_OPTION_ARG_NONE,		&option_main_data.version,			"display the version of uGet and exit.",		NULL},
	{"set-offline",		0,	0,	G_OPTION_ARG_INT,		&option_main_data.offline,			"set offline mode to N. (0=Disable)",			"N"},

	{"quiet",			0,	0,	G_OPTION_ARG_NONE,		&option_main_data.quiet,			"add download directly. Don't show dialog.",	NULL},
	{"category-index",	0,	0,	G_OPTION_ARG_INT,		&option_main_data.category_index,	"add download to Nth category. (default -1)",	"N"},
	{"input-file",	  'i',	0,	G_OPTION_ARG_STRING,	&option_main_data.input_file,		"add URLs found in FILE.",						"FILE"},

	{"folder",			0,	0,	G_OPTION_ARG_STRING,	&option_main_data.folder,			"placed download file in FOLDER.",		"FOLDER"},
	{"filename",		0,	0,	G_OPTION_ARG_STRING,	&option_main_data.file,				"set download filename to FILE.",		"FILE"},

	{"user",			0,	0,	G_OPTION_ARG_STRING,	&option_main_data.user,				"set both ftp and http user to USER.",		"USER"},
	{"password",		0,	0,	G_OPTION_ARG_STRING,	&option_main_data.password,			"set both ftp and http password to PASS.",	"PASS"},

	{"proxy-type",		0,	0,	G_OPTION_ARG_INT,		&option_main_data.proxy.type,		"set proxy type to N. (0=Don't use)",	"N"},
	{"proxy-host",		0,	0,	G_OPTION_ARG_STRING,	&option_main_data.proxy.host,		"set proxy host to HOST.",				"HOST"},
	{"proxy-port",		0,	0,	G_OPTION_ARG_INT,		&option_main_data.proxy.port,		"set proxy port to PORT.",				"PORT"},
	{"proxy-user",		0,	0,	G_OPTION_ARG_STRING,	&option_main_data.proxy.user,		"set USER as proxy username.",			"USER"},
	{"proxy-password",	0,	0,	G_OPTION_ARG_STRING,	&option_main_data.proxy.password,	"set PASS as proxy password.",			"PASS"},

	{"http-user",		0,	0,	G_OPTION_ARG_STRING,	&option_main_data.http.user,		"set http user to USER.",							"USER"},
	{"http-password",	0,	0,	G_OPTION_ARG_STRING,	&option_main_data.http.password,	"set http password to PASS.",						"PASS"},
	{"http-referer",	0,	0,	G_OPTION_ARG_STRING,	&option_main_data.http.referrer,	"include `Referer: URL' header in HTTP request.",	"URL"},
	{"http-user-agent",	0,	0,	G_OPTION_ARG_STRING,	&option_main_data.http.user_agent,	"identify as AGENT instead of default.",			"AGENT"},
	{"http-cookie-data",0,	0,	G_OPTION_ARG_STRING,	&option_main_data.http.cookie_data,	"load cookies from STRING.",						"STRING"},
	{"http-cookie-file",0,	0,	G_OPTION_ARG_STRING,	&option_main_data.http.cookie_file,	"load cookies from FILE.",							"FILE"},
	{"http-post-data",	0,	0,	G_OPTION_ARG_STRING,	&option_main_data.http.post_data,	"use the POST method; send STRING as the data.",	"STRING"},
	{"http-post-file",	0,	0,	G_OPTION_ARG_STRING,	&option_main_data.http.post_file,	"use the POST method; send contents of FILE",		"FILE"},

	{"ftp-user",		0,	0,	G_OPTION_ARG_STRING,	&option_main_data.ftp.user,			"set ftp user to USER.",				"USER"},
	{"ftp-password",	0,	0,	G_OPTION_ARG_STRING,	&option_main_data.ftp.password,		"set ftp password to PASS.",			"PASS"},

	{NULL}
};

static void		ug_option_main_entry_init (UgOptionMainData* option_data);
static gboolean	ug_option_main_entry_get  (UgOptionEntry* oentry, UgDataset* dataset);

static const	UgOptionEntry	ug_option_main_entry =
{
	"main",
	NULL,
	option_main_entry,
	&option_main_data,
	sizeof (option_main_data),

	(UgInitFunc)			ug_option_main_entry_init,
	(UgOptionEntryGet)		ug_option_main_entry_get,
};

static void	ug_option_main_entry_init	(UgOptionMainData* option_data)
{
	option_data->offline = -1;
	option_data->category_index = -1;
}

static gboolean	ug_option_main_entry_get (UgOptionEntry* oentry, UgDataset* dataset)
{
	UgOptionMainData*	option_data;
	gboolean		retval = FALSE;
	UgDataCommon*	common;
	UgDataProxy*	proxy;
	UgDataHttp*		http;
	UgDataFtp*		ftp;

	option_data = oentry->data;
	// common
	if (option_data->folder) {
		common = ug_dataset_realloc (dataset, UG_DATA_COMMON_I, 0);
		g_free (common->folder);
		common->folder = g_strdup (option_data->folder);
		common->keeping.folder = TRUE;
		retval = TRUE;
	}

	if (option_data->file) {
		common = ug_dataset_realloc (dataset, UG_DATA_COMMON_I, 0);
		g_free (common->file);
		common->file = g_strdup (option_data->file);
		common->keeping.file = TRUE;
		retval = TRUE;
	}

	if (option_data->user) {
		common = ug_dataset_realloc (dataset, UG_DATA_COMMON_I, 0);
		g_free (common->user);
		common->user = g_strdup (option_data->user);
		common->keeping.user = TRUE;
		retval = TRUE;
	}

	if (option_data->password) {
		common = ug_dataset_realloc (dataset, UG_DATA_COMMON_I, 0);
		g_free (common->password);
		common->password = g_strdup (option_data->password);
		common->keeping.password = TRUE;
		retval = TRUE;
	}

	// proxy
	if (option_data->proxy.type) {
		proxy = ug_dataset_realloc (dataset, UG_DATA_PROXY_I, 0);
		proxy->type = option_data->proxy.type;
		proxy->keeping.type = TRUE;
		retval = TRUE;
	}

	if (option_data->proxy.host) {
		proxy = ug_dataset_realloc (dataset, UG_DATA_PROXY_I, 0);
		g_free (proxy->host);
		proxy->host = g_strdup (option_data->proxy.host);
		proxy->keeping.host = TRUE;
		retval = TRUE;
	}

	if (option_data->proxy.port) {
		proxy = ug_dataset_realloc (dataset, UG_DATA_PROXY_I, 0);
		proxy->port = option_data->proxy.port;
		proxy->keeping.port = TRUE;
		retval = TRUE;
	}

	if (option_data->proxy.user) {
		proxy = ug_dataset_realloc (dataset, UG_DATA_PROXY_I, 0);
		g_free (proxy->user);
		proxy->user = g_strdup (option_data->proxy.user);
		proxy->keeping.user = TRUE;
		retval = TRUE;
	}

	if (option_data->proxy.password) {
		proxy = ug_dataset_realloc (dataset, UG_DATA_PROXY_I, 0);
		g_free (proxy->password);
		proxy->password = g_strdup (option_data->proxy.password);
		proxy->keeping.password = TRUE;
		retval = TRUE;
	}

	// http
	if (option_data->http.user) {
		http = ug_dataset_realloc (dataset, UG_DATA_HTTP_I, 0);
		g_free (http->user);
		http->user = g_strdup (option_data->http.user);
		http->keeping.user = TRUE;
		retval = TRUE;
	}

	if (option_data->http.password) {
		http = ug_dataset_realloc (dataset, UG_DATA_HTTP_I, 0);
		g_free (http->password);
		http->password = g_strdup (option_data->http.password);
		http->keeping.password = TRUE;
		retval = TRUE;
	}

	if (option_data->http.referrer) {
		http = ug_dataset_realloc (dataset, UG_DATA_HTTP_I, 0);
		g_free (http->referrer);
		http->referrer = g_strdup (option_data->http.referrer);
		http->keeping.referrer = TRUE;
		retval = TRUE;
	}

	if (option_data->http.user_agent) {
		http = ug_dataset_realloc (dataset, UG_DATA_HTTP_I, 0);
		g_free (http->user_agent);
		http->user_agent = g_strdup (option_data->http.user_agent);
		http->keeping.user_agent = TRUE;
		retval = TRUE;
	}

	if (option_data->http.cookie_data) {
		http = ug_dataset_realloc (dataset, UG_DATA_HTTP_I, 0);
		g_free (http->cookie_data);
		http->cookie_data = g_strdup (option_data->http.cookie_data);
		http->keeping.cookie_data = TRUE;
		retval = TRUE;
	}

	if (option_data->http.cookie_file) {
		http = ug_dataset_realloc (dataset, UG_DATA_HTTP_I, 0);
		g_free (http->cookie_file);
		http->cookie_file = g_strdup (option_data->http.cookie_file);
		http->keeping.cookie_file = TRUE;
		retval = TRUE;
	}

	if (option_data->http.post_data) {
		http = ug_dataset_realloc (dataset, UG_DATA_HTTP_I, 0);
		g_free (http->post_data);
		http->post_data = g_strdup (option_data->http.post_data);
		http->keeping.post_data = TRUE;
		retval = TRUE;
	}

	if (option_data->http.post_file) {
		http = ug_dataset_realloc (dataset, UG_DATA_HTTP_I, 0);
		g_free (http->post_file);
		http->post_file = g_strdup (option_data->http.post_file);
		http->keeping.post_file = TRUE;
		retval = TRUE;
	}

	// ftp
	if (option_data->ftp.user) {
		ftp = ug_dataset_realloc (dataset, UG_DATA_FTP_I, 0);
		g_free (ftp->user);
		ftp->user = g_strdup (option_data->ftp.user);
		ftp->keeping.user = TRUE;
		retval = TRUE;
	}

	if (option_data->ftp.password) {
		ftp = ug_dataset_realloc (dataset, UG_DATA_FTP_I, 0);
		g_free (ftp->password);
		ftp->password = g_strdup (option_data->ftp.password);
		ftp->keeping.password = TRUE;
		retval = TRUE;
	}

	return retval;
}

// ------------------------------------
// argument functions, free or copy GPtrArray from ug_ipc_peek() and ug_ipc_pop()
//
GPtrArray*	ug_arg_new (int argc, char** argv, gboolean copy_strings)
{
	GPtrArray*	args;
	gchar**		args_cur;
	gchar**		argv_end;

	args = g_ptr_array_sized_new (argc);
	args->len = argc;
	args_cur = (gchar**) args->pdata;
	argv_end = argv + argc;

	if (copy_strings) {
		while (argv < argv_end)
			*args_cur++ = g_strdup (*argv++);
	}
	else {
		while (argv < argv_end)
			*args_cur++ = *argv++;
	}
	return args;
}

void	ug_arg_free (GPtrArray* args, gboolean free_strings)
{
	if (free_strings)
		g_ptr_array_foreach (args, (GFunc)g_free, NULL);
	g_ptr_array_free (args, TRUE);
}

GPtrArray*	ug_arg_copy (GPtrArray* args, gboolean copy_strings)
{
	return ug_arg_new (args->len, (char**)args->pdata, copy_strings);
}

// find -?, -h, --help, or --help- in command-line options and return it.
char*	ug_arg_find_help (int argc, char** argv)
{
	char*	arg;
	int		arg_len;

	for (argc -= 1;  argc >= 0;  argc--) {
		arg = argv[argc];
		arg_len = strlen (arg);
#ifdef _WIN32
		// Check and remove some character (space,0x20) in tail of argument from command line.
		// This problem only happen in Windows platform.
		ug_str_clear_tail_charset (arg, arg_len, " \n");
#endif
		// check short_name: -h or -?
		if (arg_len < 2 || arg[0] != '-')
			continue;
		if (arg_len == 2 && (arg[1] == 'h' || arg[1] == '?'))
			return arg;
		// check long name: --help
		if (arg_len < 6 || arg[1] != '-')
			continue;
		if (strncmp (arg+2, "help", 4) == 0) {
			if (arg_len == 6 || arg[6] == '-')
				return arg;
		}
	}
	return NULL;
}

// find -V, --version
char*	ug_arg_find_version (int argc, char** argv)
{
	char*	arg;
	int		arg_len;

	for (argc -= 1;  argc >= 0;  argc--) {
		arg = argv[argc];
		arg_len = strlen (arg);
#ifdef _WIN32
		// Check and remove some character (space,0x20) in tail of argument from command line.
		// This problem only happen in Windows platform.
		ug_str_clear_tail_charset (arg, arg_len, " \n");
#endif
		// check short_name: -V
		if (arg_len < 2 || arg[0] != '-')
			continue;
		if (arg_len == 2 && arg[1] == 'V')
			return arg;
		// check long name: --version
		if (arg_len != 9 || arg[1] != '-')
			continue;
		if (strncmp (arg+2, "version", 7) == 0)
			return arg;
	}
	return NULL;
}

// used by ug_option_parse()
static GList*	ug_arg_get_rest (GPtrArray* args)
{
	GList*		strings;
	gchar**		cur;
	gchar**		end;

	strings = NULL;
	cur  = (gchar**) args->pdata;
	end  = cur + args->len;
//        skip argv[0]
	for (cur = cur + 1;  cur < end;  cur++) {
		if ((*cur)[0] == '-')
			continue;
		strings = g_list_prepend (strings, g_strdup (*cur));
	}
	return g_list_reverse (strings);
}

