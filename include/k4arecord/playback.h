/** \file playback.h
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License.
 * Kinect For Azure Recording Playback SDK.
 */

#ifndef K4A_PLAYBACK_H
#define K4A_PLAYBACK_H

#include <k4arecord/types.h>
#include <k4arecord/k4arecord_export.h>

#ifdef __cplusplus

extern "C" {
#endif

/**
 *
 * \addtogroup Functions
 *
 * @{
 */

/** Opens an existing recording file for reading.
 *
 * \param path
 * Filesystem path of the existing recording.
 *
 * \param playback_handle
 * If successful, this contains a pointer to the recording handle. Caller must call k4a_playback_close() when
 * finished with the recording.
 *
 * \headerfile playback.h <k4arecord/playback.h>
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \relates k4a_playback_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_playback_open(const char *path, k4a_playback_t *playback_handle);

/** Get the raw calibration blob for the Azure Kinect device used during recording.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param data
 * Location to write the calibration data to. This field may optionally be set to NULL if the caller wants to query for
 * the needed data size.
 *
 * \param data_size
 * On passing \p data_size into the function this variable represents the available size to write the raw data to. On
 * return this variable is updated with the amount of data actually written to the buffer.
 *
 * \returns
 * ::K4A_BUFFER_RESULT_SUCCEEDED if \p data was successfully written. If \p data_size points to a buffer size that is
 * too small to hold the output, ::K4A_BUFFER_RESULT_TOO_SMALL is returned and \p data_size is updated to contain the
 * minimum buffer size needed to capture the calibration data.
 *
 * \remarks
 * The raw calibration may not exist if the device was not specified during recording.
 *
 * \relates k4a_playback_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_buffer_result_t k4a_playback_get_raw_calibration(k4a_playback_t playback_handle,
                                                                      uint8_t *data,
                                                                      size_t *data_size);

/** Get the camera calibration for Azure Kinect device used during recording. The output struct is used as input to all
 * transformation functions.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param calibration
 * Location to write the calibration.
 *
 * \returns
 * ::K4A_RESULT_SUCCEEDED if \p calibration was successfully written. ::K4A_RESULT_FAILED otherwise.
 *
 * \remarks
 * The calibration may not exist if the device was not specified during recording.
 *
 * \relates k4a_playback_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_playback_get_calibration(k4a_playback_t playback_handle,
                                                           k4a_calibration_t *calibration);

/** Get the device configuration used during recording.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param config
 * Location to write the recording configuration.
 *
 * \returns
 * ::K4A_RESULT_SUCCEEDED if \p config was successfully written. ::K4A_RESULT_FAILED otherwise.
 *
 * \relates k4a_playback_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_playback_get_record_configuration(k4a_playback_t playback_handle,
                                                                    k4a_record_configuration_t *config);

/** Checks whether a track with the given track name exists in the playback file.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param track_name
 * The track name to be checked to see whether it exists or not.
 *
 * \returns true if the track exists.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT bool k4a_playback_check_track_exists(k4a_playback_t playback_handle, const char *track_name);

/** Get the number of tracks in a playback file.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \returns the number of tracks in the playback file.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT size_t k4a_playback_get_track_count(k4a_playback_t playback_handle);

/** Gets the name of a track at a specific index.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param track_index
 * The index of the track to read the name form.
 *
 * \param track_name
 * Location to write the track name. This will be a UTF8 null terminated string. If a NULL buffer is specified,
 * \p track_name_size will be set to the size of buffer needed to store the string.
 *
 * \param track_name_size
 * On input, the size of the \p track_name buffer. On output, this is set to the length of the track_name value
 * (including the null terminator).
 *
 * \returns
 * A return of ::K4A_BUFFER_RESULT_SUCCEEDED means that the \p track_name has been filled in. If the buffer is too small
 * the function returns ::K4A_BUFFER_RESULT_TOO_SMALL and the needed size of the \p track_name buffer is returned in the
 * \p track_name_size parameter. ::K4A_BUFFER_RESULT_FAILED is returned if the track index does not exist. All other
 * failures return ::K4A_BUFFER_RESULT_FAILED.
 *
 * \remarks
 * When used along with k4a_playback_get_track_count(), this function can be used to enumerate all the available tracks
 * in a playback file. Additionally k4a_playback_track_is_builtin() can be used to filter custom tracks.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_buffer_result_t k4a_playback_get_track_name(k4a_playback_t playback_handle,
                                                                 size_t track_index,
                                                                 char *track_name,
                                                                 size_t *track_name_size);

/** Checks whether a track is one of the built-in tracks: "COLOR", "DEPTH", etc...
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param track_name
 * The track name to be checked to see whether it is a built-in track.
 *
 * \returns true if the track is built-in. If the provided track name does not exist, false will be returned.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT bool k4a_playback_track_is_builtin(k4a_playback_t playback_handle, const char *track_name);

/** Gets the video-specific track information for a particular video track.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param track_name
 * The track name to read video settings from.
 *
 * \param video_settings
 * Location to write the track's video settings.
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success, ::K4A_RESULT_FAILED is returned if the specified track does
 * not exist or is not a video track.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_playback_track_get_video_settings(k4a_playback_t playback_handle,
                                                                    const char *track_name,
                                                                    k4a_record_video_settings_t *video_settings);

/** Gets the codec id string for a particular track.
 *
 * The codec ID is a string that corresponds to the codec of the track's data. Some of the existing formats are listed
 * here: https://www.matroska.org/technical/specs/codecid/index.html. It can also be custom defined by the user.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param track_name
 * The track name to read the codec id from.
 *
 * \param codec_id
 * Location to write the codec id. This will be a UTF8 null terminated string. If a NULL buffer is specified,
 * \p codec_id_size will be set to the size of buffer needed to store the string.
 *
 * \param codec_id_size
 * On input, the size of the \p codec_id buffer. On output, this is set to the length of the codec_id value (including
 * the null terminator).
 *
 * \returns
 * A return of ::K4A_BUFFER_RESULT_SUCCEEDED means that the \p codec_id has been filled in. If the buffer is too small
 * the function returns ::K4A_BUFFER_RESULT_TOO_SMALL and the needed size of the \p codec_id buffer is returned in the
 * \p codec_id_size parameter. ::K4A_BUFFER_RESULT_FAILED is returned if the track_name does not exist. All other
 * failures return ::K4A_BUFFER_RESULT_FAILED.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_buffer_result_t k4a_playback_track_get_codec_id(k4a_playback_t playback_handle,
                                                                     const char *track_name,
                                                                     char *codec_id,
                                                                     size_t *codec_id_size);

/** Gets the codec context for a particular track.
 *
 * The codec context is a codec-specific buffer that contains any required codec metadata that is only known to the
 * codec. It is mapped to the matroska Codec Private field.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param track_name
 * The track name to read the codec context from.
 *
 * \param codec_context
 * Location to write the codec context data. If a NULL buffer is specified, \p codec_context_size will be set to the
 * size of buffer needed to store the data.
 *
 * \param codec_context_size
 * On input, the size of the \p codec_context buffer. On output, this is set to the length of the codec_context data.
 *
 * \returns
 * A return of ::K4A_BUFFER_RESULT_SUCCEEDED means that the \p codec_context has been filled in. If the buffer is too
 * small the function returns ::K4A_BUFFER_RESULT_TOO_SMALL and the needed size of the \p codec_context buffer is
 * returned in the \p codec_context_size parameter. ::K4A_BUFFER_RESULT_FAILED is returned if the track_name does not
 * exist. All other failures return ::K4A_BUFFER_RESULT_FAILED.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_buffer_result_t k4a_playback_track_get_codec_context(k4a_playback_t playback_handle,
                                                                          const char *track_name,
                                                                          uint8_t *codec_context,
                                                                          size_t *codec_context_size);

/** Read the value of a tag from a recording.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param name
 * The name of the tag to read.
 *
 * \param value
 * Location to write the tag value. This will be a UTF8 null terminated string. If a NULL buffer is specified,
 * \p value_size will be set to the size of buffer needed to store the string.
 *
 * \param value_size
 * On input, the size of the \p value buffer. On output, this is set to the length of the tag value (including the null
 * terminator).
 *
 * \returns
 * A return of ::K4A_BUFFER_RESULT_SUCCEEDED means that the \p value has been filled in. If the buffer is too small the
 * function returns ::K4A_BUFFER_RESULT_TOO_SMALL and the needed size of the \p value buffer is returned in the
 * \p value_size parameter. ::K4A_BUFFER_RESULT_FAILED is returned if the tag does not exist. All other failures return
 * ::K4A_BUFFER_RESULT_FAILED.
 *
 * \remarks
 * Tags are global to a file, and should store data related to the entire recording, such as camera configuration or
 * recording location.
 *
 * \relates k4a_playback_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_buffer_result_t k4a_playback_get_tag(k4a_playback_t playback_handle,
                                                          const char *name,
                                                          char *value,
                                                          size_t *value_size);

/** Set the image format that color captures will be converted to. By default the conversion format will be the same as
 * the image format stored in the recording file, and no conversion will occur.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param target_format
 * The target image format to be returned in captures.
 *
 * \returns
 * ::K4A_RESULT_SUCCEEDED if the format conversion is supported. ::K4A_RESULT_FAILED otherwise.
 *
 * \remarks
 * After the color conversion format is set, all \ref k4a_capture_t objects returned from the playback handle will have
 * their color images converted to the \p target_format.
 *
 * \remarks
 * Color format conversion occurs in the user-thread, so setting \p target_format to anything other than the format
 * stored in the file may significantly increase the latency of \p k4a_playback_get_next_capture() and
 * \p k4a_playback_get_previous_capture().
 *
 * \relates k4a_playback_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_playback_set_color_conversion(k4a_playback_t playback_handle,
                                                                k4a_image_format_t target_format);

/** Reads an attachment file from a recording.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param file_name
 * The attachment file name.
 *
 * \param data
 * Location to write the attachment data. If a NULL buffer is specified, \p data_size will be set to the size of
 * buffer needed to store the data.
 *
 * \param data_size
 * On input, the size of the \p data buffer. On output, this is set to the length of the attachment data.
 *
 * \returns
 * A return of ::K4A_BUFFER_RESULT_SUCCEEDED means that the \p data has been filled in. If the buffer is too small the
 * function returns ::K4A_BUFFER_RESULT_TOO_SMALL and the needed size of the \p data buffer is returned in the
 * \p data_size parameter. ::K4A_BUFFER_RESULT_FAILED is returned if the attachment \p file_name does not exist. All
 * other failures return ::K4A_BUFFER_RESULT_FAILED.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_buffer_result_t k4a_playback_get_attachment(k4a_playback_t playback_handle,
                                                                 const char *file_name,
                                                                 uint8_t *data,
                                                                 size_t *data_size);

/** Read the next capture in the recording sequence.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param capture_handle
 * If successful this contains a handle to a capture object. Caller must call k4a_capture_release() when its done using
 * this capture
 *
 * \returns
 * ::K4A_STREAM_RESULT_SUCCEEDED if a capture is returned, or ::K4A_STREAM_RESULT_EOF if the end of the recording is
 * reached. All other failures will return ::K4A_STREAM_RESULT_FAILED.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * k4a_playback_get_next_capture() always returns the next capture in sequence after the most recently returned capture.
 *
 * \remarks
 * The first call to k4a_playback_get_next_capture() after k4a_playback_seek_timestamp() will return the capture
 * in the recording closest to the seek time with an image timestamp greater than or equal to the seek time.
 *
 * \remarks
 * If a call was made to k4a_playback_get_previous_capture() that returned ::K4A_STREAM_RESULT_EOF, the playback
 * position is at the beginning of the stream and k4a_playback_get_next_capture() will return the first capture in the
 * recording.
 *
 * \remarks
 * Capture objects returned by the playback API will always contain at least one image, but may have images missing if
 * frames were dropped in the original recording. When calling k4a_capture_get_color_image(),
 * k4a_capture_get_depth_image(), or k4a_capture_get_ir_image(), the image should be checked for NULL.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_stream_result_t k4a_playback_get_next_capture(k4a_playback_t playback_handle,
                                                                   k4a_capture_t *capture_handle);

/** Read the previous capture in the recording sequence.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param capture_handle
 * If successful this contains a handle to a capture object. Caller must call k4a_capture_release() when its done using
 * this capture.
 *
 * \returns
 * ::K4A_STREAM_RESULT_SUCCEEDED if a capture is returned, or ::K4A_STREAM_RESULT_EOF if the start of the recording is
 * reached. All other failures will return ::K4A_STREAM_RESULT_FAILED.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * k4a_playback_get_previous_capture() always returns the previous capture in the sequence before the most
 * recently returned capture.
 *
 * \remarks
 * If a call was made to k4a_playback_get_next_capture() that returned ::K4A_STREAM_RESULT_EOF, the playback position
 * is at the end of the stream and k4a_playback_get_previous_capture() will return the last capture in
 * the recording.
 *
 * \remarks
 * The first call to k4a_playback_get_previous_capture() after k4a_playback_seek_timestamp() will return the
 * capture in the recording closest to the seek time with all image timestamps less than the seek time.
 *
 * \remarks
 * Capture objects returned by this API will always contain at least one image, but may have images missing if frames
 * were dropped in the original recording. When calling k4a_capture_get_color_image(), k4a_capture_get_depth_image(), or
 * k4a_capture_get_ir_image(), the image should be checked for NULL.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_stream_result_t k4a_playback_get_previous_capture(k4a_playback_t playback_handle,
                                                                       k4a_capture_t *capture_handle);

/** Read the next IMU sample in the recording sequence.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param imu_sample
 * The location to write the IMU sample.
 *
 * \returns
 * ::K4A_STREAM_RESULT_SUCCEEDED if a sample is returned, or ::K4A_STREAM_RESULT_EOF if the end of the recording is
 * reached. All other failures will return ::K4A_STREAM_RESULT_FAILED.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * k4a_playback_get_next_imu_sample() always returns the IMU sample after the most recently returned sample.
 *
 * \remarks
 * If a call was made to k4a_playback_get_previous_imu_sample() which returned ::K4A_STREAM_RESULT_EOF, then the
 * playback position is at the beginning of the recording and k4a_playback_get_next_imu_sample() will return the first
 * IMU sample in the recording.
 *
 * \remarks
 * The first call to k4a_playback_get_next_imu_sample() after k4a_playback_seek_timestamp() will return the IMU
 * sample in the recording closest to the seek time with a timestamp greater than or equal to the seek time.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_stream_result_t k4a_playback_get_next_imu_sample(k4a_playback_t playback_handle,
                                                                      k4a_imu_sample_t *imu_sample);

/** Read the previous IMU sample in the recording sequence.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param imu_sample [OUT]
 * The location to write the IMU sample.
 *
 * \returns
 * ::K4A_STREAM_RESULT_SUCCEEDED if a sample is returned, or ::K4A_STREAM_RESULT_EOF if the start of the recording is
 * reached. All other failures will return ::K4A_STREAM_RESULT_FAILED.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * k4a_playback_get_previous_imu_sample() always returns the IMU sample before the most recently returned sample.
 *
 * \remarks
 * If a call was made to to k4a_playback_get_next_imu_sample() which returned ::K4A_STREAM_RESULT_EOF, then the playback
 * position is at the end of the recording and k4a_playback_get_previous_imu_sample() will return the last IMU sample in
 * the recording.
 *
 * \remarks
 * The first call to k4a_playback_get_previous_imu_sample() after k4a_playback_seek_timestamp() will return the
 * IMU sample closest to the seek time with a timestamp less than the seek time.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_stream_result_t k4a_playback_get_previous_imu_sample(k4a_playback_t playback_handle,
                                                                          k4a_imu_sample_t *imu_sample);

/** Read the next data block for a particular track.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param track_name
 * The name of the track to read the next data block from.
 *
 * \param data_block_handle
 * The location to write the data block handle.
 *
 * \returns
 * ::K4A_STREAM_RESULT_SUCCEEDED if a data block is returned, or ::K4A_STREAM_RESULT_EOF if the end of the recording is
 * reached. All other failures will return ::K4A_STREAM_RESULT_FAILED.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * k4a_playback_get_next_data_block() always returns the data block after the most recently returned data block for a
 * particular track.
 *
 * \remarks
 * If a call was made to k4a_playback_get_previous_data_block() which returned ::K4A_STREAM_RESULT_EOF, then the
 * playback position is at the beginning of the recording and calling k4a_playback_get_next_data_block() with the same
 * track will return the first data block in the track.
 *
 * \remarks
 * The first call to k4a_playback_get_next_data_block() after k4a_playback_seek_timestamp() will return the data
 * block in the recording closest to the seek time with a timestamp greater than or equal to the seek time.
 *
 * \remarks
 * k4a_playback_get_next_data_block() cannot be used with the built-in tracks: "COLOR", "DEPTH", etc...
 * k4a_playback_track_is_builtin() can be used to determine if a track is a built-in track.
 *
 * \remarks
 * If the call is successful, callers must call k4a_playback_data_block_release() to return the allocated memory for
 * data_block_handle.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_stream_result_t k4a_playback_get_next_data_block(k4a_playback_t playback_handle,
                                                                      const char *track_name,
                                                                      k4a_playback_data_block_t *data_block_handle);

/** Read the previous data block for a particular track.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param track_name
 * The name of the track to read the previous data block from.
 *
 * \param data_block_handle
 * The location to write the data block.
 *
 * \returns
 * ::K4A_STREAM_RESULT_SUCCEEDED if a sample is returned, or ::K4A_STREAM_RESULT_EOF if the start of the recording is
 * reached. All other failures will return ::K4A_STREAM_RESULT_FAILED.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * k4a_playback_get_previous_data_block() always returns the data block before the most recently returned data block for
 * a particular track.
 *
 * \remarks
 * If a call was made to to k4a_playback_get_next_data_block() which returned ::K4A_STREAM_RESULT_EOF, then the playback
 * position is at the end of the recording and calling k4a_playback_get_previous_data_block() with the same track will
 * return the last data block in the track.
 *
 * \remarks
 * The first call to k4a_playback_get_previous_data_block() after k4a_playback_seek_timestamp() will return the
 * data block closest to the seek time with a timestamp less than the seek time.
 *
 * \remarks
 * If the call is successful, callers must call k4a_playback_data_block_release() to return the allocated memory for
 * data_block_handle.
 *
 * \remarks
 * k4a_playback_get_previous_data_block() cannot be used with the built-in tracks: "COLOR", "DEPTH", etc...
 * k4a_playback_track_is_builtin() can be used to determine if a track is a built-in track.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_stream_result_t k4a_playback_get_previous_data_block(k4a_playback_t playback_handle,
                                                                          const char *track_name,
                                                                          k4a_playback_data_block_t *data_block_handle);

/** Get the device timestamp of a data block in microseconds.
 *
 * \param data_block_handle
 * Handle obtained by k4a_playback_get_next_data_block() or k4a_playback_get_previous_data_block().
 *
 * \returns
 * Returns the device timestamp of the data block. If the \p data_block_handle is invalid this function will return 0.
 * It is also possible for 0 to be a valid timestamp originating from when a device was first powered on.
 *
 * \relates k4a_playback_data_block_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT uint64_t
k4a_playback_data_block_get_device_timestamp_usec(k4a_playback_data_block_t data_block_handle);

/** Get the buffer size of a data block.
 *
 * \param data_block_handle
 * Handle obtained by k4a_playback_get_next_data_block() or k4a_playback_get_previous_data_block().
 *
 * \returns
 * Returns the buffer size of the data block, or 0 if the data block is invalid.
 *
 * \relates k4a_playback_data_block_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT size_t k4a_playback_data_block_get_buffer_size(k4a_playback_data_block_t data_block_handle);

/** Get the buffer of a data block.
 *
 * \param data_block_handle
 * Handle obtained by k4a_playback_get_next_data_block() or k4a_playback_get_previous_data_block().
 *
 * \remarks
 * Use this buffer to access the data written to a custom recording track.
 *
 * \returns
 * Returns a pointer to the data block buffer, or NULL if the data block is invalid.
 *
 * \relates k4a_playback_data_block_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT uint8_t *k4a_playback_data_block_get_buffer(k4a_playback_data_block_t data_block_handle);

/** Release a data block handle.
 *
 * \param data_block_handle
 * Handle obtained by k4a_playback_get_next_data_block() or k4a_playback_get_previous_data_block().
 *
 * \remarks
 * Release the memory of a data block. The caller must not access the object after it is released.
 *
 * \relates k4a_playback_data_block_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT void k4a_playback_data_block_release(k4a_playback_data_block_t data_block_handle);

/** Seek to a specific timestamp within a recording.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \param offset_usec
 * The timestamp offset to seek to, relative to \p origin
 *
 * \param origin
 * Specifies how the given timestamp should be interpreted. Seek can be done relative to the beginning or end of the
 * recording, or using an absolute device timestamp.
 *
 * \returns
 * ::K4A_RESULT_SUCCEEDED if the seek operation was successful, or ::K4A_RESULT_FAILED if an error occured. The current
 * seek position is left unchanged if a failure is returned.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * The first device timestamp in a recording is usually non-zero. The recording file starts at the device timestamp
 * defined by start_timestamp_offset_usec, which is accessible via k4a_playback_get_record_configuration().
 *
 * \remarks
 * The first call to k4a_playback_get_next_capture() after k4a_playback_seek_timestamp() will return the first capture
 * containing an image timestamp greater than or equal to the seek time.
 *
 * \remarks
 * The first call to k4a_playback_get_previous_capture() after k4a_playback_seek_timestamp() will return the first
 * capture with all image timestamps less than the seek time.
 *
 * \remarks
 * The first call to k4a_playback_get_next_imu_sample() after k4a_playback_seek_timestamp() will return the first imu
 * sample with a timestamp greater than or equal to the seek time.
 *
 * \remarks
 * The first call to k4a_playback_get_previous_imu_sample() after k4a_playback_seek_timestamp() will return the first
 * imu sample with a timestamp less than the seek time.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_playback_seek_timestamp(k4a_playback_t playback_handle,
                                                          int64_t offset_usec,
                                                          k4a_playback_seek_origin_t origin);

/** Returns the length of the recording in microseconds.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \returns
 * The recording length, calculated as the difference between the first and last timestamp in the file.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * The recording length may be longer than an individual track if, for example, the IMU continues to run after the last
 * color image is recorded.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT uint64_t k4a_playback_get_recording_length_usec(k4a_playback_t playback_handle);

/** Gets the last timestamp in a recording, relative to the start of the recording.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \returns
 * The file timestamp of the last capture image or IMU sample in microseconds.
 *
 * \relates k4a_playback_t
 *
 * \remarks
 * This function returns a file timestamp, not an absolute device timestamp, meaning it is relative to the start of the
 * recording. This function is equivalent to the length of the recording.
 *
 * \deprecated
 * Deprecated starting in 1.2.0. Please use k4a_playback_get_recording_length_usec().
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_DEPRECATED_EXPORT uint64_t k4a_playback_get_last_timestamp_usec(k4a_playback_t playback_handle);

/** Closes a recording playback handle.
 *
 * \param playback_handle
 * Handle obtained by k4a_playback_open().
 *
 * \headerfile playback.h <k4arecord/playback.h>
 *
 * \relates k4a_playback_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">playback.h (include k4arecord/playback.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT void k4a_playback_close(k4a_playback_t playback_handle);

/**
 * @}
 */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* K4A_PLAYBACK_H */
