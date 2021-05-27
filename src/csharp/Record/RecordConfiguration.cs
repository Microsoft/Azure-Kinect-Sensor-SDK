//------------------------------------------------------------------------------
// <copyright file="RecordConfiguration.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;

namespace Microsoft.Azure.Kinect.Sensor.Record
{
    /// <summary>
    /// Structure containing the device configuration used to record.
    /// </summary>
    public class RecordConfiguration
    {
        /// <summary>
        /// Gets or sets the image format used to record the color camera.
        /// </summary>
        public ImageFormat ColorFormat { get; set; }

        /// <summary>
        /// Gets or sets the image resolution used to record the color camera.
        /// </summary>
        public ColorResolution ColorResolution { get; set; }

        /// <summary>
        /// Gets or sets the mode used to record the depth camera.
        /// </summary>
        public DepthMode DepthMode { get; set; }

        /// <summary>
        /// Gets or sets the frame rate used to record the color and depth camera.
        /// </summary>
        public FPS CameraFPS { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the recording contains Color camera frames.
        /// </summary>
        public bool ColorTrackEnabled { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the recording contains Depth camera frames.
        /// </summary>
        public bool DepthTrackEnabled { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the recording contains IR camera frames.
        /// </summary>
        public bool IRTrackEnabled { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether the recording contains IMU sample data.
        /// </summary>
        public bool ImuTrackEnabled { get; set; }

        /// <summary>
        /// Gets or sets the delay between color and depth images in the recording.
        /// </summary>
        /// <remarks>
        /// A negative delay means depth images are first, and a positive delay means color images are first.
        /// </remarks>
        public TimeSpan DepthDelayOffColor { get; set; }

        /// <summary>
        /// Gets or sets external synchronization mode.
        /// </summary>
        public WiredSyncMode WiredSyncMode { get; set; }

        /// <summary>
        /// Gets or sets the delay between this recording and the externally synced master camera.
        /// </summary>
        /// <remarks>
        /// This value is 0 unless <see cref="WiredSyncMode"/> is set to <see cref="WiredSyncMode.Subordinate"/>.
        /// </remarks>
        public TimeSpan SubordinateDelayOffMaster { get; set; }

        /// <summary>
        /// Gets or sets the timestamp offset of the start of the recording.
        /// </summary>
        /// <remarks>
        /// All recorded timestamps are offset by this value such that
        /// the recording starts at timestamp 0. This value can be used to synchronize timestamps between 2 recording files.
        /// </remarks>
        public TimeSpan StartTimestampOffset { get; set; }

        /// <summary>
        /// Gets a <see cref="RecordConfiguration"/> object from a native <see cref="NativeMethods.k4a_record_configuration_t"/> object.
        /// </summary>
        /// <param name="config">Native object.</param>
        /// <returns>Managed object.</returns>
        internal static RecordConfiguration FromNative(NativeMethods.k4a_record_configuration_t config)
        {
            return new RecordConfiguration()
            {
                ColorFormat = config.color_format,
                ColorResolution = config.color_resolution,
                DepthMode = config.depth_mode,
                CameraFPS = config.camera_fps,
                ColorTrackEnabled = config.color_track_enabled == 0 ? false : true,
                DepthTrackEnabled = config.depth_track_enabled == 0 ? false : true,
                IRTrackEnabled = config.ir_track_enabled == 0 ? false : true,
                ImuTrackEnabled = config.imu_track_enabled == 0 ? false : true,
                DepthDelayOffColor = TimeSpan.FromTicks(config.subordinate_delay_off_master_usec * 10),
                WiredSyncMode = config.wired_sync_mode,
                SubordinateDelayOffMaster = TimeSpan.FromTicks(config.subordinate_delay_off_master_usec * 10),
                StartTimestampOffset = TimeSpan.FromTicks(config.start_timestamp_offset_usec),
            };
        }
    }
}
