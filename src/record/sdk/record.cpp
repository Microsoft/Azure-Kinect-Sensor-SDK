// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <ctime>
#include <iostream>
#include <sstream>

#include <k4a/k4a.h>
#include <k4arecord/record.h>
#include <k4ainternal/matroska_write.h>
#include <k4ainternal/logging.h>
#include <k4ainternal/common.h>

using namespace k4arecord;
using namespace LIBMATROSKA_NAMESPACE;

k4a_result_t k4a_record_set_debug_message_handler(k4a_logging_message_cb_t *message_cb,
                                                  void *message_cb_context,
                                                  k4a_log_level_t min_level)
{
    return logger_register_message_callback(message_cb, message_cb_context, min_level);
}

k4a_result_t k4a_record_create(const char *path,
                               k4a_device_t device,
                               const k4a_device_configuration_t device_config,
                               k4a_record_t *recording_handle)
{
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, path == NULL);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, recording_handle == NULL);
    k4a_record_context_t *context = NULL;
    k4a_result_t result = K4A_RESULT_SUCCEEDED;

    context = k4a_record_t_create(recording_handle);
    result = K4A_RESULT_FROM_BOOL(context != NULL);

    if (K4A_SUCCEEDED(result))
    {
        context->file_path = path;

        try
        {
            context->ebml_file = make_unique<LargeFileIOCallback>(path, MODE_CREATE);
        }
        catch (std::ios_base::failure &e)
        {
            LOG_ERROR("Unable to open file '%s': %s", path, e.what());
            result = K4A_RESULT_FAILED;
        }
    }

    if (K4A_SUCCEEDED(result))
    {
        context->device_config = device_config;

        context->timecode_scale = MATROSKA_TIMESCALE_NS;
        context->camera_fps = k4a_convert_fps_to_uint(device_config.camera_fps);
        if (context->camera_fps == 0)
        {
            // Set camera FPS to 30 if no cameras are enabled so IMU can still be written.
            context->camera_fps = 30;
        }
    }

    uint32_t color_width = 0;
    uint32_t color_height = 0;
    if (K4A_SUCCEEDED(result) && device_config.color_resolution != K4A_COLOR_RESOLUTION_OFF)
    {
        if (!k4a_convert_resolution_to_width_height(device_config.color_resolution, &color_width, &color_height))
        {
            LOG_ERROR("Unsupported color_resolution specified in recording: %d", device_config.color_resolution);
            result = K4A_RESULT_FAILED;
        }
    }

    std::ostringstream color_mode_str;
    if (K4A_SUCCEEDED(result))
    {
        if (device_config.color_resolution != K4A_COLOR_RESOLUTION_OFF)
        {
            switch (device_config.color_format)
            {
            case K4A_IMAGE_FORMAT_COLOR_NV12:
                color_mode_str << "NV12_" << color_height << "P";
                break;
            case K4A_IMAGE_FORMAT_COLOR_YUY2:
                color_mode_str << "YUY2_" << color_height << "P";
                break;
            case K4A_IMAGE_FORMAT_COLOR_MJPG:
                color_mode_str << "MJPG_" << color_height << "P";
                break;
            case K4A_IMAGE_FORMAT_COLOR_BGRA32:
                color_mode_str << "BGRA_" << color_height << "P";
                break;
            default:
                LOG_ERROR("Unsupported color_format specified in recording: %d", device_config.color_format);
                result = K4A_RESULT_FAILED;
            }
        }
        else
        {
            color_mode_str << "OFF";
        }
    }

    const char *depth_mode_str = "OFF";
    uint32_t depth_width = 0;
    uint32_t depth_height = 0;
    if (K4A_SUCCEEDED(result))
    {
        if (device_config.depth_mode != K4A_DEPTH_MODE_OFF)
        {
            for (size_t i = 0; i < arraysize(depth_modes); i++)
            {
                if (device_config.depth_mode == depth_modes[i].first)
                {
                    if (!k4a_convert_depth_mode_to_width_height(depth_modes[i].first, &depth_width, &depth_height))
                    {
                        LOG_ERROR("Unsupported depth_mode specified in recording: %d", device_config.depth_mode);
                        result = K4A_RESULT_FAILED;
                    }
                    depth_mode_str = depth_modes[i].second.c_str();
                    break;
                }
            }
            if (depth_width == 0 || depth_height == 0)
            {
                LOG_ERROR("Unsupported depth_mode specified in recording: %d", device_config.depth_mode);
                result = K4A_RESULT_FAILED;
            }
        }
    }

    if (K4A_SUCCEEDED(result))
    {
        context->file_segment = make_unique<KaxSegment>();

        { // Setup segment info
            auto &segment_info = GetChild<KaxInfo>(*context->file_segment);

            GetChild<KaxTimecodeScale>(segment_info).SetValue(context->timecode_scale);
            GetChild<KaxMuxingApp>(segment_info).SetValue(L"libmatroska-1.4.9");
            std::ostringstream version_str;
            version_str << "k4arecord-" << K4A_VERSION_STR;
            GetChild<KaxWritingApp>(segment_info).SetValueUTF8(version_str.str());
            GetChild<KaxDateUTC>(segment_info).SetEpochDate(time(0));
            GetChild<KaxTitle>(segment_info).SetValue(L"Azure Kinect");
        }

        auto &tags = GetChild<KaxTags>(*context->file_segment);
        tags.EnableChecksum();
    }

    if (K4A_SUCCEEDED(result) && device_config.color_resolution != K4A_COLOR_RESOLUTION_OFF)
    {
        BITMAPINFOHEADER codec_info = {};
        result = TRACE_CALL(
            populate_bitmap_info_header(&codec_info, color_width, color_height, device_config.color_format));

        context->color_track = add_track(context,
                                         K4A_TRACK_NAME_COLOR,
                                         track_video,
                                         "V_MS/VFW/FOURCC",
                                         reinterpret_cast<uint8_t *>(&codec_info),
                                         sizeof(codec_info));
        if (context->color_track != nullptr)
        {
            set_track_info_video(context->color_track, color_width, color_height, context->camera_fps);

            uint64_t track_uid = GetChild<KaxTrackUID>(*context->color_track->track).GetValue();
            std::ostringstream track_uid_str;
            track_uid_str << track_uid;
            add_tag(context, "K4A_COLOR_TRACK", track_uid_str.str().c_str(), TAG_TARGET_TYPE_TRACK, track_uid);
            add_tag(context, "K4A_COLOR_MODE", color_mode_str.str().c_str(), TAG_TARGET_TYPE_TRACK, track_uid);
        }
        else
        {
            LOG_ERROR("Failed to add color track.", 0);
            result = K4A_RESULT_FAILED;
        }
    }

    if (K4A_SUCCEEDED(result))
    {
        if (device_config.depth_mode == K4A_DEPTH_MODE_PASSIVE_IR)
        {
            add_tag(context, "K4A_DEPTH_MODE", depth_mode_str);
        }
        else if (device_config.depth_mode != K4A_DEPTH_MODE_OFF)
        {
            // Depth track
            BITMAPINFOHEADER codec_info = {};
            result = TRACE_CALL(
                populate_bitmap_info_header(&codec_info, depth_width, depth_height, K4A_IMAGE_FORMAT_DEPTH16));

            context->depth_track = add_track(context,
                                             K4A_TRACK_NAME_DEPTH,
                                             track_video,
                                             "V_MS/VFW/FOURCC",
                                             reinterpret_cast<uint8_t *>(&codec_info),
                                             sizeof(codec_info));
            if (context->depth_track != nullptr)
            {
                set_track_info_video(context->depth_track, depth_width, depth_height, context->camera_fps);

                uint64_t track_uid = GetChild<KaxTrackUID>(*context->depth_track->track).GetValue();
                std::ostringstream track_uid_str;
                track_uid_str << track_uid;
                add_tag(context, "K4A_DEPTH_TRACK", track_uid_str.str().c_str(), TAG_TARGET_TYPE_TRACK, track_uid);
                add_tag(context, "K4A_DEPTH_MODE", depth_mode_str, TAG_TARGET_TYPE_TRACK, track_uid);
            }
            else
            {
                LOG_ERROR("Failed to add depth track.", 0);
                result = K4A_RESULT_FAILED;
            }
        }
    }

    if (K4A_SUCCEEDED(result) && device_config.depth_mode != K4A_DEPTH_MODE_OFF)
    {
        // IR Track
        BITMAPINFOHEADER codec_info = {};
        result = TRACE_CALL(populate_bitmap_info_header(&codec_info, depth_width, depth_height, K4A_IMAGE_FORMAT_IR16));

        context->ir_track = add_track(context,
                                      K4A_TRACK_NAME_IR,
                                      track_video,
                                      "V_MS/VFW/FOURCC",
                                      reinterpret_cast<uint8_t *>(&codec_info),
                                      sizeof(codec_info));
        if (context->ir_track != nullptr)
        {
            set_track_info_video(context->ir_track, depth_width, depth_height, context->camera_fps);

            uint64_t track_uid = GetChild<KaxTrackUID>(*context->ir_track->track).GetValue();
            std::ostringstream track_uid_str;
            track_uid_str << track_uid;
            add_tag(context, "K4A_IR_TRACK", track_uid_str.str().c_str(), TAG_TARGET_TYPE_TRACK, track_uid);
            add_tag(context,
                    "K4A_IR_MODE",
                    device_config.depth_mode == K4A_DEPTH_MODE_PASSIVE_IR ? "PASSIVE" : "ACTIVE",
                    TAG_TARGET_TYPE_TRACK,
                    track_uid);
        }
        else
        {
            LOG_ERROR("Failed to add ir track.", 0);
            result = K4A_RESULT_FAILED;
        }
    }

    if (K4A_SUCCEEDED(result) && device_config.color_resolution != K4A_COLOR_RESOLUTION_OFF &&
        device_config.depth_mode != K4A_DEPTH_MODE_OFF)
    {
        std::ostringstream delay_str;
        delay_str << device_config.depth_delay_off_color_usec * 1000;
        add_tag(context, "K4A_DEPTH_DELAY_NS", delay_str.str().c_str());
    }

    if (K4A_SUCCEEDED(result))
    {
        switch (device_config.wired_sync_mode)
        {
        case K4A_WIRED_SYNC_MODE_STANDALONE:
            add_tag(context, "K4A_WIRED_SYNC_MODE", "STANDALONE");
            break;
        case K4A_WIRED_SYNC_MODE_MASTER:
            add_tag(context, "K4A_WIRED_SYNC_MODE", "MASTER");
            break;
        case K4A_WIRED_SYNC_MODE_SUBORDINATE:
            add_tag(context, "K4A_WIRED_SYNC_MODE", "SUBORDINATE");

            std::ostringstream delay_str;
            delay_str << device_config.subordinate_delay_off_master_usec * 1000;
            add_tag(context, "K4A_SUBORDINATE_DELAY_NS", delay_str.str().c_str());
            break;
        }
    }

    if (K4A_SUCCEEDED(result) && device != NULL)
    {
        // Add the firmware version and device serial number to the recording
        k4a_hardware_version_t version_info;
        result = TRACE_CALL(k4a_device_get_version(device, &version_info));

        std::ostringstream color_firmware_str;
        color_firmware_str << version_info.rgb.major << "." << version_info.rgb.minor << "."
                           << version_info.rgb.iteration;
        std::ostringstream depth_firmware_str;
        depth_firmware_str << version_info.depth.major << "." << version_info.depth.minor << "."
                           << version_info.depth.iteration;
        add_tag(context, "K4A_COLOR_FIRMWARE_VERSION", color_firmware_str.str().c_str());
        add_tag(context, "K4A_DEPTH_FIRMWARE_VERSION", depth_firmware_str.str().c_str());

        char serial_number_buffer[256];
        size_t serial_number_buffer_size = sizeof(serial_number_buffer);
        // If reading the device serial number fails, just log the error and continue. The recording is still valid.
        if (TRACE_BUFFER_CALL(k4a_device_get_serialnum(device, serial_number_buffer, &serial_number_buffer_size)) ==
            K4A_BUFFER_RESULT_SUCCEEDED)
        {
            add_tag(context, "K4A_DEVICE_SERIAL_NUMBER", serial_number_buffer);
        }
    }

    if (K4A_SUCCEEDED(result) && device != NULL)
    {
        // Add calibration.json to the recording
        size_t calibration_size = 0;
        k4a_buffer_result_t buffer_result = TRACE_BUFFER_CALL(
            k4a_device_get_raw_calibration(device, NULL, &calibration_size));
        if (buffer_result == K4A_BUFFER_RESULT_TOO_SMALL)
        {
            std::vector<uint8_t> calibration_buffer = std::vector<uint8_t>(calibration_size);
            buffer_result = TRACE_BUFFER_CALL(
                k4a_device_get_raw_calibration(device, calibration_buffer.data(), &calibration_size));
            if (buffer_result == K4A_BUFFER_RESULT_SUCCEEDED)
            {
                // Remove the null-terminated byte from the file before writing it.
                if (calibration_size > 0 && calibration_buffer[calibration_size - 1] == '\0')
                {
                    calibration_size--;
                }
                KaxAttached *attached = add_attachment(context,
                                                       "calibration.json",
                                                       "application/octet-stream",
                                                       calibration_buffer.data(),
                                                       calibration_size);
                add_tag(context,
                        "K4A_CALIBRATION_FILE",
                        "calibration.json",
                        TAG_TARGET_TYPE_ATTACHMENT,
                        get_attachment_uid(attached));
            }
            else
            {
                result = K4A_RESULT_FAILED;
            }
        }
        else
        {
            result = K4A_RESULT_FAILED;
        }
    }

    if (K4A_SUCCEEDED(result))
    {
        auto &cues = GetChild<KaxCues>(*context->file_segment);
        cues.SetGlobalTimecodeScale(context->timecode_scale);
    }
    else
    {
        if (context && context->ebml_file)
        {
            try
            {
                context->ebml_file->close();
            }
            catch (std::ios_base::failure &)
            {
                // The file is empty at this point, ignore any close failures.
            }
        }

        k4a_record_t_destroy(*recording_handle);
        *recording_handle = NULL;
    }

    return result;
}

