#pragma once
/*
* Copyright @ Members of the EMI Collaboration, 2013.
* See www.eu-emi.eu for details on the copyright holders.
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

/*
 * @file gfal_common_srm_access.h
 * @brief header file for bringing a file online
 * @author Alejandro Álvarez Ayllón
 * @version 2.0
 * @date 14/01/2013
 * */

#include <glib.h>

#include "gfal_common_srm.h"
#include <common/gfal_types.h>

int gfal_srmv2_bring_onlineG(plugin_handle ch, const char* surl,
                             time_t pintime, time_t timeout,
                             GError** err);