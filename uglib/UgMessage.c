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


#include <memory.h>
// uglib
#include <UgMessage.h>

#include <glib/gi18n.h>


// ----------------------------------------------
// common message string & data

// UG_MESSAGE_INFO
static const gchar*	common_info[] =
{
	NULL,														// UG_MESSAGE_INFO_CUSTOM
	N_("Connecting..."),										// UG_MESSAGE_INFO_CONNECT
	N_("Transmitting..."),										// UG_MESSAGE_INFO_TRANSMIT,
	N_("Retry"),												// UG_MESSAGE_INFO_RETRY,
	N_("Download completed"),									// UG_MESSAGE_INFO_COMPLETE,
	N_("Finished"),												// UG_MESSAGE_INFO_FINISH,
	// resumable
	N_("Resumable"),											// UG_MESSAGE_INFO_RESUMABLE,
	N_("Not Resumable"),										// UG_MESSAGE_INFO_NOT_RESUMABLE,
};
static const guint	common_info_count = sizeof (common_info) / sizeof (gchar*);

// UG_MESSAGE_WARNING
static const gchar*	common_warning[] =
{
	NULL,														// UG_MESSAGE_WARNING_CUSTOM
	N_("Output file can't be renamed."),						// UG_MESSAGE_WARNING_FILE_RENAME_FAILED
};
static const guint	common_warning_count = sizeof (common_warning) / sizeof (gchar*);

// UG_MESSAGE_ERROR
static const gchar*	common_error[] =
{
	NULL,														// UG_MESSAGE_ERROR_CUSTOM
	N_("couldn't connect to host."),							// UG_MESSAGE_ERROR_CONNECT_FAILED
	N_("Folder can't be created."),								// UG_MESSAGE_ERROR_FOLDER_CREATE_FAILED
	N_("File can't be created (bad filename or file exist)."),	// UG_MESSAGE_ERROR_FILE_CREATE_FAILED
	N_("File can't be opened."),								// UG_MESSAGE_ERROR_FILE_OPEN_FAILED
	N_("Out of resource (disk full or run out of memory)."),	// UG_MESSAGE_ERROR_OUT_OF_RESOURCE
	N_("No output file."),										// UG_MESSAGE_ERROR_NO_OUTPUT_FILE
	N_("No output setting."),									// UG_MESSAGE_ERROR_NO_OUTPUT_SETTING
	N_("Too many retries."),									// UG_MESSAGE_ERROR_TOO_MANY_RETRIES
	N_("Unsupported scheme (protocol)."),						// UG_MESSAGE_ERROR_UNSUPPORTED_SCHEME
	N_("Unsupported file."),									// UG_MESSAGE_ERROR_UNSUPPORTED_FILE
};
static const guint	common_error_count = sizeof (common_error) / sizeof (gchar*);

// UG_MESSAGE_DATA
static guint	common_data[] =
{
	UG_DATA_STRING,	// UG_MESSAGE_DATA_FILE_CHANGED		// filename
	UG_DATA_STRING,	// UG_MESSAGE_DATA_URL_CHANGED		// URL
};
static guint	common_data_count = sizeof (common_data) / sizeof (guint);


// ----------------------------------------------
// HTTP message string
static const gchar*	http_error[] =
{
	NULL,														// UG_MESSAGE_ERROR_HTTP_MEMBER_CUSTOM
	N_("Too many redirections."),								// UG_MESSAGE_ERROR_HTTP_MEMBER_TOO_MANY_REDIRECTIONS
	N_("Failed to open posted file."),							// UG_MESSAGE_ERROR_HTTP_MEMBER_OPEN_POSTED_FILE_FAILED
};
static const guint	http_error_count = sizeof (http_error) / sizeof (gchar*);

// UG_MESSAGE_DATA
static guint	http_data[] =
{
	UG_DATA_STRING,	// UG_MESSAGE_DATA_HTTP_MEMBER_LOCATION == UG_MESSAGE_DATA_HTTP_MEMBER_REDIRECTION
	UG_DATA_STRING,	// UG_MESSAGE_DATA_HTTP_MEMBER_CONTENT_LOCATION
	UG_DATA_STRING,	// UG_MESSAGE_DATA_HTTP_MEMBER_CONTENT_DISPOSITION
};
static guint	http_data_count = sizeof (http_data) / sizeof (guint);


// ----------------------------------------------------------------------------
// UgMessage

static void	ug_message_finalize	(UgMessage* message);
static void	ug_message_assign	(UgMessage* message, UgMessage* src);

static const UgDataEntry	ug_message_entry[] =
{
	{"type",		G_STRUCT_OFFSET (UgMessage, type),		UG_DATA_UINT,		NULL,	NULL},
	{"code",		G_STRUCT_OFFSET (UgMessage, code),		UG_DATA_UINT,		NULL,	NULL},
	{"string",		G_STRUCT_OFFSET (UgMessage, string),	UG_DATA_STRING,		NULL,	NULL},
	{NULL},		// null-terminated
};
// extern
const UgDataInterface ug_message_iface =
{
	sizeof (UgMessage),		// instance_size
	"message",				// name
	ug_message_entry,		// entry

	(UgInitFunc)		NULL,
	(UgFinalizeFunc)	ug_message_finalize,
	(UgAssignFunc)		ug_message_assign,
};


static void	ug_message_finalize	(UgMessage* message)
{
	g_free (message->string);

	if (message->data_type == UG_DATA_STRING)
		g_free (message->data.v_string);
	else if (message->data_type == UG_DATA_INSTANCE)
		ug_data_free (message->data.v_instance);
}