k4a_result_t k4a_record_add_tag(const k4a_record_t recording_handle, const char *name, const char *value)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, name == NULL || value == NULL);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (context->header_written)
    {
        LOG_ERROR("Tags must be added before the recording header is written.", 0);
        return K4A_RESULT_FAILED;
    }

    if (NULL == add_tag(context, name, value))
    {
        return K4A_RESULT_FAILED;
    }

    return K4A_RESULT_SUCCEEDED;
}

k4a_result_t k4a_record_add_attachment(const k4a_record_t recording_handle,
                                       const char *file_name,
                                       const uint8_t *buffer,
                                       size_t buffer_size)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, file_name == NULL);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, buffer == NULL);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (context->header_written)
    {
        LOG_ERROR("Attachments must be added before the recording header is written.", 0);
        return K4A_RESULT_FAILED;
    }

    KaxAttached *attached = add_attachment(context, file_name, "application/octet-stream", buffer, buffer_size);

    return K4A_RESULT_FROM_BOOL(attached != NULL);
}

k4a_result_t k4a_record_add_imu_track(const k4a_record_t recording_handle)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (context->header_written)
    {
        LOG_ERROR("The IMU track must be added before the recording header is written.", 0);
        return K4A_RESULT_FAILED;
    }

    if (context->imu_track)
    {
        LOG_ERROR("The IMU track has already been added to this recording.", 0);
        return K4A_RESULT_FAILED;
    }

    context->imu_track = add_track(context, K4A_TRACK_NAME_IMU, track_subtitle, "S_K4A/IMU");
    if (context->imu_track == nullptr)
    {
        LOG_ERROR("Failed to add imu track.", 0);
        return K4A_RESULT_FAILED;
    }

    context->imu_track->high_freq_data = true;

    uint64_t track_uid = GetChild<KaxTrackUID>(*context->imu_track->track).GetValue();
    std::ostringstream track_uid_str;
    track_uid_str << track_uid;
    add_tag(context, "K4A_IMU_TRACK", track_uid_str.str().c_str(), TAG_TARGET_TYPE_TRACK, track_uid);
    add_tag(context, "K4A_IMU_MODE", "ON", TAG_TARGET_TYPE_TRACK, track_uid);

    return K4A_RESULT_SUCCEEDED;
}

