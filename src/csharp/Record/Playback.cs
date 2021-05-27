//------------------------------------------------------------------------------
// <copyright file="Playback.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Text;
using Microsoft.Azure.Kinect.Sensor.Record.Exceptions;

namespace Microsoft.Azure.Kinect.Sensor.Record
{
    /// <summary>
    /// Respresents a file being used to playback data from an Azure Kinect device.
    /// </summary>
    public class Playback : IDisposable
    {
        // The native handle for this recording.
        private readonly NativeMethods.k4a_playback_t handle;

        // To detect redundant calls to Dispose
        private bool disposedValue = false;

        // Cached values
        private Calibration? calibration = null;
        private RecordConfiguration recordConfiguration = null;

        private Playback(NativeMethods.k4a_playback_t handle)
        {
            this.handle = handle;
        }

        /// <summary>
        /// Gets get the camera calibration for Azure Kinect device used during recording. The output struct is used as input to all transformation functions.
        /// </summary>
        /// <remarks>
        /// The calibration may not exist if the device was not specified during recording.
        /// </remarks>
        public Calibration? Calibration
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Playback));
                    }

                    if (!this.calibration.HasValue)
                    {
                        if (NativeMethods.k4a_playback_get_calibration(this.handle, out Calibration localCalibration) == NativeMethods.k4a_result_t.K4A_RESULT_SUCCEEDED)
                        {
                            this.calibration = localCalibration;
                        }
                    }

                    return this.calibration;
                }
            }
        }

        /// <summary>
        /// Gets get the device configuration used during recording.
        /// </summary>
        public RecordConfiguration RecordConfiguration
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Playback));
                    }

                    if (this.recordConfiguration == null)
                    {
                        NativeMethods.k4a_record_configuration_t nativeConfig = new NativeMethods.k4a_record_configuration_t();

                        if (NativeMethods.k4a_playback_get_record_configuration(this.handle, ref nativeConfig) == NativeMethods.k4a_result_t.K4A_RESULT_SUCCEEDED)
                        {
                            this.recordConfiguration = RecordConfiguration.FromNative(nativeConfig);
                        }
                    }

                    return this.recordConfiguration;
                }
            }
        }

        /// <summary>
        /// Gets get the number of tracks in a playback file.
        /// </summary>
        public int TrackCount
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Playback));
                    }

                    return checked((int)NativeMethods.k4a_playback_get_track_count(this.handle));
                }
            }
        }

        /// <summary>
        /// Gets the length of the recording in microseconds.
        /// </summary>
        /// <remarks>
        /// The recording length, calculated as the difference between the first and last timestamp in the file.
        ///
        /// The recording length may be longer than an individual track if, for example, the IMU continues to run after the last
        /// color image is recorded.
        /// </remarks>
        public TimeSpan RecordingLength
        {
            get
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                long length = checked((long)NativeMethods.k4a_playback_get_recording_length_usec(this.handle));
                return TimeSpan.FromTicks(length * 10);
            }
        }

        /// <summary>
        /// Opens an existing recording file for reading.
        /// </summary>
        /// <param name="path">Filesystem path of the existing recording.</param>
        /// <returns>An object representing the file for playback.</returns>
        public static Playback Open(string path)
        {
            NativeMethods.k4a_playback_t handle = null;

            AzureKinectOpenPlaybackException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_open(path, out handle));

            return new Playback(handle);
        }

        /// <summary>
        /// Get the raw calibration blob for the Azure Kinect device used during recording.
        /// </summary>
        /// <returns>The raw calibration may not exist if the device was not specified during recording.</returns>
        public byte[] GetRawCalibration()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                // Determine the required calibration size
                UIntPtr size = new UIntPtr(0);
                if (NativeMethods.k4a_playback_get_raw_calibration(this.handle, null, ref size) != NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_TOO_SMALL)
                {
                    throw new AzureKinectGetRawCalibrationException($"Unexpected result calling {nameof(NativeMethods.k4a_playback_get_raw_calibration)}");
                }

                // Allocate a string buffer
                byte[] raw = new byte[size.ToUInt32()];

                // Get the raw calibration
                AzureKinectGetRawCalibrationException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_get_raw_calibration(this.handle, raw, ref size));

                return raw;
            }
        }

        /// <summary>
        /// Checks whether a track with the given track name exists in the playback file.
        /// </summary>
        /// <param name="trackName">The track name to be checked to see whether it exists or not.</param>
        /// <returns>True if the track exists in the file.</returns>
        public bool CheckTrackExists(string trackName)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (trackName == null)
                {
                    throw new ArgumentNullException(nameof(trackName));
                }

                return NativeMethods.k4a_playback_check_track_exists(this.handle, trackName);
            }
        }

        /// <summary>
        /// Gets the name of a track at a specific index.
        /// </summary>
        /// <param name="index">The index of the track to read the name form.</param>
        /// <returns>The name of the track.</returns>
        /// <remarks>When used along with <see cref="TrackCount"/>, this function can be used to enumerate all the available tracks
        /// in a playback file. Additionally <see cref="GetTrackIsBuiltin(string)"/> can be used to filter custom tracks.</remarks>
        public string GetTrackName(int index)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (index >= this.TrackCount || index < 0)
                {
                    throw new ArgumentOutOfRangeException(nameof(index));
                }

                // Determine the required string size
                UIntPtr size = new UIntPtr(0);
                if (NativeMethods.k4a_playback_get_track_name(this.handle, (UIntPtr)index, null, ref size) != NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_TOO_SMALL)
                {
                    throw new AzureKinectException($"Unexpected internal state calling {nameof(NativeMethods.k4a_playback_get_track_name)}");
                }

                // Allocate a string buffer
                StringBuilder trackname = new StringBuilder((int)size.ToUInt32());

                // Get the track name
                AzureKinectGetTrackNameException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_get_track_name(this.handle, (UIntPtr)index, trackname, ref size));

                return trackname.ToString();
            }
        }

        /// <summary>
        /// Checks whether a track is one of the built-in tracks: "COLOR", "DEPTH", etc...
        /// </summary>
        /// <param name="trackName">The track name to be checked to see whether it is a built-in track.</param>
        /// <returns>true if the track is built-in. If the provided track name does not exist, false will be returned.</returns>
        public bool GetTrackIsBuiltin(string trackName)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (trackName == null)
                {
                    throw new ArgumentNullException(nameof(trackName));
                }

                return NativeMethods.k4a_playback_track_is_builtin(this.handle, trackName);
            }
        }

        /// <summary>
        /// Gets the video-specific track information for a particular video track.
        /// </summary>
        /// <param name="trackName">The track name to read video settings from.</param>
        /// <returns>The track's video settings.</returns>
        public RecordVideoSettings GetTrackVideoSettings(string trackName)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (trackName == null)
                {
                    throw new ArgumentNullException(nameof(trackName));
                }

                RecordVideoSettings recordVideoSettings = new RecordVideoSettings();

                AzureKinectTrackGetVideoSettingsException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_track_get_video_settings(this.handle, trackName, out recordVideoSettings));

                return recordVideoSettings;
            }
        }

        /// <summary>
        /// Gets the codec id string for a particular track.
        /// </summary>
        /// <param name="trackName">The track name to read the codec id from.</param>
        /// <returns>Codec ID for the track.</returns>
        /// <remarks>
        /// The codec ID is a string that corresponds to the codec of the track's data. Some of the existing formats are listed
        /// here: https://www.matroska.org/technical/specs/codecid/index.html. It can also be custom defined by the user.
        /// </remarks>
        public string GetTrackCodecId(string trackName)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (trackName == null)
                {
                    throw new ArgumentNullException(nameof(trackName));
                }

                // Determine the required string size
                UIntPtr size = new UIntPtr(0);
                if (NativeMethods.k4a_playback_track_get_codec_id(this.handle, trackName, null, ref size) != NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_TOO_SMALL)
                {
                    throw new AzureKinectException($"Unexpected internal state calling {nameof(NativeMethods.k4a_playback_get_track_name)}");
                }

                // Allocate a string buffer
                StringBuilder codec = new StringBuilder((int)size.ToUInt32());

                // Get the codec id
                AzureKinectGetTrackNameException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_track_get_codec_id(this.handle, trackName, codec, ref size));

                return codec.ToString();
            }
        }

        /// <summary>
        /// Gets the codec context for a particular track.
        /// </summary>
        /// <param name="trackName">The track name to read the codec context from.</param>
        /// <returns>The codec context data.</returns>
        /// <remarks>
        /// The codec context is a codec-specific buffer that contains any required codec metadata that is only known to the
        /// codec. It is mapped to the matroska Codec Private field.
        /// </remarks>
        public byte[] GetTrackCodecContext(string trackName)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (trackName == null)
                {
                    throw new ArgumentNullException(nameof(trackName));
                }

                // Determine the required buffer size
                UIntPtr size = new UIntPtr(0);
                if (NativeMethods.k4a_playback_track_get_codec_context(this.handle, trackName, null, ref size) != NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_TOO_SMALL)
                {
                    throw new AzureKinectException($"Unexpected internal state calling {nameof(NativeMethods.k4a_playback_get_track_name)}");
                }

                // Allocate a buffer
                byte[] context = new byte[checked((int)size)];

                // Get the codec id
                AzureKinectGetTrackNameException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_track_get_codec_context(this.handle, trackName, context, ref size));

                return context;
            }
        }

        /// <summary>
        /// Read the value of a tag from a recording.
        /// </summary>
        /// <param name="name">The name of the tag to read.</param>
        /// <returns>The value of the tag.</returns>
        public string GetTag(string name)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (name == null)
                {
                    throw new ArgumentNullException(nameof(name));
                }

                // Determine the required string size
                UIntPtr size = new UIntPtr(0);
                NativeMethods.k4a_buffer_result_t result;

