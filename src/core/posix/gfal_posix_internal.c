/*
 * Copyright (c) CERN 2013-2015
 *
 * Copyright (c) Members of the EMI Collaboration. 2010-2013
 *  See  http://www.eu-emi.eu/partners for details on the copyright
 *  holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <glib.h>
#include <pthread.h>

#include <common/gfal_prototypes.h>
#include <common/gfal_types.h>
#include <common/gfal_common_plugin.h>

#include "gfal_posix_internal.h"


pthread_mutex_t m_instance = PTHREAD_MUTEX_INITIALIZER;
static gfal2_context_t handle = NULL;
static __thread GError* last_error = NULL;


static void gfal_posix_free_handle(void)
{
    gfal2_context_free(handle);
}


gfal2_context_t gfal_posix_instance()
{
    if (handle == NULL) {
        pthread_mutex_lock(&m_instance);
        if (handle == NULL) {
            handle = gfal2_context_new(&last_error);
        }
        atexit(gfal_posix_free_handle);
        pthread_mutex_unlock(&m_instance);
    }
    return handle;
}


GError** gfal_posix_get_last_error()
{
    return &last_error;
}


/*
 *  register the last error in the handle and display a VERBOSE warning if an error was registered and not deleted
 * */
void gfal_posix_register_internal_error(gfal2_context_t mhandle,
        const char* prefix, GError * tmp_err)
{
    GError** err = gfal_posix_get_last_error();
    if (*err != NULL) {
        gfal2_log(G_LOG_LEVEL_MESSAGE,
                "%s Warning : existing registered error replaced ! old err : %s ",
                prefix, (*err)->message);
        g_clear_error(err);
    }
    gfal2_propagate_prefixed_error(err, tmp_err, prefix);
}