k4a_result_t k4a_record_add_custom_video_track(const k4a_record_t recording_handle,
                                               const char *track_name,
                                               const char *codec_id,
                                               const uint8_t *codec_context,
                                               size_t codec_context_size,
                                               const k4a_record_video_settings_t *track_settings)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, track_name == NULL);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, codec_id == NULL);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, track_settings == NULL);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (context->header_written)
    {
        LOG_ERROR("Custom tracks must be added before the recording header is written.", 0);
        return K4A_RESULT_FAILED;
    }

    track_header_t *track = add_track(context, track_name, track_video, codec_id, codec_context, codec_context_size);
    if (track == NULL)
    {
        LOG_ERROR("Failed to add custom video track: %s", track_name);
        return K4A_RESULT_FAILED;
    }
    set_track_info_video(track, track_settings->width, track_settings->height, track_settings->frame_rate);
    track->custom_track = true;

    uint64_t track_uid = GetChild<KaxTrackUID>(*track->track).GetValue();
    std::ostringstream track_tag_str, track_uid_str;
    track_tag_str << "K4A_CUSTOM_TRACK_" << track_name;
    track_uid_str << track_uid;
    add_tag(context, track_tag_str.str().c_str(), track_uid_str.str().c_str(), TAG_TARGET_TYPE_TRACK, track_uid);

    return K4A_RESULT_SUCCEEDED;
}