#pragma warning disable CA1508 // Avoid dead conditional code
                using (LoggingTracer tracer = new LoggingTracer(LogLevel.Warning, Logger.LogProvider, RecordLogger.LogProvider))
#pragma warning restore CA1508 // Avoid dead conditional code
                {
                    result = NativeMethods.k4a_playback_get_tag(this.handle, name, null, ref size);

                    if (result == NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_FAILED)
                    {
                        AzureKinectGetTagException.ThrowIfNotSuccess(tracer, result);
                    }
                }

                if (result != NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_TOO_SMALL)
                {
                    throw new AzureKinectException($"Unexpected internal state calling {nameof(NativeMethods.k4a_playback_get_track_name)}");
                }

                // Allocate a string buffer
                StringBuilder value = new StringBuilder((int)size.ToUInt32());

                // Get the codec id
                AzureKinectGetTagException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_get_tag(this.handle, name, value, ref size));

                return value.ToString();
            }
        }

        /// <summary>
        /// Set the image format that color captures will be converted to. By default the conversion format will be the same as
        /// the image format stored in the recording file, and no conversion will occur.
        /// </summary>
        /// <param name="targetFormat">The target image format to be returned in captures.</param>
        /// <remarks>
        /// Color format conversion occurs in the user-thread, so setting <paramref name="targetFormat"/> to anything other than the format
        /// stored in the file may significantly increase the latency of <see cref="GetNextCapture"/> and <see cref="GetPreviousCapture"/>.
        /// </remarks>
        public void SetColorConversion(ImageFormat targetFormat)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                AzureKinectSetColorConversionException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_set_color_conversion(this.handle, targetFormat));
            }
        }

        /// <summary>
        /// Reads an attachment file from a recording.
        /// </summary>
        /// <param name="fileName">The attachment file name.</param>
        /// <returns>The attachment data.</returns>
        public byte[] GetAttachment(string fileName)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (fileName == null)
                {
                    throw new ArgumentNullException(nameof(fileName));
                }

                // Determine the required buffer size
                UIntPtr size = new UIntPtr(0);
                if (NativeMethods.k4a_playback_get_attachment(this.handle, fileName, null, ref size) != NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_TOO_SMALL)
                {
                    throw new AzureKinectException($"Unexpected internal state calling {nameof(NativeMethods.k4a_playback_get_track_name)}");
                }

                // Allocate a buffer
                byte[] buffer = new byte[checked((int)size)];

                // Get the codec id
                AzureKinectGetTrackNameException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_get_attachment(this.handle, fileName, buffer, ref size));

                return buffer;
            }
        }

        /// <summary>
        /// Read the next capture in the recording sequence.
        /// </summary>
        /// <returns>The next capture in the sequence, or null if at the end of the sequence.</returns>
        /// <remarks>
        /// <see cref="GetNextCapture"/> always returns the next capture in sequence after the most recently returned capture.
        ///
        /// The first call to <see cref="GetNextCapture"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return the capture
        /// in the recording closest to the seek time with an image timestamp greater than or equal to the seek time.
        ///
        /// If a call was made to <see cref="GetPreviousCapture"/> that returned null, the playback
        /// position is at the beginning of the stream and <see cref="GetNextCapture"/> will return the first capture in the
        /// recording.
        ///
        /// Capture objects returned by the playback API will always contain at least one image, but may have images missing if
        /// frames were dropped in the original recording. When calling <see cref="Capture.Color"/>,
        /// <see cref="Capture.Depth"/>, or <see cref="Capture.IR"/>, the image should be checked for null.
        /// </remarks>
        public Capture GetNextCapture()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                IntPtr captureHandle = IntPtr.Zero;

                switch (NativeMethods.k4a_playback_get_next_capture(this.handle, out captureHandle))
                {
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_EOF:
                        return null;
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_FAILED:
                        throw new AzureKinectGetCaptureException();
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_SUCCEEDED:
                        return new Capture(true, captureHandle);
                }

                return null;
            }
        }

        /// <summary>
        /// Read the previous capture in the recording sequence.
        /// </summary>
        /// <returns>The previous capture in the sequence, or null if at the beginning of the sequence.</returns>
        /// <remarks>
        /// <see cref="GetPreviousCapture"/> always returns the previous capture in sequence after the most recently returned capture.
        ///
        /// The first call to <see cref="GetPreviousCapture"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return the capture
        /// in the recording closest to the seek time with all image timestamps less than the seek time.
        ///
        /// If a call was made to <see cref="GetNextCapture"/> that returned null, the playback
        /// position is at the end of the stream and <see cref="GetPreviousCapture"/> will return the last capture in the
        /// recording.
        ///
        /// Capture objects returned by the playback API will always contain at least one image, but may have images missing if
        /// frames were dropped in the original recording. When calling <see cref="Capture.Color"/>,
        /// <see cref="Capture.Depth"/>, or <see cref="Capture.IR"/>, the image should be checked for null.
        /// </remarks>
        public Capture GetPreviousCapture()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                IntPtr captureHandle = IntPtr.Zero;

                switch (NativeMethods.k4a_playback_get_previous_capture(this.handle, out captureHandle))
                {
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_EOF:
                        return null;
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_FAILED:
                        throw new AzureKinectGetCaptureException();
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_SUCCEEDED:
                        return new Capture(true, captureHandle);
                }

                return null;
            }
        }

        /// <summary>
        /// Read the next IMU sample in the recording sequence.
        /// </summary>
        /// <returns>The next IMU sample in the sequence, or null if at the end of the sequence.</returns>
        /// <remarks>
        /// <see cref="GetNextImuSample"/> always returns the next IMU sample in sequence after the most recently returned sample.
        ///
        /// The first call to <see cref="GetNextImuSample"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return the sample
        /// in the recording closest to the seek time with a timestamp greater than or equal to the seek time.
        ///
        /// If a call was made to <see cref="GetPreviousImuSample"/> that returned null, the playback
        /// position is at the beginning of the stream and <see cref="GetNextImuSample"/> will return the first sample in the
        /// recording.
        /// </remarks>
        public ImuSample GetNextImuSample()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                NativeMethods.k4a_imu_sample_t imu_sample = new NativeMethods.k4a_imu_sample_t();

                switch (NativeMethods.k4a_playback_get_next_imu_sample(this.handle, imu_sample))
                {
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_EOF:
                        return null;
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_FAILED:
                        throw new AzureKinectGetImuSampleException();
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_SUCCEEDED:
                        return imu_sample.ToImuSample();
                }

                return null;
            }
        }

        /// <summary>
        /// Read the previous IMU sample in the recording sequence.
        /// </summary>
        /// <returns>The previous IMU sample in the sequence, or null if at the beginning of the sequence.</returns>
        /// <remarks>
        /// <see cref="GetPreviousImuSample"/> always returns the previous IMU sample in sequence before the most recently returned sample.
        ///
        /// The first call to <see cref="GetPreviousImuSample"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return the sample
        /// in the recording closest to the seek time with a timestamp less than the seek time.
        ///
        /// If a call was made to <see cref="GetNextImuSample"/> that returned null, the playback
        /// position is at the end of the stream and <see cref="GetPreviousImuSample"/> will return the last sample in the
        /// recording.
        /// </remarks>
        public ImuSample GetPreviousImuSample()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                NativeMethods.k4a_imu_sample_t imu_sample = new NativeMethods.k4a_imu_sample_t();

                switch (NativeMethods.k4a_playback_get_previous_imu_sample(this.handle, imu_sample))
                {
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_EOF:
                        return null;
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_FAILED:
                        throw new AzureKinectGetImuSampleException();
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_SUCCEEDED:
                        return imu_sample.ToImuSample();
                }

                return null;
            }
        }

        /// <summary>
        /// Read the next data block for a particular track.
        /// </summary>
        /// <param name="trackName">The name of the track to read the next data block from.</param>
        /// <returns>The next data block in the sequence, or null if at the end of the sequence.</returns>
        /// <remarks>
        /// <see cref="GetNextDataBlock(string)"/> always returns the next data block in sequence after the most recently returned data block
        /// for a particular track.
        ///
        /// The first call to <see cref="GetNextDataBlock(string)"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return the data block
        /// in the recording closest to the seek time with a timestamp greater than or equal to the seek time.
        ///
        /// If a call was made to <see cref="GetPreviousDataBlock(string)"/> that returned null for a particular track, the playback
        /// position is at the beginning of the stream and <see cref="GetNextDataBlock(string)"/> will return the first data block in the
        /// recording.
        /// </remarks>
        public DataBlock GetNextDataBlock(string trackName)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (trackName == null)
                {
                    throw new ArgumentNullException(nameof(trackName));
                }

                switch (NativeMethods.k4a_playback_get_next_data_block(this.handle, trackName, out NativeMethods.k4a_playback_data_block_t dataBlock))
                {
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_EOF:
                        return null;
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_FAILED:
                        throw new AzureKinectGetDataBlockException();
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_SUCCEEDED:
                        return new DataBlock(dataBlock);
                }

                return null;
            }
        }

        /// <summary>
        /// Read the previous data block for a particular track.
        /// </summary>
        /// <param name="trackName">The name of the track to read the previous data block from.</param>
        /// <returns>The previous data block in the sequence, or null if at the beginning of the sequence.</returns>
        /// <remarks>
        /// <see cref="GetPreviousDataBlock(string)"/> always returns the previous data block in sequence after the most recently returned data block
        /// for a particular track.
        ///
        /// The first call to <see cref="GetPreviousDataBlock(string)"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return the data block
        /// in the recording closest to the seek time with a timestamp less than the seek time.
        ///
        /// If a call was made to <see cref="GetNextDataBlock(string)"/> that returned null for a particular track, the playback
        /// position is at the end of the stream and <see cref="GetPreviousDataBlock(string)"/> will return the last data block in the
        /// recording.
        /// </remarks>
        public DataBlock GetPreviousDataBlock(string trackName)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                if (trackName == null)
                {
                    throw new ArgumentNullException(nameof(trackName));
                }

                switch (NativeMethods.k4a_playback_get_previous_data_block(this.handle, trackName, out NativeMethods.k4a_playback_data_block_t dataBlock))
                {
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_EOF:
                        return null;
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_FAILED:
                        throw new AzureKinectGetDataBlockException();
                    case NativeMethods.k4a_stream_result_t.K4A_STREAM_RESULT_SUCCEEDED:
                        return new DataBlock(dataBlock);
                }

                return null;
            }
        }

        /// <summary>
        /// Seek to a specific timestamp within a recording.
        /// </summary>
        /// <param name="offset">The timestamp offset to seek to, relative to <paramref name="origin"/>.</param>
        /// <param name="origin">Specifies how the given timestamp should be interpreted. Seek can be done relative to the beginning or end of the
        /// recording, or using an absolute device timestamp.</param>
        /// <remarks>
        /// The first device timestamp in a recording is usually non-zero. The recording file starts at the device timestamp
        /// defined by <see cref="RecordConfiguration.StartTimestampOffset"/>, which is accessible via <see cref="Playback.RecordConfiguration"/>.
        ///
        /// The first call to <see cref="GetNextCapture"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return a capture containing an image
        /// timestamp greater than or equal to the seek time.
        ///
        /// The first call to <see cref="GetPreviousCapture"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return a capture with
        /// all image timstamps less than the seek time.
        ///
        /// The first call to <see cref="GetNextImuSample"/> and <see cref="GetNextDataBlock(string)"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return the
        /// first data with a timestamp greater than or equal to the seek time.
        ///
        /// The first call to <see cref="GetPreviousImuSample"/> and <see cref="GetPreviousDataBlock(string)"/> after <see cref="Seek(TimeSpan, PlaybackSeekOrigin)"/> will return the
        /// first data with a timestamp less than the seek time.
        /// </remarks>
        public void Seek(TimeSpan offset, PlaybackSeekOrigin origin = PlaybackSeekOrigin.Begin)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Playback));
                }

                AzureKinectSeekException.ThrowIfNotSuccess(() => NativeMethods.k4a_playback_seek_timestamp(this.handle, checked((ulong)(offset.Ticks / 10)), origin));
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
