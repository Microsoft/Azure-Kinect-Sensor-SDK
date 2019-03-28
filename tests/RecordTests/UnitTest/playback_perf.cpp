// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <utcommon.h>
#include <k4a/k4a.h>
#include <k4ainternal/common.h>
#include <k4ainternal/matroska_common.h>

#include "test_helpers.h"
#include <fstream>

// Module being tested
#include <k4arecord/playback.h>

using namespace testing;

static std::string g_test_file_name;

class playback_perf : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(playback_perf, test_open)
{
    k4a_playback_t handle = NULL;
    k4a_result_t result = K4A_RESULT_FAILED;
    {
        Timer t("File open: " + g_test_file_name);
        result = k4a_playback_open(g_test_file_name.c_str(), &handle);
    }
    ASSERT_EQ(result, K4A_RESULT_SUCCEEDED);

    k4a_record_configuration_t config;
    result = k4a_playback_get_record_configuration(handle, &config);
    ASSERT_EQ(result, K4A_RESULT_SUCCEEDED);
    std::cout << "Config:" << std::endl;
    std::cout << "    Tracks enabled:";
    static const std::pair<bool *, std::string> tracks[] = { { &config.color_track_enabled, "Color" },
                                                             { &config.depth_track_enabled, "Depth" },
                                                             { &config.ir_track_enabled, "IR" },
                                                             { &config.imu_track_enabled, "IMU" } };
    for (int i = 0; i < 4; i++)
    {
        if (*tracks[i].first)
        {
            std::cout << " " << tracks[i].second;
        }
    }
    std::cout << std::endl;
    std::cout << "    Color format: " << format_names[config.color_format] << std::endl;
    std::cout << "    Color resolution: " << resolution_names[config.color_resolution] << std::endl;
    std::cout << "    Depth mode: " << depth_names[config.depth_mode] << std::endl;
    std::cout << "    Frame rate: " << fps_names[config.camera_fps] << std::endl;
    std::cout << "    Depth delay: " << config.depth_delay_off_color_usec << " usec" << std::endl;
    std::cout << "    Start offset: " << config.start_timestamp_offset_usec << " usec" << std::endl;

    k4a_playback_close(handle);
}

TEST_F(playback_perf, test_1000_reads_forward)
{
    k4a_playback_t handle = NULL;
    k4a_result_t result = K4A_RESULT_FAILED;
    {
        Timer t("File open: " + g_test_file_name);
        result = k4a_playback_open(g_test_file_name.c_str(), &handle);
    }
    ASSERT_EQ(result, K4A_RESULT_SUCCEEDED);

    {
        k4a_capture_t capture = NULL;
        k4a_stream_result_t playback_result = K4A_STREAM_RESULT_FAILED;
        Timer t("Next capture x1000");
        for (int i = 0; i < 1000; i++)
        {
            playback_result = k4a_playback_get_next_capture(handle, &capture);
            ASSERT_EQ(playback_result, K4A_STREAM_RESULT_SUCCEEDED);
            ASSERT_NE(capture, nullptr);
            k4a_capture_release(capture);
        }
    }

    k4a_playback_close(handle);
}

TEST_F(playback_perf, test_1000_reads_backward)
{
    k4a_playback_t handle = NULL;
    k4a_result_t result = K4A_RESULT_FAILED;
    {
        Timer t("File open: " + g_test_file_name);
        result = k4a_playback_open(g_test_file_name.c_str(), &handle);
    }
    ASSERT_EQ(result, K4A_RESULT_SUCCEEDED);

    {
        Timer t("Seek to end");
        result = k4a_playback_seek_timestamp(handle, 0, K4A_PLAYBACK_SEEK_END);
        ASSERT_EQ(result, K4A_RESULT_SUCCEEDED);
    }

    {
        k4a_capture_t capture = NULL;
        k4a_stream_result_t playback_result = K4A_STREAM_RESULT_FAILED;
        Timer t("Previous capture x1000");
        for (int i = 0; i < 1000; i++)
        {
            playback_result = k4a_playback_get_previous_capture(handle, &capture);
            ASSERT_EQ(playback_result, K4A_STREAM_RESULT_SUCCEEDED);
            ASSERT_NE(capture, nullptr);
            k4a_capture_release(capture);
        }
    }

    k4a_playback_close(handle);
}

TEST_F(playback_perf, test_read_latency_30fps)
{
    k4a_playback_t handle = NULL;
    k4a_result_t result = K4A_RESULT_FAILED;
    {
        Timer t("File open: " + g_test_file_name);
        result = k4a_playback_open(g_test_file_name.c_str(), &handle);
    }
    ASSERT_EQ(result, K4A_RESULT_SUCCEEDED);

    std::vector<uint64_t> deltas;

    {
        k4a_capture_t capture = NULL;
        k4a_stream_result_t playback_result = K4A_STREAM_RESULT_FAILED;
        Timer t("Next capture x1000");
        for (int i = 0; i < 1000; i++)
        {

            auto start = std::chrono::high_resolution_clock::now();
            playback_result = k4a_playback_get_next_capture(handle, &capture);
            auto delta = std::chrono::high_resolution_clock::now() - start;

            ASSERT_EQ(playback_result, K4A_STREAM_RESULT_SUCCEEDED);
            ASSERT_NE(capture, nullptr);
            k4a_capture_release(capture);

            deltas.push_back(delta.count());
            std::this_thread::sleep_until(start + std::chrono::milliseconds(33));
        }
    }

    std::sort(deltas.begin(), deltas.end(), std::less<uint64_t>());
    uint64_t total_ns = 0;
    for (auto d : deltas)
    {
        total_ns += d;
    }
    std::cout << "Avg latency: " << (total_ns / deltas.size() / 1000) << " usec" << std::endl;
    std::cout << "P95 latency: " << (deltas[(size_t)((double)deltas.size() * 0.95) - 1] / 1000) << " usec" << std::endl;
    std::cout << "P99 latency: " << (deltas[(size_t)((double)deltas.size() * 0.99) - 1] / 1000) << " usec" << std::endl;

    k4a_playback_close(handle);
}

int main(int argc, char **argv)
{
    k4a_unittest_init();

    ::testing::InitGoogleTest(&argc, argv);

    if (argc < 2)
    {
        std::cout << "Usage: playback_perf <options> <testfile.mkv>" << std::endl;
        return 1;
    }
    g_test_file_name = std::string(argv[1]);

    return RUN_ALL_TESTS();
}