k4a_result_t k4a_record_add_custom_subtitle_track(const k4a_record_t recording_handle,
                                                  const char *track_name,
                                                  const char *codec_id,
                                                  const uint8_t *codec_context,
                                                  size_t codec_context_size,
                                                  const k4a_record_subtitle_settings_t *track_settings)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, track_name == NULL);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, codec_id == NULL);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (context->header_written)
    {
        LOG_ERROR("Custom tracks must be added before the recording header is written.", 0);
        return K4A_RESULT_FAILED;
    }

    track_header_t *track = add_track(context, track_name, track_subtitle, codec_id, codec_context, codec_context_size);
    if (track == NULL)
    {
        LOG_ERROR("Failed to add custom subtitle track: %s", track_name);
        return K4A_RESULT_FAILED;
    }
    if (track_settings != NULL)
    {
        track->high_freq_data = track_settings->high_freq_data;
    }
    track->custom_track = true;

    uint64_t track_uid = GetChild<KaxTrackUID>(*track->track).GetValue();
    std::ostringstream track_tag_str, track_uid_str;
    track_tag_str << "K4A_CUSTOM_TRACK_" << track_name;
    track_uid_str << track_uid;
    add_tag(context, track_tag_str.str().c_str(), track_uid_str.str().c_str(), TAG_TARGET_TYPE_TRACK, track_uid);

    return K4A_RESULT_SUCCEEDED;
}

