#pragma once
/* 
* Copyright @ Members of the EMI Collaboration, 2010.
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
 * @file gfal_srm_mkdir.h
 * @brief header file for the mkdir function on the srm url type
 * @author Devresse Adrien
 * @version 2.0
 * @date 19/05/2011
 * */

#include <glib.h>
#include <common/gfal_types.h>
#include <common/gfal_constants.h>

int gfal_srm_mkdir_recG(plugin_handle ch, const char* surl, mode_t mode, GError** err);

int gfal_srm_mkdirG(plugin_handle handle, const char* surl, mode_t mode, gboolean pflag, GError** err); 