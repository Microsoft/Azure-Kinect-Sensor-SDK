//------------------------------------------------------------------------------
// <copyright file="Recorder.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using Microsoft.Azure.Kinect.Sensor.Record.Exceptions;

namespace Microsoft.Azure.Kinect.Sensor.Record
{
    /// <summary>
    /// Represents a writable sensor recording.
    /// </summary>
    public class Recorder : IDisposable
    {
        // The native handle for this recording.
        private readonly NativeMethods.k4a_record_t handle;

        // To detect redundant calls to Dispose
        private bool disposedValue = false;

        private Recorder(NativeMethods.k4a_record_t handle)
        {
            this.handle = handle;
        }

        /// <summary>
        /// Create a recording.
        /// </summary>
        /// <param name="path">Path to the recording.</param>
        /// <param name="device">Device to get properties from. May be null for user-generated recordings.</param>
        /// <param name="deviceConfiguration">Parameters used to open the device.</param>
        /// <returns>A new recording object.</returns>
        public static Recorder Create(string path, Device device, DeviceConfiguration deviceConfiguration)
        {
            if (path == null)
            {
                throw new ArgumentNullException(nameof(path));
            }

            if (deviceConfiguration == null)
            {
                throw new ArgumentNullException(nameof(deviceConfiguration));
            }

            NativeMethods.k4a_record_t handle = null;
            NativeMethods.k4a_device_configuration_t configuration = NativeMethods.k4a_device_configuration_t.FromDeviceConfiguration(deviceConfiguration);
            if (device != null)
            {
                // If a device was specified, lock that device to avoid disposal while in use.
                // Device.Dispose will take the same lock.
                lock (device)
                {
                    AzureKinectCreateRecordingException.ThrowIfNotSuccess(path, () => NativeMethods.k4a_record_create(path, device.Handle, configuration, out handle));
                }
            }
            else
            {
                AzureKinectCreateRecordingException.ThrowIfNotSuccess(path, () => NativeMethods.k4a_record_create(path, IntPtr.Zero, configuration, out handle));
            }

            return new Recorder(handle);
        }