k4a_result_t k4a_record_write_header(const k4a_record_t recording_handle)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (context->header_written)
    {
        LOG_ERROR("The header for this recording has already been written.", 0);
        return K4A_RESULT_FAILED;
    }

    try
    {
        // Make sure we're at the beginning of the file in case we're rewriting a file.
        context->ebml_file->setFilePointer(0, libebml::seek_beginning);

        { // Render Ebml header
            EbmlHead file_head;

            GetChild<EDocType>(file_head).SetValue("matroska");
            GetChild<EDocTypeVersion>(file_head).SetValue(2);
            GetChild<EDocTypeReadVersion>(file_head).SetValue(2);

            file_head.Render(*context->ebml_file, true);
        }

        // Recordings can get very large, so pad the length field up to 8 bytes from the start.
        context->file_segment->WriteHead(*context->ebml_file, 8);

        { // Write void blocks to reserve space for seeking metadata and the segment info so they can be updated at
          // the end
            context->seek_void = make_unique<EbmlVoid>();
            context->seek_void->SetSize(1024);
            context->seek_void->Render(*context->ebml_file);

            context->segment_info_void = make_unique<EbmlVoid>();
            context->segment_info_void->SetSize(256);
            context->segment_info_void->Render(*context->ebml_file);
        }

        { // Write tracks
            auto &tracks = GetChild<KaxTracks>(*context->file_segment);
            tracks.Render(*context->ebml_file);
        }

        { // Write attachments
            auto &attachments = GetChild<KaxAttachments>(*context->file_segment);
            attachments.Render(*context->ebml_file);
        }

        { // Write tags with a void block after to make editing easier
            auto &tags = GetChild<KaxTags>(*context->file_segment);
            tags.Render(*context->ebml_file);

            context->tags_void = make_unique<EbmlVoid>();
            context->tags_void->SetSize(1024);
            context->tags_void->Render(*context->ebml_file);
        }
    }
    catch (std::ios_base::failure &e)
    {
        LOG_ERROR("Failed to write recording header '%s': %s", context->file_path, e.what());
        return K4A_RESULT_FAILED;
    }

    RETURN_IF_ERROR(start_matroska_writer_thread(context));

    context->header_written = true;

    return K4A_RESULT_SUCCEEDED;
}

