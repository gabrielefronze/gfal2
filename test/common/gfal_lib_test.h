/*
 * Copyright (c) CERN 2013-2017
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

#pragma once
#ifndef GFAL_LIB_TEST_H
#define GFAL_LIB_TEST_H

#include <gfal_api.h>

#ifdef __cplusplus
extern "C" {
#endif

char* generate_random_uri(const char* uri_dir, const char* prefix, char* buff,
        size_t s_buff);

/**
 * If surl does not exist, it creates it putting the content of src.
 */
int generate_file_if_not_exists(gfal2_context_t handle, const char* surl,
        const char* src, GError** error);

/**
 * Return 1 if the copy from url1 to url2 should be third party copy
 */
int expect_third_party_copy(const char *url1, const char *url2);

#ifdef __cplusplus
}
#endif

#endif /* GFAL_LIB_TEST_H */