        /// <summary>
        /// Adds a tag to the recroding.
        /// </summary>
        /// <param name="name">Name of the tag.</param>
        /// <param name="value">Value of the tag.</param>
        public void AddTag(string name, string value)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                AzureKinectAddTagException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_add_tag(this.handle, name, value));
            }
        }

        /// <summary>
        /// Adds an IMU track to the recording.
        /// </summary>
        public void AddImuTrack()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                AzureKinectAddImuTrackException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_add_imu_track(this.handle));
            }
        }

        /// <summary>
        /// Adds an attachment to a recording.
        /// </summary>
        /// <param name="attachmentName">The name of the attachment to be stored in the recording file. This name should be a valid filename with an extension.</param>
        /// <param name="buffer">The attachment data buffer.</param>
        public void AddAttachment(string attachmentName, byte[] buffer)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                AzureKinectAddAttachmentException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_add_attachment(this.handle, attachmentName, buffer, (UIntPtr)buffer.Length));
            }
        }

        /// <summary>
        /// Adds custom video tracks to the recording.
        /// </summary>
        /// <param name="trackName">The name of the custom video track to be added.</param>
        /// <param name="codecId">A UTF8 null terminated string containing the codec ID of the track.
        /// Some of the existing formats are listed here: https://www.matroska.org/technical/specs/codecid/index.html.
        /// The codec ID can also be custom defined by the user. Video codec ID's should start with 'V_'.</param>
        /// <param name="codecContext">The codec context is a codec-specific buffer that contains any required codec metadata that is only known to the codec. It is mapped to the matroska 'CodecPrivate' element.</param>
        /// <param name="trackSettings">Additional metadata for the video track such as resolution and framerate.</param>
        /// <remarks>
        /// Built-in video tracks like the DEPTH, IR, and COLOR tracks will be created automatically when the k4a_record_create()
        /// API is called.This API can be used to add additional video tracks to save custom data.
        ///
        ///  Track names must be ALL CAPS and may only contain A-Z, 0-9, '-' and '_'.
        ///
        ///  All tracks need to be added before the recording header is written.
        ///
        ///  Call k4a_record_write_custom_track_data() with the same track_name to write data to this track.
        ///
        /// </remarks>
        public void AddCustomVideoTrack(
            string trackName,
            string codecId,
            byte[] codecContext,
            RecordVideoSettings trackSettings)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                AzureKinectAddCustomVideoTrackException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_add_custom_video_track(
                    this.handle,
                    trackName,
                    codecId,
                    codecContext,
                    (UIntPtr)codecContext.Length,
                    trackSettings));
            }
        }

        /// <summary>
        /// Adds custom subtitle tracks to the recording.
        /// </summary>
        /// <param name="trackName">The name of the custom subtitle track to be added.</param>
        /// <param name="codecId">A UTF8 null terminated string containing the codec ID of the track.
        /// Some of the existing formats are listed here: https://www.matroska.org/technical/specs/codecid/index.html. The codec ID can also be custom defined by the user.
        /// Subtitle codec ID's should start with 'S_'.</param>
        /// <param name="codecContext">The codec context is a codec-specific buffer that contains any required codec metadata that is only known to the codec.It is mapped to the matroska 'CodecPrivate' element.</param>
        /// <param name="trackSettings">Additional metadata for the subtitle track. If null, the default settings will be used.</param>
        /// <remarks>
        /// Built-in subtitle tracks like the IMU track will be created automatically when the k4a_record_add_imu_track() API is
        /// called.This API can be used to add additional subtitle tracks to save custom data.
        ///
        /// Track names must be ALL CAPS and may only contain A-Z, 0-9, '-' and '_'.
        ///
        /// All tracks need to be added before the recording header is written.
        ///
        /// Call k4a_record_write_custom_track_data() with the same track_name to write data to this track.</remarks>
        public void AddCustomSubtitleTrack(
            string trackName,
            string codecId,
            byte[] codecContext,
            RecordSubtitleSettings trackSettings)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                AzureKinectAddCustomSubtitleTrackException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_add_custom_subtitle_track(
                    this.handle,
                    trackName,
                    codecId,
                    codecContext,
                    (UIntPtr)codecContext.Length,
                    trackSettings));
            }
        }

        /// <summary>
        /// Writes the recording header and metadata to file.
        /// </summary>
        /// <remarks>
        /// This must be called before captures or any track data can be written.
        /// </remarks>
        public void WriteHeader()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                AzureKinectWriteHeaderException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_write_header(this.handle));
            }
        }

        /// <summary>
        /// Writes a capture to the recording file.
        /// </summary>
        /// <param name="capture">Capture containing data to write.</param>
        /// <remarks>
        /// Captures must be written in increasing order of timestamp, and the file's header must already be written.
        ///
        /// k4a_record_write_capture() will write all images in the capture to the corresponding tracks in the recording file.
        /// If any of the images fail to write, other images will still be written before a failure is returned.
        /// </remarks>
        public void WriteCapture(Capture capture)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                if (capture == null)
                {
                    throw new ArgumentNullException(nameof(capture));
                }

                using (Capture reference = capture.Reference())
                {
                    AzureKinectWriteCaptureException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_write_capture(this.handle, reference.Handle));
                }
            }
        }

        /// <summary>
        /// Writes an IMU sample to the recording.
        /// </summary>
        /// <param name="imuSample">Sample with the IMU data.</param>
        public void WriteImuSample(ImuSample imuSample)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                if (imuSample == null)
                {
                    throw new ArgumentNullException(nameof(imuSample));
                }

                NativeMethods.k4a_imu_sample_t sample = new NativeMethods.k4a_imu_sample_t()
                {
                    temperature = imuSample.Temperature,
                    acc_sample = imuSample.AccelerometerSample,
                    acc_timestamp_usec = checked((ulong)imuSample.AccelerometerTimestamp.Ticks / 10),
                    gyro_sample = imuSample.GyroSample,
                    gyro_timestamp_usec = checked((ulong)imuSample.GyroTimestamp.Ticks / 10),
                };

                AzureKinectWriteImuSampleException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_write_imu_sample(this.handle, sample));
            }
        }

        /// <summary>
        /// Writes data for a custom track to file.
        /// </summary>
        /// <param name="trackName">The name of the custom track that the data is going to be written to.</param>
        /// <param name="deviceTimestamp">The timestamp for the custom track data. This timestamp should be in the same time domain as the device timestamp used for recording.</param>
        /// <param name="customData">The buffer of custom track data.</param>
        /// <remarks>
        /// Custom track data must be written in increasing order of timestamp, and the file's header must already be written.
        /// When writing custom track data at the same time as captures or IMU data, the custom data should be within 1 second of
        /// the most recently written timestamp.
        /// </remarks>
        public void WriteCustomTrackData(
            string trackName,
            TimeSpan deviceTimestamp,
            byte[] customData)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                if (trackName == null)
                {
                    throw new ArgumentNullException(nameof(trackName));
                }

                if (customData == null)
                {
                    throw new ArgumentNullException(nameof(customData));
                }

                AzureKinectWriteCustomTrackDataException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_write_custom_track_data(
                    this.handle,
                    trackName,
                    checked((ulong)deviceTimestamp.Ticks / 10),
                    customData,
                    (UIntPtr)customData.Length));
            }
        }

        /// <summary>
        /// Flushes all pending recording data to disk.
        /// </summary>
        /// <remarks>
        /// If an error occurs, best effort is made to flush as much data to disk as possible, but the integrity of the file is not guaranteed.</remarks>
        public void Flush()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Recorder));
                }

                AzureKinectFlushException.ThrowIfNotSuccess(() => NativeMethods.k4a_record_flush(this.handle));
            }
        }

        /// <inheritdoc/>
        public void Dispose()
        {
            this.Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Handle the disposing of the object.
        /// </summary>
        /// <param name="disposing">true when called by Dispose(), false when called by the finalizer.</param>
        protected virtual void Dispose(bool disposing)
        {
            lock (this)
            {
                this.handle.Close();

                this.disposedValue = true;
            }
        }
    }
}
