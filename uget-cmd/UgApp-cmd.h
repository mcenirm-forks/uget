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


#ifndef UG_APP_CMD_H
#define UG_APP_CMD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <UgApp-base.h>
#include <UgRunning.h>
#include <UgCategory-cmd.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UG_APP_CMD_DIR				"uGet"
#define UG_APP_CMD_CATEGORY_FILE	"CategoryList-cmd.xml"
#define UG_APP_CMD_DOWNLOAD_FILE	"DownloadList-cmd.xml"

typedef struct	UgAppCmd			UgAppCmd;


// ----------------------------------------------------------------------------
// UgAppCmd: uGet application for command-line
//
struct UgAppCmd
{
	UgIpc			ipc;
	UgOption		option;
	UgRunning		running;

	GList*			category_list;

	GMainLoop*		main_loop;
};

void	ug_app_cmd_run  (UgAppCmd* app);
void	ug_app_cmd_save (UgAppCmd* app);
void	ug_app_cmd_load (UgAppCmd* app);


#ifdef __cplusplus
}
#endif

#endif  // UG_APP_CMD_H

