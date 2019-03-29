// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "test_helpers.h"

#include <k4ainternal/common.h>
#include <k4ainternal/logging.h>
#include <k4ainternal/matroska_common.h>

#define EXIT_IF_FALSE(x)                                                                                               \
    {                                                                                                                  \
        if (!(x))                                                                                                      \
        {                                                                                                              \
            logger_error("PlaybackTest", "%s == false", #x);                                                           \
            exit(1);                                                                                                   \
        }                                                                                                              \
    }

#define VALIDATE_PARAMETER(actual, expected)                                                                           \
    {                                                                                                                  \
        auto _actual = (actual);                                                                                       \
        if (_actual != (expected))                                                                                     \
        {                                                                                                              \
            logger_error("PlaybackTest", "%s is incorrect. Actual: %d, Expected: %d", #actual, _actual, (expected));   \
            return false;                                                                                              \
        }                                                                                                              \
    }

k4a_capture_t create_test_capture(uint64_t timestamp_us[3],
                                  k4a_image_format_t color_format,
                                  k4a_color_resolution_t resolution,
                                  k4a_depth_mode_t mode)
{
    k4a_capture_t capture = NULL;
    k4a_result_t result = k4a_capture_create(&capture);
    EXIT_IF_FALSE(result == K4A_RESULT_SUCCEEDED);

    uint32_t width = 0;
    uint32_t height = 0;
    if (resolution != K4A_COLOR_RESOLUTION_OFF)
    {
        EXIT_IF_FALSE(k4a_convert_resolution_to_width_height(resolution, &width, &height));

        uint32_t color_stride = 0;
        if (color_format == K4A_IMAGE_FORMAT_COLOR_NV12)
        {
            color_stride = width;
        }
        else if (color_format == K4A_IMAGE_FORMAT_COLOR_YUY2)
        {
            color_stride = width * 2;
        }
        k4a_image_t color_image = create_test_image(timestamp_us[0], color_format, width, height, color_stride);
        k4a_capture_set_color_image(capture, color_image);
        k4a_image_release(color_image);
    }

    if (mode != K4A_DEPTH_MODE_OFF)
    {
        EXIT_IF_FALSE(k4a_convert_depth_mode_to_width_height(mode, &width, &height));
        if (mode != K4A_DEPTH_MODE_PASSIVE_IR)
        {
            k4a_image_t depth_image =
                create_test_image(timestamp_us[1], K4A_IMAGE_FORMAT_DEPTH16, width, height, width * 2);
            k4a_capture_set_depth_image(capture, depth_image);
            k4a_image_release(depth_image);
        }
        k4a_image_t ir_image = create_test_image(timestamp_us[2], K4A_IMAGE_FORMAT_IR16, width, height, width * 2);
        k4a_capture_set_ir_image(capture, ir_image);
        k4a_image_release(ir_image);
    }
    return capture;
}

bool validate_test_capture(k4a_capture_t capture,
                           uint64_t timestamp_us[3],
                           k4a_image_format_t color_format,
                           k4a_color_resolution_t resolution,
                           k4a_depth_mode_t mode)
{

    if (capture != NULL)
    {
        if (resolution != K4A_COLOR_RESOLUTION_OFF)
        {
            uint32_t width = 0;
            uint32_t height = 0;
            EXIT_IF_FALSE(k4a_convert_resolution_to_width_height(resolution, &width, &height));

            uint32_t color_stride = 0;
            if (color_format == K4A_IMAGE_FORMAT_COLOR_NV12)
            {
                color_stride = width;
            }
            else if (color_format == K4A_IMAGE_FORMAT_COLOR_YUY2)
            {
                color_stride = width * 2;
            }

            k4a_image_t color_image = k4a_capture_get_color_image(capture);
            if (color_image == NULL)
            {
                logger_error("PlaybackTest", "Color image is missing");
                return false;
            }
            bool image_valid =
                validate_test_image(color_image, timestamp_us[0], color_format, width, height, color_stride);
            k4a_image_release(color_image);
            if (!image_valid)
            {
                logger_error("PlaybackTest", "Color image is invalid");
                return false;
            }
        }
        else if (k4a_capture_get_color_image(capture) != NULL)
        {
            logger_error("PlaybackTest", "Color image is set when it should be NULL");
            return false;
        }

        if (mode != K4A_DEPTH_MODE_OFF)
        {
            uint32_t width = 0;
            uint32_t height = 0;
            EXIT_IF_FALSE(k4a_convert_depth_mode_to_width_height(mode, &width, &height));

            if (mode != K4A_DEPTH_MODE_PASSIVE_IR)
            {
                k4a_image_t depth_image = k4a_capture_get_depth_image(capture);
                if (depth_image == NULL)
                {
                    logger_error("PlaybackTest", "Depth image is missing");
                    return false;
                }
                bool image_valid = validate_test_image(depth_image,
                                                       timestamp_us[1],
                                                       K4A_IMAGE_FORMAT_DEPTH16,
                                                       width,
                                                       height,
                                                       width * 2);
                k4a_image_release(depth_image);
                if (!image_valid)
                {
                    logger_error("PlaybackTest", "Depth image is invalid");
                    return false;
                }
            }
            else if (k4a_capture_get_depth_image(capture) != NULL)
            {
                logger_error("PlaybackTest", "Depth image is set when it should be NULL (Passive IR Mode)");
                return false;
            }

            k4a_image_t ir_image = k4a_capture_get_ir_image(capture);
            if (ir_image == NULL)
            {
                logger_error("PlaybackTest", "IR image is missing");
                return false;
            }
            bool image_valid =
                validate_test_image(ir_image, timestamp_us[2], K4A_IMAGE_FORMAT_IR16, width, height, width * 2);
            k4a_image_release(ir_image);
            if (!image_valid)
            {
                logger_error("PlaybackTest", "IR image is invalid");
                return false;
            }
        }
        else if (k4a_capture_get_depth_image(capture) != NULL)
        {
            logger_error("PlaybackTest", "Depth image is set when it should be NULL");
            return false;
        }
        else if (k4a_capture_get_ir_image(capture) != NULL)
        {
            logger_error("PlaybackTest", "IR image is set when it should be NULL");
            return false;
        }
        return true;
    }
    logger_error("PlaybackTest", "Capture is NULL");
    return false;
}

k4a_image_t
create_test_image(uint64_t timestamp_us, k4a_image_format_t format, uint32_t width, uint32_t height, uint32_t stride)
{
    // Ignore the correct buffer size for testing, and create a 8KB image instead.
    // Generating 1GB+ recordings for testing is too slow.
    size_t buffer_size = 8096;
    uint8_t *buffer = new uint8_t[buffer_size];
    for (size_t i = 0; i < buffer_size / sizeof(uint32_t); i++)
    {
        reinterpret_cast<uint32_t *>(buffer)[i] = 0xAABBCCDD;
    }

    k4a_image_t image = NULL;
    k4a_result_t result = k4a_image_create_from_buffer(format,
                                                       (int)width,
                                                       (int)height,
                                                       (int)stride,
                                                       buffer,
                                                       buffer_size,
                                                       [](void *_buffer, void *context) {
                                                           delete[](uint8_t *) _buffer;
                                                           (void)context;
                                                       },
                                                       NULL,
                                                       &image);
    EXIT_IF_FALSE(result == K4A_RESULT_SUCCEEDED);

    k4a_image_set_timestamp_usec(image, timestamp_us);
    return image;
}

bool validate_test_image(k4a_image_t image,
                         uint64_t timestamp_us,
                         k4a_image_format_t format,
                         uint32_t width,
                         uint32_t height,
                         uint32_t stride)
{
    if (image != NULL)
    {
        // Round to file timescale, and then convert check timestamp.
        uint64_t image_timestamp = k4a_image_get_timestamp_usec(image) * 1000 / MATROSKA_TIMESCALE_NS;
        uint64_t expected_timestamp = timestamp_us * 1000 / MATROSKA_TIMESCALE_NS;
        VALIDATE_PARAMETER(image_timestamp, expected_timestamp);
        VALIDATE_PARAMETER(k4a_image_get_format(image), format);
        VALIDATE_PARAMETER(k4a_image_get_width_pixels(image), (int)width);
        VALIDATE_PARAMETER(k4a_image_get_height_pixels(image), (int)height);
        VALIDATE_PARAMETER(k4a_image_get_stride_bytes(image), (int)stride);

        uint32_t *buffer = reinterpret_cast<uint32_t *>(k4a_image_get_buffer(image));
        size_t buffer_size = k4a_image_get_size(image);
        VALIDATE_PARAMETER(buffer_size, 8096);
        for (size_t i = 0; i < buffer_size / sizeof(uint32_t); i++)
        {
            if (buffer[i] != 0xAABBCCDD)
            {
                logger_error("PlaybackTest",
                             "Image data is incorrect (index %d): 0x%X != 0x%X",
                             i,
                             buffer[i],
                             0xAABBCCDD);
                return false;
            }
        }
        return true;
    }
    logger_error("PlaybackTest", "Image is NULL");
    return false;
}

k4a_imu_sample_t create_test_imu_sample(uint64_t timestamp_us)
{
    k4a_imu_sample_t sample = {};
    sample.acc_timestamp_usec = timestamp_us;
    sample.acc_sample = { { 1.0f, 2.0f, 3.0f } };
    sample.gyro_timestamp_usec = timestamp_us;
    sample.gyro_sample = { { -1.0f, -2.0f, -3.0f } };
    return sample;
}

#if defined(__clang__)
#pragma clang diagnostic push
// 1.0, 2.0, and 3.0 are all exact float values, and no math is done. Equals is fine in this case.
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif
bool validate_imu_sample(k4a_imu_sample_t imu_sample, uint64_t timestamp_us)
{
    if (imu_sample.acc_timestamp_usec != timestamp_us || imu_sample.gyro_timestamp_usec != timestamp_us)
    {
        return false;
    }
    if (imu_sample.acc_sample.v[0] != 1.0f || imu_sample.acc_sample.v[1] != 2.0f || imu_sample.acc_sample.v[2] != 3.0f)
    {
        return false;
    }
    if (imu_sample.gyro_sample.v[0] != -1.0f || imu_sample.gyro_sample.v[1] != -2.0f ||
        imu_sample.gyro_sample.v[2] != -3.0f)
    {
        return false;
    }
    return true;
}
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

template<typename T> void write_value(const T &value, std::vector<uint8_t> *stream)
{
    const uint8_t *valueBegin = reinterpret_cast<const uint8_t *>(&value);
    const uint8_t *valueEnd = valueBegin + sizeof(value);
    stream->insert(stream->end(), valueBegin, valueEnd);
}

std::vector<uint8_t> create_test_custom_track_block(uint64_t timestamp_us)
{
    std::srand(static_cast<uint32_t>(timestamp_us));

    uint32_t item_number = static_cast<uint32_t>(std::rand()) % 100;
    std::vector<uint8_t> track_data;
    write_value(item_number, &track_data);
    for (uint32_t i = 0; i < item_number; i++)
    {
        write_value(static_cast<uint32_t>(std::rand()), &track_data);
    }

    return track_data;
}

bool validate_custom_track_block(const uint8_t *block, size_t block_size, uint64_t timestamp_us)
{
    std::srand(static_cast<uint32_t>(timestamp_us));
    uint32_t expected_item_number = static_cast<uint32_t>(std::rand()) % 100;

    if (block_size != (expected_item_number + 1) * sizeof(uint32_t))
    {
        return false;
    }

    const uint32_t *block_data = reinterpret_cast<const uint32_t *>(block);
    if (*block_data++ != expected_item_number)
    {
        return false;
    }

    for (uint32_t i = 0; i < expected_item_number; i++)
    {
        uint32_t expected_value = static_cast<uint32_t>(std::rand());
        if (block_data[i] != expected_value)
        {
            return false;
        }
    }

    return true;
}
