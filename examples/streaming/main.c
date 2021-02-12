// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <stdio.h>
#include <stdlib.h>
#include <k4a/k4a.h>

int main(int argc, char **argv)
{
    int returnCode = 1;
    k4a_device_t device = NULL;
    const int32_t TIMEOUT_IN_MS = 1000;
    int captureFrameCount;
    k4a_capture_t capture = NULL;

    // 1. declare device info and mode infos
    k4a_device_info_t device_info = { sizeof(k4a_device_info_t), K4A_ABI_VERSION, 0 };
    k4a_color_mode_info_t color_mode_info = { sizeof(k4a_color_mode_info_t), K4A_ABI_VERSION, 0 };
    k4a_depth_mode_info_t depth_mode_info = { sizeof(k4a_depth_mode_info_t), K4A_ABI_VERSION, 0 };
    k4a_fps_mode_info_t fps_mode_info = { sizeof(k4a_fps_mode_info_t), K4A_ABI_VERSION, 0 };

    // 2. initialize device capabilities

    uint32_t capabilities = 0;
    bool hasDepthDevice = false;
    bool hasColorDevice = false;

    // 2. initialize default mode ids
    uint32_t color_mode_id = 0;
    uint32_t depth_mode_id = 0;
    uint32_t fps_mode_id = 0;

    // 3. get the count of modes
    uint32_t color_mode_count = 0;
    uint32_t depth_mode_count = 0;
    uint32_t fps_mode_count = 0;


    if (argc < 2)
    {
        printf("%s FRAMECOUNT\n", argv[0]);
        printf("Capture FRAMECOUNT color and depth frames from the device using the separate get frame APIs\n");
        returnCode = 2;
        goto Exit;
    }

    captureFrameCount = atoi(argv[1]);
    printf("Capturing %d frames\n", captureFrameCount);

    uint32_t device_count = k4a_device_get_installed_count();

    if (device_count == 0)
    {
        printf("No K4A devices found\n");
        return 0;
    }

    if (K4A_RESULT_SUCCEEDED != k4a_device_open(K4A_DEVICE_DEFAULT, &device))
    {
        printf("Failed to open device\n");
        goto Exit;
    }

    // 4. get available modes from device info
    if (!k4a_device_get_info(device, &device_info) == K4A_RESULT_SUCCEEDED)
    {
        printf("Failed to get device info");
        exit(-1);
    }

    // Capabilities is a bitmask in which bit 0 is depth and bit 1 is color.  See k4a_device_capabilities_t in
    // k4atypes.h.
    capabilities = device_info.capabilities;
    hasDepthDevice = (capabilities & 0x0001) == 1;
    hasColorDevice = ((capabilities >> 1) & 0x01) == 1;

    if (hasColorDevice && !k4a_device_get_color_mode_count(device, &color_mode_count) == K4A_RESULT_SUCCEEDED)
    {
        printf("Failed to get color mode count\n");
        exit(-1);
    }

    if (hasDepthDevice && !k4a_device_get_depth_mode_count(device, &depth_mode_count) == K4A_RESULT_SUCCEEDED)
    {
        printf("Failed to get depth mode count\n");
        exit(-1);
    }

    if (!k4a_device_get_fps_mode_count(device, &fps_mode_count) == K4A_RESULT_SUCCEEDED)
    {
        printf("Failed to get fps mode count\n");
        exit(-1);
    }

    // 5. find the mode ids you want
    if (hasColorDevice && color_mode_count > 1)
    {
        for (uint32_t c = 1; c < color_mode_count; c++)
        {
            k4a_color_mode_info_t color_mode = { sizeof(k4a_color_mode_info_t), K4A_ABI_VERSION, 0 };
            if (k4a_device_get_color_mode(device, c, &color_mode) == K4A_RESULT_SUCCEEDED)
            {
                if (color_mode.height >= 2160)
                {
                    color_mode_id = c;
                    break;
                }
            }
        }
    }

    if (hasDepthDevice && depth_mode_count > 1)
    {
        for (uint32_t d = 1; d < depth_mode_count; d++)
        {
            k4a_depth_mode_info_t depth_mode = { sizeof(k4a_depth_mode_info_t), K4A_ABI_VERSION, 0 };
            if (k4a_device_get_depth_mode(device, d, &depth_mode) == K4A_RESULT_SUCCEEDED)
            {
                if (depth_mode.height >= 576 && depth_mode.vertical_fov <= 65)
                {
                    depth_mode_id = d;
                    break;
                }
            }
        }
    }

    if (fps_mode_count > 1)
    {
        uint32_t max_fps = 0;
        for (uint32_t f = 1; f < fps_mode_count; f++)
        {
            k4a_fps_mode_info_t fps_mode = { sizeof(k4a_fps_mode_info_t), K4A_ABI_VERSION, 0 };
            if (k4a_device_get_fps_mode(device, f, &fps_mode) == K4A_RESULT_SUCCEEDED)
            {
                if (fps_mode.fps >= (int)max_fps)
                {
                    max_fps = (uint32_t)fps_mode.fps;
                    fps_mode_id = f;
                }
            }
        }
    }

    // 6. fps mode id must not be set to 0, which is Off, and either color mode id or depth mode id must not be set to 0
    if (fps_mode_id == 0)
    {
        printf("Fps mode id must not be set to 0 (Off)\n");
        exit(-1);
    }

    if (color_mode_id == 0 && depth_mode_id == 0)
    {
        printf("Either color mode id or depth mode id must not be set to 0 (Off)\n");
        exit(-1);
    }

    // 7. use the mode ids to get the modes
    if (hasColorDevice)
    {
        k4a_device_get_color_mode(device, color_mode_id, &color_mode_info);
    }
    if (hasDepthDevice)
    {
        k4a_device_get_depth_mode(device, depth_mode_id, &depth_mode_info);
    }
    k4a_device_get_fps_mode(device, fps_mode_id, &fps_mode_info);

    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_MJPG;
    config.color_mode_id = color_mode_info.mode_id;
    config.depth_mode_id = depth_mode_info.mode_id;
    config.fps_mode_id = fps_mode_info.mode_id;

    if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
    {
        printf("Failed to start device\n");
        goto Exit;
    }

    while (captureFrameCount-- > 0)
    {
        k4a_image_t image;

        // Get a depth frame
        switch (k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS))
        {
        case K4A_WAIT_RESULT_SUCCEEDED:
            break;
        case K4A_WAIT_RESULT_TIMEOUT:
            printf("Timed out waiting for a capture\n");
            continue;
            break;
        case K4A_WAIT_RESULT_FAILED:
        case K4A_WAIT_RESULT_UNSUPPORTED:
            printf("Failed to read a capture\n");
            goto Exit;
        }

        printf("Capture");

        // Probe for a color image
        image = k4a_capture_get_color_image(capture);
        if (image)
        {
            printf(" | Color res:%4dx%4d stride:%5d ",
                   k4a_image_get_height_pixels(image),
                   k4a_image_get_width_pixels(image),
                   k4a_image_get_stride_bytes(image));
            k4a_image_release(image);
        }
        else
        {
            printf(" | Color None                       ");
        }

        // probe for a IR16 image
        image = k4a_capture_get_ir_image(capture);
        if (image != NULL)
        {
            printf(" | Ir16 res:%4dx%4d stride:%5d ",
                   k4a_image_get_height_pixels(image),
                   k4a_image_get_width_pixels(image),
                   k4a_image_get_stride_bytes(image));
            k4a_image_release(image);
        }
        else
        {
            printf(" | Ir16 None                       ");
        }

        // Probe for a depth16 image
        image = k4a_capture_get_depth_image(capture);
        if (image != NULL)
        {
            printf(" | Depth16 res:%4dx%4d stride:%5d\n",
                   k4a_image_get_height_pixels(image),
                   k4a_image_get_width_pixels(image),
                   k4a_image_get_stride_bytes(image));
            k4a_image_release(image);
        }
        else
        {
            printf(" | Depth16 None\n");
        }

        // release capture
        k4a_capture_release(capture);
        fflush(stdout);
    }

    returnCode = 0;
Exit:
    if (device != NULL)
    {
        k4a_device_close(device);
    }

    return returnCode;
}
