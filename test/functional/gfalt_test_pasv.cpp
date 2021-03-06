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

#include <gtest/gtest.h>

#include <gfal_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/exceptions/gerror_to_cpp.h>
#include <transfer/gfal_transfer.h>

#include <common/gfal_lib_test.h>
#include <common/gfal_gtest_asserts.h>

static void eventCallback(const gfalt_event_t e, gpointer user_data);


class PasvTest: public testing::Test {
public:
    static const char* source_root;
    static const char* destination_root;

    char source[2048];
    char destination[2048];
    gfal2_context_t handle;
    gfalt_params_t params;

    bool pasvEventTriggered;

    PasvTest(): pasvEventTriggered(false) {
        GError *error = NULL;
        handle =  gfal2_context_new(&error);
        Gfal::gerror_to_cpp(&error);
        params = gfalt_params_handle_new(&error);
        Gfal::gerror_to_cpp(&error);

        gfal2_set_opt_boolean(handle, "GRIDFTP PLUGIN", "ENABLE_PASV_PLUGIN", TRUE, &error);
        Gfal::gerror_to_cpp(&error);
    }

    virtual ~PasvTest() {
        gfal2_context_free(handle);
        GError *error = NULL;
        gfalt_params_handle_delete(params, &error);
        g_clear_error(&error);
    }

    virtual void SetUp() {
        pasvEventTriggered = false;

        generate_random_uri(source_root, "pasv_source", source, 2048);
        generate_random_uri(destination_root, "pasv", destination, 2048);

        RecordProperty("Source", source);
        RecordProperty("Destination", source);

        GError* error = NULL;
        int ret = generate_file_if_not_exists(handle, source, "file:///etc/hosts", &error);
        EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);
    }

    virtual void TearDown() {
        GError *error = NULL;
        gfal2_unlink(handle, source, &error);
        g_clear_error(&error);
        gfal2_unlink(handle, destination, &error);
        g_clear_error(&error);

        gfalt_remove_event_callback(params, eventCallback, &error);
        g_clear_error(&error);
    }
};

const char* PasvTest::source_root;
const char* PasvTest::destination_root;


static void eventCallback(const gfalt_event_t e, gpointer user_data)
{
    const GQuark gridftpQuark = g_quark_from_string("GSIFTP");
    const GQuark pasvQuark = g_quark_from_static_string("PASV");

    if (e->domain == gridftpQuark && e->stage == pasvQuark) {
        PasvTest* pasv = (PasvTest*)user_data;
        pasv->pasvEventTriggered = true;

        printf("%s\n", e->description);
    }
}


TEST_F(PasvTest, EnablePasvPlugin)
{
    GError* error = NULL;

    int ret = gfalt_add_event_callback(params, eventCallback, this, NULL, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    ret = gfalt_copy_file(handle, params, source, destination, &error);
    EXPECT_PRED_FORMAT2(AssertGfalSuccess, ret, error);

    EXPECT_TRUE(pasvEventTriggered);
}


int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    if (argc < 3) {
        printf("Missing source and destination base urls\n");
        printf("\t%s [options] srm://host/base/path/ srm://destination/base/path/\n", argv[0]);
        return 1;
    }

    PasvTest::source_root = argv[1];
    PasvTest::destination_root = argv[2];

    return RUN_ALL_TESTS();
}