k4a_result_t k4a_record_write_capture(const k4a_record_t recording_handle, k4a_capture_t capture)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (!context->header_written)
    {
        LOG_ERROR("The recording header needs to be written before any captures.", 0);
        return K4A_RESULT_FAILED;
    }

    // Arrays used to map image formats to tracks, these 3 arrays are order dependant.
    k4a_image_t images[] = {
        k4a_capture_get_color_image(capture),
        k4a_capture_get_depth_image(capture),
        k4a_capture_get_ir_image(capture),
    };
    k4a_image_format_t expected_formats[] = { context->device_config.color_format,
                                              K4A_IMAGE_FORMAT_DEPTH16,
                                              K4A_IMAGE_FORMAT_IR16 };
    track_header_t *tracks[] = { context->color_track, context->depth_track, context->ir_track };
    static_assert(arraysize(images) == arraysize(tracks), "Invalid mapping from images to track");
    static_assert(arraysize(images) == arraysize(expected_formats), "Invalid mapping from images to formats");

    k4a_result_t result = K4A_RESULT_SUCCEEDED;
    for (size_t i = 0; i < arraysize(images); i++)
    {
        if (images[i])
        {
            size_t buffer_size = k4a_image_get_size(images[i]);
            uint8_t *image_buffer = k4a_image_get_buffer(images[i]);
            if (image_buffer != NULL && buffer_size > 0)
            {
                k4a_image_format_t image_format = k4a_image_get_format(images[i]);
                if (image_format == expected_formats[i])
                {
                    // Create a copy of the image buffer for writing to file.
                    assert(buffer_size <= UINT32_MAX);
                    DataBuffer *data_buffer = new (std::nothrow)
                        DataBuffer(image_buffer, (uint32)buffer_size, NULL, true);
                    if (image_format == K4A_IMAGE_FORMAT_DEPTH16 || image_format == K4A_IMAGE_FORMAT_IR16)
                    {
                        // 16 bit grayscale needs to be converted to big-endian in the file.
                        assert(data_buffer->Size() % sizeof(uint16_t) == 0);
                        uint16_t *data_buffer_raw = reinterpret_cast<uint16_t *>(data_buffer->Buffer());
                        for (size_t j = 0; j < data_buffer->Size() / sizeof(uint16_t); j++)
                        {
                            data_buffer_raw[j] = swap_bytes_16(data_buffer_raw[j]);
                        }
                    }

                    uint64_t timestamp_ns = k4a_image_get_device_timestamp_usec(images[i]) * 1000;
                    k4a_result_t tmp_result = TRACE_CALL(
                        write_track_data(context, tracks[i], timestamp_ns, data_buffer));
                    if (K4A_FAILED(tmp_result))
                    {
                        // Write as many of the image buffers as possible, even if some fail due to timestamp.
                        result = tmp_result;
                        data_buffer->FreeBuffer(*data_buffer);
                        delete data_buffer;
                    }
                }
                else
                {
                    LOG_ERROR("Tried to write capture with unexpected image format.", 0);
                    result = K4A_RESULT_FAILED;
                }
            }
            k4a_image_release(images[i]);
        }
    }

    return result;
}

