/** \file record.h
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License.
 * Kinect For Azure Recording SDK.
 */

#ifndef K4A_RECORD_H
#define K4A_RECORD_H

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

/** Opens a new recording file for writing.
 *
 * \param path
 * Filesystem path for the new recording.
 *
 * \param device
 * The Azure Kinect device that is being recorded. The device handle is used to store device calibration and serial
 * number information. May be NULL if recording user-generated data.
 *
 * \param device_config
 * The configuration the Azure Kinect device was started with.
 *
 * \param recording_handle
 * If successful, this contains a pointer to the new recording handle. Caller must call k4a_record_close()
 * when finished with recording.
 *
 * \remarks
 * The file will be created if it doesn't exist, or overwritten if an existing file is specified.
 *
 * \remarks
 * Streaming does not need to be started on the device at the time this function is called, but when it is started
 * it should be started with the same configuration provided in \p device_config.
 *
 * \remarks
 * Subsequent calls to k4a_record_write_capture() will need to have images in the resolution and format defined
 * in \p device_config.
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \relates k4a_record_t
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_create(const char *path,
                                                k4a_device_t device,
                                                const k4a_device_configuration_t device_config,
                                                k4a_record_t *recording_handle);

/** Adds a tag to the recording.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \param name
 * The name of the tag to write.
 *
 * \param value
 * The string value to store in the tag.
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success.
 *
 * \remarks
 * Tags are global to a file, and should store data related to the entire recording, such as camera configuration or
 * recording location.
 *
 * \remarks
 * Tag names must be ALL CAPS and may only contain A-Z, 0-9, '-' and '_'.
 *
 * \remarks
 * All tags need to be added before the recording header is written.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_add_tag(k4a_record_t recording_handle, const char *name, const char *value);

/** Adds the track header for recording IMU.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \remarks
 * The track needs to be added before the recording header is written.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_add_imu_track(k4a_record_t recording_handle);

/** Adds an attachment to the recording.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \param attachment_name
 * The name of the attachment to be stored in the recording file. This name should be a valid filename with an
 * extension.
 *
 * \param buffer
 * The attachment data buffer.
 *
 * \param buffer_size
 * The size of the attachment data buffer.
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \remarks
 * All attachments need to be added before the recording header is written.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_add_attachment(const k4a_record_t recording_handle,
                                                        const char *attachment_name,
                                                        const uint8_t *buffer,
                                                        size_t buffer_size);

/** Adds custom video tracks to the recording.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \param track_name
 * The name of the custom video track to be added.
 *
 * \param codec_id
 * A UTF8 null terminated string containing the codec ID of the track. Some of the existing formats are listed here:
 * https://www.matroska.org/technical/specs/codecid/index.html. The codec ID can also be custom defined by the user.
 * Video codec ID's should start with 'V_'.
 *
 * \param codec_context
 * The codec context is a codec-specific buffer that contains any required codec metadata that is only known to the
 * codec. It is mapped to the matroska 'CodecPrivate' element.
 *
 * \param codec_context_size
 * The size of the codec context buffer.
 *
 * \param track_settings
 * Additional metadata for the video track such as resolution and framerate.
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \remarks
 * Built-in video tracks like the DEPTH, IR, and COLOR tracks will be created automatically when the k4a_record_create()
 * API is called. This API can be used to add additional video tracks to save custom data.
 *
 * \remarks
 * Track names must be ALL CAPS and may only contain A-Z, 0-9, '-' and '_'.
 *
 * \remarks
 * All tracks need to be added before the recording header is written.
 *
 * \remarks
 * Call k4a_record_write_custom_track_data() with the same track_name to write data to this track.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_add_custom_video_track(const k4a_record_t recording_handle,
                                                                const char *track_name,
                                                                const char *codec_id,
                                                                const uint8_t *codec_context,
                                                                size_t codec_context_size,
                                                                const k4a_record_video_settings_t *track_settings);

/** Adds custom subtitle tracks to the recording.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \param track_name
 * The name of the custom subtitle track to be added.
 *
 * \param codec_id
 * A UTF8 null terminated string containing the codec ID of the track. Some of the existing formats are listed here:
 * https://www.matroska.org/technical/specs/codecid/index.html. The codec ID can also be custom defined by the user.
 * Subtitle codec ID's should start with 'S_'.
 *
 * \param codec_context
 * The codec context is a codec-specific buffer that contains any required codec metadata that is only known to the
 * codec. It is mapped to the matroska 'CodecPrivate' element.
 *
 * \param codec_context_size
 * The size of the codec context buffer.
 *
 * \param track_settings
 * Additional metadata for the subtitle track. If NULL, the default settings will be used.
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \remarks
 * Built-in subtitle tracks like the IMU track will be created automatically when the k4a_record_add_imu_track() API is
 * called. This API can be used to add additional subtitle tracks to save custom data.
 *
 * \remarks
 * Track names must be ALL CAPS and may only contain A-Z, 0-9, '-' and '_'.
 *
 * \remarks
 * All tracks need to be added before the recording header is written.
 *
 * \remarks
 * Call k4a_record_write_custom_track_data() with the same track_name to write data to this track.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t
k4a_record_add_custom_subtitle_track(const k4a_record_t recording_handle,
                                     const char *track_name,
                                     const char *codec_id,
                                     const uint8_t *codec_context,
                                     size_t codec_context_size,
                                     const k4a_record_subtitle_settings_t *track_settings);

/** Writes the recording header and metadata to file.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \remarks
 * This must be called before captures or any track data can be written.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_write_header(k4a_record_t recording_handle);

/** Writes a camera capture to file.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \param capture_handle
 * The handle of a capture to write to file.
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \remarks
 * Captures must be written in increasing order of timestamp, and the file's header must already be written.
 *
 * \remarks
 * k4a_record_write_capture() will write all images in the capture to the corresponding tracks in the recording file.
 * If any of the images fail to write, other images will still be written before a failure is returned.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_write_capture(k4a_record_t recording_handle, k4a_capture_t capture_handle);

/** Writes an imu sample to file.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \param imu_sample
 * A structure containing the imu sample data and timestamps.
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \remarks
 * Samples must be written in increasing order of timestamp, and the file's header must already be written.
 *
 * \remarks
 * When writing imu samples at the same time as captures, the samples should be within 1 second of the most recently
 * written capture.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_write_imu_sample(k4a_record_t recording_handle, k4a_imu_sample_t imu_sample);

/** Writes data for a custom track to file.
 *
 * \param recording_handle
 * The handle of a new recording, obtained by k4a_record_create().
 *
 * \param track_name
 * The name of the custom track that the data is going to be written to.
 *
 * \param device_timestamp_usec
 * The timestamp in microseconds for the custom track data. This timestamp should be in the same time domain as the
 * device timestamp used for recording.
 *
 * \param custom_data
 * The buffer of custom track data.
 *
 * \param custom_data_size
 * The size of the custom track data buffer.
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success
 *
 * \remarks
 * Custom track data must be written in increasing order of timestamp, and the file's header must already be written.
 * When writing custom track data at the same time as captures or IMU data, the custom data should be within 1 second of
 * the most recently written timestamp.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_write_custom_track_data(const k4a_record_t recording_handle,
                                                                 const char *track_name,
                                                                 uint64_t device_timestamp_usec,
                                                                 uint8_t *custom_data,
                                                                 size_t custom_data_size);

/** Flushes all pending recording data to disk.
 *
 * \param recording_handle
 * Handle obtained by k4a_record_create().
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \returns ::K4A_RESULT_SUCCEEDED is returned on success, or ::K4A_RESULT_FAILED if an error occurred.
 *
 * \remarks
 * k4a_record_flush() ensures that all data passed to the recording API prior to calling flush is written to disk.
 * If continuing to write recording data, care must be taken to ensure no new timestamps are added from before the
 * flush.
 *
 * \remarks
 * If an error occurs, best effort is made to flush as much data to disk as possible, but the integrity of the file is
 * not guaranteed.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_flush(k4a_record_t recording_handle);

/** Closes a recording handle.
 *
 * \param recording_handle
 * Handle obtained by k4a_record_create().
 *
 * \headerfile record.h <k4arecord/record.h>
 *
 * \relates k4a_record_t
 *
 * \remarks
 * If there is any unwritten data it will be flushed to disk before closing the recording.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">record.h (include k4arecord/record.h)</requirement>
 *   <requirement name="Library">k4arecord.lib</requirement>
 *   <requirement name="DLL">k4arecord.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT void k4a_record_close(k4a_record_t recording_handle);

/** Sets and clears the callback function to receive debug messages from the Azure Kinect record and playback APIs.
 *
 * \param message_cb
 * The callback function to receive messages from. Set to NULL to unregister the callback function.
 *
 * \param message_cb_context
 * The callback functions context.
 *
 * \param min_level
 * The least critical error the user wants to be notified about.
 *
 * \return ::K4A_RESULT_SUCCEEDED if the callback function was set or cleared successfully. ::K4A_RESULT_FAILED if an
 * error is encountered or the callback function has already been set.
 *
 * \remarks
 * Call this function to set or clear the callback function that is used to deliver debug messages to the caller. This
 * callback may be called concurrently, it is up to the implementation of the callback function to ensure the
 * parallelization is handled.
 *
 * \remarks
 * Clearing the callback function will block until all pending calls to the callback function have completed.
 *
 * \remarks
 * To update \p min_level, k4a_record_set_debug_message_handler() can be called with the same value \p message_cb and by
 * specifying a new \p min_level.
 *
 * \remarks
 * Logging provided via this API is independent of the logging controlled by the environmental variable controls \p
 * K4A_ENABLE_LOG_TO_STDOUT, K4A_RECORD_ENABLE_LOG_TO_A_FILE, and K4A_LOG_LEVEL. However there is a slight change in
 * default behavior when using this function. By default, when k4a_record_set_debug_message_handler() has not been used
 * to register a message callback, the default for environmental variable controls is to send debug messages as if
 * K4A_ENABLE_LOG_TO_STDOUT=1 were set. If k4a_record_set_debug_message_handler registers a callback function before
 * k4a_record_create() or k4a_playback_create() is called, then the default for environmental controls is as if
 * K4A_ENABLE_LOG_TO_STDOUT=0 was specified. Physically specifying the environmental control will override the default.
 *
 * \xmlonly
 * <requirements>
 *   <requirement name="Header">k4a.h (include k4a/k4a.h)</requirement>
 *   <requirement name="Library">k4a.lib</requirement>
 *   <requirement name="DLL">k4a.dll</requirement>
 * </requirements>
 * \endxmlonly
 */
K4ARECORD_EXPORT k4a_result_t k4a_record_set_debug_message_handler(k4a_logging_message_cb_t *message_cb,
                                                                   void *message_cb_context,
                                                                   k4a_log_level_t min_level);
/**
 * @}
 */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* K4A_RECORD_H */
