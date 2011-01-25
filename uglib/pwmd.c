/*
    Copyright (C) 2011 Ben Kibbey <bjk@luxsci.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02110-1301  USA
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBPWMD
#include <string.h>
#include <stdlib.h>
#include "pwmd.h"

gpg_error_t ug_set_pwmd_proxy_options(struct pwmd_proxy_s *pwmd,
       UgDataProxy *proxy)
{
       gpg_error_t rc;
       gchar *result;
       gchar *path = NULL;
       gint i;

       pwmd->port = 80;

       if (proxy->pwmd.element) {
               pwmd->path = path = g_strdup_printf("%s\t", proxy->pwmd.element);

               for (i = 0; i < strlen(path); i++) {
                       if (path[i] == '^')
                               path[i] = '\t';
               }
       }

       pwmd_init();
       pwm_t *pwm = pwmd_new("uget");
       rc = pwmd_connect_url(pwm, proxy->pwmd.socket);

       if (rc)
               goto fail;

       pwmd_socket_t stype;
       rc = pwmd_socket_type(pwm, &stype);

       if (rc)
               goto fail;

       if (stype == PWMD_SOCKET_SSH)
               rc = pwmd_open2(pwm, proxy->pwmd.file);
       else
               rc = pwmd_open(pwm, proxy->pwmd.file);

       if (rc)
               goto fail;

       rc = pwmd_command(pwm, &result, "GET %stype", path ? path : "");

       if (rc)
               goto fail;

       pwmd->type = result;
       rc = pwmd_command(pwm, &result, "GET %shostname", path ? path : "");

       if (rc)
               goto fail;

       pwmd->hostname = result;
       rc = pwmd_command(pwm, &result, "GET %sport", path ? path : "");

       if (rc && rc != GPG_ERR_ELEMENT_NOT_FOUND)
               goto fail;

       pwmd->port = atoi(result);
       pwmd_free(result);
       rc = pwmd_command(pwm, &result, "GET %susername", path ? path : "");

       if (rc && rc != GPG_ERR_ELEMENT_NOT_FOUND)
               goto fail;

       if (!rc)
               pwmd->username = result;

       rc = pwmd_command(pwm, &result, "GET %spassword", path ? path : "");

       if (rc && rc != GPG_ERR_ELEMENT_NOT_FOUND)
               goto fail;

       if (!rc)
               pwmd->password = result;

       rc = 0;

fail:
       return rc;
}

void ug_close_pwmd(struct pwmd_proxy_s *pwmd)
{
       if (pwmd->type)
               pwmd_free(pwmd->type);

       if (pwmd->hostname)
               pwmd_free(pwmd->hostname);

       if (pwmd->username)
               pwmd_free(pwmd->username);

       if (pwmd->password)
               pwmd_free(pwmd->password);

       if (pwmd->path)
               g_free(pwmd->path);

       if (pwmd->pwm)
               pwmd_close(pwmd->pwm);
}
#endif