k4a_result_t k4a_record_write_imu_sample(const k4a_record_t recording_handle, k4a_imu_sample_t imu_sample)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (!context->imu_track)
    {
        LOG_ERROR("The IMU track needs to be added with k4a_record_add_imu_track() before IMU samples can be written.",
                  0);
        return K4A_RESULT_FAILED;
    }

    if (!context->header_written)
    {
        LOG_ERROR("The recording header needs to be written before any imu samples.", 0);
        return K4A_RESULT_FAILED;
    }

    matroska_imu_sample_t sample_data = { 0 };
    sample_data.acc_timestamp_ns = imu_sample.acc_timestamp_usec * 1000;
    sample_data.gyro_timestamp_ns = imu_sample.gyro_timestamp_usec * 1000;
    for (size_t i = 0; i < 3; i++)
    {
        sample_data.acc_data[i] = imu_sample.acc_sample.v[i];
        sample_data.gyro_data[i] = imu_sample.gyro_sample.v[i];
    }

    DataBuffer *data_buffer = new (std::nothrow)
        DataBuffer(reinterpret_cast<binary *>(&sample_data), sizeof(matroska_imu_sample_t), NULL, true);
    k4a_result_t result = write_track_data(context, context->imu_track, sample_data.acc_timestamp_ns, data_buffer);
    if (K4A_FAILED(result))
    {
        data_buffer->FreeBuffer(*data_buffer);
        delete data_buffer;
        return result;
    }

    return K4A_RESULT_SUCCEEDED;
}

k4a_result_t k4a_record_write_custom_track_data(const k4a_record_t recording_handle,
                                                const char *track_name,
                                                uint64_t device_timestamp_usec,
                                                uint8_t *buffer,
                                                size_t buffer_size)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, track_name == NULL);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, buffer == NULL);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);

    if (!context->header_written)
    {
        LOG_ERROR("The recording header needs to be written before any track data.", 0);
        return K4A_RESULT_FAILED;
    }

    auto itr = context->tracks.find(track_name);
    if (itr == context->tracks.end())
    {
        LOG_ERROR("The custom track does not exist: %s", track_name);
        return K4A_RESULT_FAILED;
    }
    if (!itr->second.custom_track)
    {
        LOG_ERROR("Custom track data cannot be written to built-in track: %s", track_name);
        return K4A_RESULT_FAILED;
    }

    // Create a copy of the image buffer for writing to file.
    assert(buffer_size <= UINT32_MAX);
    DataBuffer *data_buffer = new DataBuffer(buffer, (uint32_t)buffer_size, NULL, true);

    k4a_result_t result = TRACE_CALL(
        write_track_data(context, &itr->second, device_timestamp_usec * 1000, data_buffer));
    if (K4A_FAILED(result))
    {
        // Clean up the data_buffer if write_track_data failed.
        data_buffer->FreeBuffer(*data_buffer);
        delete data_buffer;
    }

    return result;
}

