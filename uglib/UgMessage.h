/*
 *
 *   Copyright (C) 2005-2011 by plushuang
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

#ifndef UG_MESSAGE_H
#define UG_MESSAGE_H

#include <stdarg.h>
#include <UgData.h>

#ifdef __cplusplus
extern "C" {
#endif


// message code (32bit) = domain (16 bit) + member (16bit)
#define UG_MESSAGE_CODE_DOMAIN(code)			( ((guint32)(code) & 0xFFFF0000) >> 16 )
#define UG_MESSAGE_CODE_MEMBER(code)			( ((guint32)(code) & 0x0000FFFF) )
#define	UG_MESSAGE_CODE_MAKE(domain, member)	( ((guint32)(domain) << 16) | (guint32)(member) )
// interface address for UgDataset
#define	UG_MESSAGE_I		&ug_message_iface

typedef struct	UgMessage				UgMessage;
typedef enum	UgMessageType			UgMessageType;

extern	const	UgDataInterface			ug_message_iface;

enum UgMessageType
{
	UG_MESSAGE_NONE,
	UG_MESSAGE_STATE,		// v_int (UgState)
	UG_MESSAGE_PROGRESS,
	UG_MESSAGE_INFO,		// message string
	UG_MESSAGE_WARNING,		// message string
	UG_MESSAGE_ERROR,		// message string
	UG_MESSAGE_DATA,
};

enum UgMessageDomain
{
	UG_MESSAGE_DOMAIN_COMMON = 0,		// must be 0
	UG_MESSAGE_DOMAIN_HTTP,
	UG_MESSAGE_DOMAIN_FTP,

	UG_MESSAGE_DOMAIN_USER = 1000,
};

// ----------------------------------------------------------------------------
// domain == UG_MESSAGE_DOMAIN_COMMON
enum UgMessageInfoCode
{
	UG_MESSAGE_INFO_CUSTOM = 0,			// must be 0
	UG_MESSAGE_INFO_CONNECT,
	UG_MESSAGE_INFO_TRANSMIT,
	UG_MESSAGE_INFO_RETRY,
	UG_MESSAGE_INFO_COMPLETE,	// download completed
	UG_MESSAGE_INFO_FINISH,		// completed, it will not be used in future.
	// resumable
	UG_MESSAGE_INFO_RESUMABLE,
	UG_MESSAGE_INFO_NOT_RESUMABLE,
};

enum UgMessageWarningCode
{
	UG_MESSAGE_WARNING_CUSTOM = 0,		// must be 0
	UG_MESSAGE_WARNING_FILE_RENAME_FAILED,	// UG_MESSAGE_WARNING_FAILED_TO_RENAME_FILE
};

enum UgMessageErrorCode
{
	UG_MESSAGE_ERROR_CUSTOM = 0,		// must be 0
	UG_MESSAGE_ERROR_CONNECT_FAILED,
	UG_MESSAGE_ERROR_FOLDER_CREATE_FAILED,	// UG_MESSAGE_ERROR_FAILED_TO_CREATE_FOLDER
	UG_MESSAGE_ERROR_FILE_CREATE_FAILED,	// UG_MESSAGE_ERROR_FAILED_TO_CREATE_FILE
	UG_MESSAGE_ERROR_FILE_OPEN_FAILED,		// UG_MESSAGE_ERROR_FAILED_TO_OPEN_FILE
	UG_MESSAGE_ERROR_OUT_OF_RESOURCE,	// disk full or out of memory
	UG_MESSAGE_ERROR_NO_OUTPUT_FILE,
	UG_MESSAGE_ERROR_NO_OUTPUT_SETTING,
	UG_MESSAGE_ERROR_TOO_MANY_RETRIES,
	UG_MESSAGE_ERROR_UNSUPPORTED_SCHEME,
	UG_MESSAGE_ERROR_UNSUPPORTED_PROTOCOL = UG_MESSAGE_ERROR_UNSUPPORTED_SCHEME,
	UG_MESSAGE_ERROR_UNSUPPORTED_FILE,
};

enum UgMessageDataCode
{
	UG_MESSAGE_DATA_FILE_CHANGED,		// v_string (filename)
	UG_MESSAGE_DATA_URL_CHANGED,		// v_string (URL)
};

// ----------------------------------------------------------------------------
// domain == UG_MESSAGE_DOMAIN_HTTP
enum UgMessageDataHttpCodeMember
{
	UG_MESSAGE_DATA_HTTP_MEMBER_LOCATION,			// v_string (value of HTTP header "Location:")
	UG_MESSAGE_DATA_HTTP_MEMBER_REDIRECTION	= UG_MESSAGE_DATA_HTTP_MEMBER_LOCATION,
	UG_MESSAGE_DATA_HTTP_MEMBER_CONTENT_LOCATION,	// v_string (value of HTTP header "Content-Location:")
	UG_MESSAGE_DATA_HTTP_MEMBER_CONTENT_DISPOSITION,// v_string (value of HTTP header "Content-disposition:" or "Content-Disposition:")
};

enum UgMessageErrorHttpCodeMember
{
	UG_MESSAGE_ERROR_HTTP_MEMBER_CUSTOM = 0,		// must be 0
	UG_MESSAGE_ERROR_HTTP_MEMBER_TOO_MANY_REDIRECTIONS,
	UG_MESSAGE_ERROR_HTTP_MEMBER_OPEN_POSTED_FILE_FAILED,
};

// HTTP data message
#define UG_MESSAGE_DATA_HTTP_LOCATION				UG_MESSAGE_CODE_MAKE (UG_MESSAGE_DOMAIN_HTTP,	UG_MESSAGE_DATA_HTTP_MEMBER_LOCATION)
#define UG_MESSAGE_DATA_HTTP_REDIRECTION			UG_MESSAGE_CODE_MAKE (UG_MESSAGE_DOMAIN_HTTP,	UG_MESSAGE_DATA_HTTP_MEMBER_REDIRECTION)
#define UG_MESSAGE_DATA_HTTP_CONTENT_LOCATION		UG_MESSAGE_CODE_MAKE (UG_MESSAGE_DOMAIN_HTTP,	UG_MESSAGE_DATA_HTTP_MEMBER_CONTENT_LOCATION)
#define UG_MESSAGE_DATA_HTTP_CONTENT_DISPOSITION	UG_MESSAGE_CODE_MAKE (UG_MESSAGE_DOMAIN_HTTP,	UG_MESSAGE_DATA_HTTP_MEMBER_CONTENT_DISPOSITION)

// HTTP error message
#define UG_MESSAGE_ERROR_HTTP_CUSTOM					UG_MESSAGE_CODE_MAKE (UG_MESSAGE_DOMAIN_HTTP,	UG_MESSAGE_ERROR_HTTP_MEMBER_CUSTOM)
#define UG_MESSAGE_ERROR_HTTP_TOO_MANY_REDIRECTIONS		UG_MESSAGE_CODE_MAKE (UG_MESSAGE_DOMAIN_HTTP,	UG_MESSAGE_ERROR_HTTP_MEMBER_TOO_MANY_REDIRECTIONS)
#define UG_MESSAGE_ERROR_HTTP_OPEN_POSTED_FILE_FAILED	UG_MESSAGE_CODE_MAKE (UG_MESSAGE_DOMAIN_HTTP,	UG_MESSAGE_ERROR_HTTP_MEMBER_OPEN_POSTED_FILE_FAILED)


// ---------------------------------------------------------------------------
// UgMessage: message from plug-in

//  UgData
//  |
//  `- UgDatalist
//     |
//     `- UgMessage

struct UgMessage
{
	UG_DATALIST_MEMBERS (UgMessage);
//	const UgDataInterface*	iface;
//	UgMessage*				next;
//	UgMessage*				prev;

	UgMessageType		type;

	gpointer			src;	// UgPlugin*
	guint32				code;	// 16bit domain + 16bit member

	// If type is UG_MESSAGE_INFO, UG_MESSAGE_WARNING, or UG_MESSAGE_ERROR, string is description.
	// If type is UG_MESSAGE_DATA, string is "info-entry" of data.
	gchar*				string;

	// for UG_MESSAGE_DATA & UG_MESSAGE_STATE
	UgDataType			data_type;
	union
	{
		gchar*			v_string;	// data_type == UG_DATA_STRING
		gint			v_int;		// data_type == UG_DATA_INT
		guint			v_uint;		// data_type == UG_DATA_UINT
		gint64			v_int64;	// data_type == UG_DATA_INT64
		gdouble			v_double;	// data_type == UG_DATA_DOUBLE
		UgData*			v_instance;	// data_type == UG_DATA_INSTANCE
	} data;
};

UgMessage*	ug_message_new			(UgMessageType type);
UgMessage*	ug_message_new_state	(gint state);
UgMessage*	ug_message_new_progress	(void);

UgMessage*	ug_message_new_info		(guint info_code,    const gchar* string);
UgMessage*	ug_message_new_warning	(guint warning_code, const gchar* string);
UgMessage*	ug_message_new_error	(guint error_code,   const gchar* string);

UgMessage*	ug_message_new_data		(guint data_code, ...);

void		ug_message_set_data		(UgMessage* message, UgDataType data_type, ...);
void		ug_message_set_data_v	(UgMessage* message, UgDataType data_type, va_list args);


#ifdef __cplusplus
}
#endif

#endif  // UG_MESSAGE_H

