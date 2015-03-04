/*
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <common/gfal_common_err_helpers.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "gfal_transfer_types_internal.h"


void gfalt_params_handle_init(gfalt_params_t p, GError ** err)
{
    p->lock = FALSE;
    p->nb_data_streams = GFALT_DEFAULT_NB_STREAM;
    p->timeout = GFALT_DEFAULT_TRANSFERT_TIMEOUT;
    p->start_offset = 0;
    p->tcp_buffer_size = 0;
    p->replace_existing = FALSE;
    p->local_transfers = TRUE;
    p->strict_mode = FALSE;
    p->parent_dir_create = FALSE;
    uuid_clear(p->uuid);

    p->monitor_callbacks = NULL;
    p->event_callbacks = NULL;
}


static GSList* gfalt_params_copy_callbacks(const GSList* original)
{
    GSList* copy = NULL;
    const GSList* p = original;
    while (p) {
        struct _gfalt_callback_entry* original_entry = (struct _gfalt_callback_entry*)p->data;
        struct _gfalt_callback_entry* new_entry = g_new0(struct _gfalt_callback_entry, 1);
        new_entry->func = original_entry->func;
        new_entry->udata = original_entry->udata;
        copy = g_slist_append(copy, new_entry);
        p = g_slist_next(p);
    }
    return copy;
}


gfalt_params_t gfalt_params_handle_copy(gfalt_params_t params, GError ** err)
{
    gfalt_params_t p = g_new0(struct _gfalt_params_t, 1);
    memcpy(p, params, sizeof(struct _gfalt_params_t));
    p->src_space_token = g_strdup(params->src_space_token);
    p->dst_space_token = g_strdup(params->dst_space_token);
    p->user_checksum = g_strdup(params->user_checksum);
    p->user_checksum_type = g_strdup(params->user_checksum_type);

    p->monitor_callbacks = gfalt_params_copy_callbacks(params->monitor_callbacks);
    p->event_callbacks = gfalt_params_copy_callbacks(params->event_callbacks);

    return p;
}


gfalt_params_t gfalt_params_handle_new(GError ** err)
{

    gfalt_params_t p = g_new0(struct _gfalt_params_t, 1);
    gfalt_params_handle_init(p, err);
    return p;
}


void gfalt_params_handle_delete(gfalt_params_t params, GError ** err)
{
    if (params) {
        params->lock = FALSE;
        g_free(params->src_space_token);
        g_free(params->dst_space_token);
        g_free(params->user_checksum);
        g_free(params->user_checksum_type);
        g_free(params);
        g_slist_foreach(params->monitor_callbacks, (GFunc)g_free , NULL);
        g_slist_free(params->monitor_callbacks);
        g_slist_foreach(params->event_callbacks, (GFunc)g_free , NULL);
        g_slist_free(params->event_callbacks);
    }
}


gint gfalt_set_timeout(gfalt_params_t params, guint64 timeout, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid parameter handle value");
    params->timeout = timeout;
    return 0;
}


guint64 gfalt_get_timeout(gfalt_params_t params, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");
    return params->timeout;
}


gint gfalt_set_nbstreams(gfalt_params_t params, guint nbstreams, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid parameter handle");
    params->nb_data_streams = nbstreams;
    return 0;
}


gint gfalt_set_tcp_buffer_size(gfalt_params_t params, guint64 tcp_buffer_size,
        GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");
    params->tcp_buffer_size = tcp_buffer_size;
    return 0;
}


guint64 gfalt_get_tcp_buffer_size(gfalt_params_t params, GError** err)
{
    return params->tcp_buffer_size;
}

gint gfalt_set_local_transfer_perm(gfalt_params_t params, gboolean local_transfer_status,
        GError ** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");
    params->local_transfers = local_transfer_status;
    return 0;
}


gboolean gfalt_get_local_transfer_perm(gfalt_params_t params, GError ** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");
    return params->local_transfers;
}


gint gfalt_set_replace_existing_file(gfalt_params_t params, gboolean replace,
        GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");
    params->replace_existing = replace;
    return 0;
}


gboolean gfalt_get_replace_existing_file(gfalt_params_t params, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");
    return params->replace_existing;
}


gint gfalt_set_offset_from_source(gfalt_params_t params, off_t offset, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");
    params->start_offset = offset;
    return 0;
}


static GSList* gfalt_search_callback(GSList* list, gpointer callback)
{
    struct _gfalt_callback_entry* entry;
    GSList* p = list;

    while (p) {
        entry = (struct _gfalt_callback_entry*)p->data;
        if (entry->func == callback)
            return p;
        p = g_slist_next(p);
    }
    return NULL;
}


gint gfalt_add_monitor_callback(gfalt_params_t params, gfalt_monitor_func callback,
        gpointer udata, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");

    struct _gfalt_callback_entry* entry;
    GSList* i = gfalt_search_callback(params->monitor_callbacks, callback);

    if (i) {
        entry = (struct _gfalt_callback_entry*)i->data;
        entry->udata = udata;
    }
    else {
        entry = g_new0(struct _gfalt_callback_entry, 1);
        entry->func = callback;
        entry->udata = udata;
        params->monitor_callbacks = g_slist_append(params->monitor_callbacks, entry);
    }

    return 0;
}


gint gfalt_remove_monitor_callback(gfalt_params_t params, gfalt_monitor_func callback,
        GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");

    GSList* i = gfalt_search_callback(params->monitor_callbacks, callback);
    if (i) {
        g_free(i->data);
        params->monitor_callbacks = g_slist_delete_link(params->monitor_callbacks, i);
        return 0;
    }

    gfal2_set_error(err, gfal2_get_core_quark(), ENOENT, __func__, "Could not find the callback");
    return -1;
}


gint gfalt_add_event_callback(gfalt_params_t params, gfalt_event_func callback,
        gpointer udata, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");

    struct _gfalt_callback_entry* entry;
    GSList* i = gfalt_search_callback(params->event_callbacks, callback);

    if (i) {
        entry = (struct _gfalt_callback_entry*)i->data;
        entry->udata = udata;
    }
    else {
        entry = g_new0(struct _gfalt_callback_entry, 1);
        entry->func = callback;
        entry->udata = udata;
        params->event_callbacks = g_slist_append(params->event_callbacks, entry);
    }

    return 0;
}


gint gfalt_remove_event_callback(gfalt_params_t params, gfalt_event_func callback,
        GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");

    GSList* i = gfalt_search_callback(params->event_callbacks, callback);
    if (i) {
        g_free(i->data);
        params->event_callbacks = g_slist_delete_link(params->event_callbacks, i);
        return 0;
    }

    gfal2_set_error(err, gfal2_get_core_quark(), ENOENT, __func__, "Could not find the callback");
    return -1;
}


gint gfalt_set_user_data(gfalt_params_t params, gpointer user_data, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid params handle");
    params->user_data = user_data;
    return 0;
}


gpointer gfalt_get_user_data(gfalt_params_t params, GError** err)
{
    g_return_val_err_if_fail(params != NULL, NULL, err, "[BUG] invalid params handle");
    return params->user_data;
}


gint gfalt_set_monitor_callback(gfalt_params_t params, gfalt_monitor_func callback,
        GError** err)
{
    return gfalt_add_monitor_callback(params, callback, params->user_data, err);
}


gfalt_monitor_func gfalt_get_monitor_callback(gfalt_params_t params, GError** err)
{
    return NULL;
}


gint gfalt_set_event_callback(gfalt_params_t params, gfalt_event_func callback,
        GError** err)
{
    return gfalt_add_event_callback(params, callback, params->user_data, err);
}


gfalt_event_func gfalt_get_event_callback(gfalt_params_t params, GError** err)
{
    return NULL;
}


guint gfalt_get_nbstreams(gfalt_params_t params, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid parameter handle");
    return params->nb_data_streams;
}


gint gfalt_set_strict_copy_mode(gfalt_params_t params, gboolean strict_mode, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid parameter handle");
    params->strict_mode = strict_mode;
    return 0;
}


gboolean gfalt_get_strict_copy_mode(gfalt_params_t params, GError** err)
{
    g_return_val_err_if_fail(params != NULL, -1, err, "[BUG] invalid parameter handle");
    return params->strict_mode;
}


gint gfalt_set_src_spacetoken(gfalt_params_t params, const char* srm_spacetoken,
        GError** err)
{
    if (params->src_space_token)
        g_free(params->src_space_token);
    params->src_space_token = g_strdup(srm_spacetoken);
    return 0;
}


const gchar* gfalt_get_src_spacetoken(gfalt_params_t params, GError** err)
{
    return params->src_space_token;
}


gint gfalt_set_dst_spacetoken(gfalt_params_t params, const char* srm_spacetoken,
        GError** err)
{
    if (params->dst_space_token)
        g_free(params->dst_space_token);
    params->dst_space_token = g_strdup(srm_spacetoken);
    return 0;
}


const gchar* gfalt_get_dst_spacetoken(gfalt_params_t params, GError** err)
{
    return params->dst_space_token;
}


gint gfalt_set_checksum_check(gfalt_params_t params, gboolean value, GError** err)
{
    params->checksum_check = value;
    return 0;
}


gboolean gfalt_get_checksum_check(gfalt_params_t params, GError** err)
{
    return params->checksum_check;
}


gint gfalt_set_create_parent_dir(gfalt_params_t params, gboolean value, GError** err)
{
    params->parent_dir_create = value;
    return 0;
}


gboolean gfalt_get_create_parent_dir(gfalt_params_t params, GError** err)
{
    return params->parent_dir_create;
}


gint gfalt_set_user_defined_checksum(gfalt_params_t params, const gchar* chktype,
        const gchar* checksum, GError** err)
{
    g_free(params->user_checksum);
    g_free(params->user_checksum_type);
    params->user_checksum = NULL;
    params->user_checksum_type = NULL;

    if (chktype)
        params->user_checksum_type = g_strdup(chktype);
    if (checksum)
        params->user_checksum = g_strdup(checksum);

    return 0;
}


gint gfalt_get_user_defined_checksum(gfalt_params_t params, gchar* chktype_buff,
        size_t chk_type_len, gchar* checksum_buff, size_t checksum_len, GError** err)
{
    g_assert(chktype_buff && checksum_buff);

    if (params->user_checksum)
        g_strlcpy(checksum_buff, params->user_checksum, checksum_len);
    else
        checksum_buff[0] = '\0';

    if (params->user_checksum_type)
        g_strlcpy(chktype_buff, params->user_checksum_type, chk_type_len);
    else
        chktype_buff[0] = '\0';

    return 0;
}


gint gfalt_copy_get_status(gfalt_transfer_status_t s, GError ** err)
{
    g_return_val_err_if_fail(s != NULL, -1, err, "[BUG] invalid transfer status handle");
    return s->hook->status;
}


size_t gfalt_copy_get_average_baudrate(gfalt_transfer_status_t s, GError ** err)
{
    g_return_val_err_if_fail(s != NULL, -1, err, "[BUG] invalid transfer status handle");
    return s->hook->average_baudrate;
}


size_t gfalt_copy_get_instant_baudrate(gfalt_transfer_status_t s, GError ** err)
{
    g_return_val_err_if_fail(s != NULL, -1, err, "[BUG] invalid transfer status handle");
    return s->hook->instant_baudrate;
}


size_t gfalt_copy_get_bytes_transfered(gfalt_transfer_status_t s, GError ** err)
{
    g_return_val_err_if_fail(s != NULL, -1, err, "[BUG] invalid transfer status handle");
    return s->hook->bytes_transfered;
}


time_t gfalt_copy_get_elapsed_time(gfalt_transfer_status_t s, GError ** err)
{
    g_return_val_err_if_fail(s != NULL, -1, err, "[BUG] invalid transfer status handle");
    return s->hook->transfer_time;
}