static void	ug_message_assign (UgMessage* message, UgMessage* src)
{
	message->type = src->type;
	message->code = src->code;
	if (src->string)
		message->string = src->string;
	else
		message->string = NULL;

	message->data_type = src->data_type;
	if (src->data_type == UG_DATA_STRING)
		message->data.v_string = g_strdup (src->data.v_string);
	else if (src->data_type == UG_DATA_INSTANCE)
		message->data.v_instance = ug_data_copy (src->data.v_instance);
	else
		message->data = src->data;
}

UgMessage*	ug_message_new	(UgMessageType type)
{
	UgMessage*	message;

	message = ug_data_new (&ug_message_iface);
	message->type = type;
	return message;
}

UgMessage*	ug_message_new_state (gint state)
{
	UgMessage*	message;

	message = ug_message_new (UG_MESSAGE_STATE);
	message->data_type = UG_DATA_INT;
	message->data.v_int = state;
	return message;
}

UgMessage*	ug_message_new_progress	(void)
{
	return ug_message_new (UG_MESSAGE_PROGRESS);
}

UgMessage*	ug_message_new_info (guint info_code, const gchar* string)
{
	UgMessage*		message;
	guint			message_domain;
	guint			message_member;
	const gchar*	native_string;

	message = ug_data_new (&ug_message_iface);
	message->type = UG_MESSAGE_INFO;
	message->code = info_code;

	// use native or user string
	if (string) {
		message->string = g_strdup (string);
		return message;
	}

	// get native string by info_code
	native_string  = NULL;
	message_domain = UG_MESSAGE_CODE_DOMAIN (info_code);
	message_member = UG_MESSAGE_CODE_MEMBER (info_code);
	switch (message_domain) {
	case UG_MESSAGE_DOMAIN_COMMON:
		if (message_member < common_info_count)
			native_string = common_info [message_member];
		break;

	default:
		break;
	}
	if (native_string)
		message->string = g_strdup (gettext (native_string));

	return message;
}

UgMessage*	ug_message_new_warning (guint warning_code, const gchar* string)
{
	UgMessage*		message;
	guint			message_domain;
	guint			message_member;
	const gchar*	native_string;

	message = ug_data_new (&ug_message_iface);
	message->type = UG_MESSAGE_WARNING;
	message->code = warning_code;

	// use native or user string
	if (string) {
		message->string = g_strdup (string);
		return message;
	}

	// get native string by warning_code
	native_string  = NULL;
	message_domain = UG_MESSAGE_CODE_DOMAIN (warning_code);
	message_member = UG_MESSAGE_CODE_MEMBER (warning_code);
	switch (message_domain) {
	case UG_MESSAGE_DOMAIN_COMMON:
		if (message_member < common_warning_count)
			native_string = common_warning [message_member];
		break;

	default:
		break;
	}
	if (native_string)
		message->string = g_strdup (gettext (native_string));

	return message;
}

UgMessage*	ug_message_new_error (guint error_code, const gchar* string)
{
	UgMessage*		message;
	guint			message_domain;
	guint			message_member;
	const gchar*	native_string;

	message = ug_data_new (&ug_message_iface);
	message->type = UG_MESSAGE_ERROR;
	message->code = error_code;

	// use native or user string
	if (string) {
		message->string = g_strdup (string);
		return message;
	}

	// get native string by error_code
	native_string  = NULL;
	message_domain = UG_MESSAGE_CODE_DOMAIN (error_code);
	message_member = UG_MESSAGE_CODE_MEMBER (error_code);
	switch (message_domain) {
	case UG_MESSAGE_DOMAIN_COMMON:
		if (message_member < common_error_count)
			native_string = common_error [message_member];
		break;

	case UG_MESSAGE_DOMAIN_HTTP:
		if (message_member < http_error_count)
			native_string = http_error [message_member];
		break;

	default:
		break;
	}
	if (native_string)
		message->string = g_strdup (gettext (native_string));

	return message;
}

UgMessage*	ug_message_new_data	(guint data_code, ...)
{
	UgMessage*		message;
	guint			message_domain;
	guint			message_member;
	UgDataType		data_type;
	va_list			args;

	message = ug_data_new (&ug_message_iface);
	message->type = UG_MESSAGE_DATA;
	message->code = data_code;

	message_domain = UG_MESSAGE_CODE_DOMAIN (data_code);
	message_member = UG_MESSAGE_CODE_MEMBER (data_code);
	data_type      = UG_DATA_NONE;
	switch (message_domain) {
	case UG_MESSAGE_DOMAIN_COMMON:
		if (message_member < common_data_count)
			data_type = common_data [message_member];
		break;

	case UG_MESSAGE_DOMAIN_HTTP:
		if (message_member < http_data_count)
			data_type = http_data [message_member];
		break;

	default:
		break;
	}

	va_start (args, data_code);
	ug_message_set_data_v (message, data_type, args);
	va_end (args);

	return message;
}

void	ug_message_set_data   (UgMessage* message, UgDataType data_type, ...)
{
	va_list		args;

	va_start (args, data_type);
	ug_message_set_data_v (message, data_type, args);
	va_end (args);
}

void	ug_message_set_data_v  (UgMessage* message, UgDataType data_type, va_list args)
{
	switch (data_type) {
	case UG_DATA_STRING:
		message->data.v_string = g_strdup (va_arg (args, gchar*));
		break;

	case UG_DATA_INT:
		message->data.v_int  = va_arg (args, gint);
		break;

	case UG_DATA_UINT:
		message->data.v_uint  = va_arg (args, guint);
		break;

	case UG_DATA_INT64:
		message->data.v_int64  = va_arg (args, gint64);
		break;

	case UG_DATA_DOUBLE:
		message->data.v_double = va_arg (args, gdouble);
		break;

	case UG_DATA_INSTANCE:
		message->data.v_instance = ug_data_copy (va_arg (args, UgData*));
		break;

	default:
		break;
	}
}