k4a_result_t k4a_record_flush(const k4a_record_t recording_handle)
{
    RETURN_VALUE_IF_HANDLE_INVALID(K4A_RESULT_FAILED, k4a_record_t, recording_handle);

    k4a_result_t result = K4A_RESULT_SUCCEEDED;
    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, context == NULL);
    RETURN_VALUE_IF_ARG(K4A_RESULT_FAILED, !context->header_written);

    try
    {
        // Lock the writer thread first so we don't have conflicts
        std::lock_guard<std::mutex> writer_lock(context->writer_lock);

        LargeFileIOCallback *file_io = dynamic_cast<LargeFileIOCallback *>(context->ebml_file.get());
        if (file_io != NULL)
        {
            file_io->setOwnerThread();
        }

        std::lock_guard<std::mutex> cluster_lock(context->pending_cluster_lock);

        if (!context->pending_clusters.empty())
        {
            for (cluster_t *cluster : context->pending_clusters)
            {
                k4a_result_t write_result = TRACE_CALL(
                    write_cluster(context, cluster, &context->last_written_timestamp));
                if (K4A_FAILED(write_result))
                {
                    // Try to flush as much of the recording as possible to disk before returning any errors.
                    result = write_result;
                }
            }
            context->pending_clusters.clear();
        }

        auto &segment_info = GetChild<KaxInfo>(*context->file_segment);

        uint64_t current_position = context->ebml_file->getFilePointer();

        // Update segment info
        GetChild<KaxDuration>(segment_info)
            .SetValue(
                (double)((context->most_recent_timestamp - context->start_timestamp_offset) / context->timecode_scale));
        context->segment_info_void->ReplaceWith(segment_info, *context->ebml_file);

        // Render cues
        auto &cues = GetChild<KaxCues>(*context->file_segment);
        cues.Render(*context->ebml_file);

        // Update tags
        auto &tags = GetChild<KaxTags>(*context->file_segment);
        if (tags.GetElementPosition() > 0)
        {
            context->ebml_file->setFilePointer((int64_t)tags.GetElementPosition());
            tags.Render(*context->ebml_file);
            if (tags.GetEndPosition() != context->tags_void->GetElementPosition())
            {
                // Rewrite the void block after tags
                EbmlVoid tags_void;
                tags_void.SetSize(context->tags_void->GetSize() -
                                  (tags.GetEndPosition() - context->tags_void->GetElementPosition()));
                tags_void.Render(*context->ebml_file);
            }
        }

        { // Update seek info
            auto &seek_head = GetChild<KaxSeekHead>(*context->file_segment);
            // RemoveAll() has a bug and does not free the elements before emptying the list.
            for (auto element : seek_head.GetElementList())
            {
                delete element;
            }
            seek_head.RemoveAll(); // Remove any seek entries from previous flushes

            seek_head.IndexThis(segment_info, *context->file_segment);

            auto &tracks = GetChild<KaxTracks>(*context->file_segment);
            if (tracks.GetElementPosition() > 0)
            {
                seek_head.IndexThis(tracks, *context->file_segment);
            }

            auto &attachments = GetChild<KaxAttachments>(*context->file_segment);
            if (attachments.GetElementPosition() > 0)
            {
                seek_head.IndexThis(attachments, *context->file_segment);
            }

            if (tags.GetElementPosition() > 0)
            {
                seek_head.IndexThis(tags, *context->file_segment);
            }

            if (cues.GetElementPosition() > 0)
            {
                seek_head.IndexThis(cues, *context->file_segment);
            }

            context->seek_void->ReplaceWith(seek_head, *context->ebml_file);
        }

        // Update the file segment head to write the current size
        context->ebml_file->setFilePointer(0, seek_end);
        uint64 segment_size = context->ebml_file->getFilePointer() - context->file_segment->GetElementPosition() -
                              context->file_segment->HeadSize();
        // Segment size can only be set once normally, so force the flag.
        context->file_segment->SetSizeInfinite(true);
        if (!context->file_segment->ForceSize(segment_size))
        {
            LOG_ERROR("Failed set file segment size.", 0);
        }
        context->file_segment->OverwriteHead(*context->ebml_file);

        // Set the write pointer back in case we're not done recording yet.
        assert(current_position <= INT64_MAX);
        context->ebml_file->setFilePointer((int64_t)current_position);
    }
    catch (std::ios_base::failure &e)
    {
        LOG_ERROR("Failed to write recording '%s': %s", context->file_path, e.what());
        return K4A_RESULT_FAILED;
    }
    catch (std::system_error &e)
    {
        LOG_ERROR("Failed to flush recording '%s': %s", context->file_path, e.what());
        return K4A_RESULT_FAILED;
    }
    return result;
}

void k4a_record_close(const k4a_record_t recording_handle)
{
    RETURN_VALUE_IF_HANDLE_INVALID(VOID_VALUE, k4a_record_t, recording_handle);

    k4a_record_context_t *context = k4a_record_t_get_context(recording_handle);
    if (context != NULL)
    {
        // If the recording was started, flush any unwritten data.
        if (context->header_written)
        {
            // If these fail, there's nothing we can do but log.
            (void)TRACE_CALL(k4a_record_flush(recording_handle));
            stop_matroska_writer_thread(context);
        }

        try
        {
            context->ebml_file->close();
        }
        catch (std::ios_base::failure &e)
        {
            LOG_ERROR("Failed to close recording '%s': %s", context->file_path, e.what());
        }
    }
    k4a_record_t_destroy(recording_handle);
}
