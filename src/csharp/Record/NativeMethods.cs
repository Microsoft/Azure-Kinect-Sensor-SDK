//------------------------------------------------------------------------------
// <copyright file="NativeMethods.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;

namespace Microsoft.Azure.Kinect.Sensor.Record
{
#pragma warning disable IDE1006 // Naming Styles
#pragma warning disable SA1600 // Elements should be documented
#pragma warning disable SA1602 // Enumeration items should be documented
#pragma warning disable CA2101 // Specify marshaling for P/Invoke string arguments
    internal static class NativeMethods
    {
        private const CallingConvention k4aCallingConvention = CallingConvention.Cdecl;

        [UnmanagedFunctionPointer(k4aCallingConvention)]
        public delegate void k4a_logging_message_cb_t(IntPtr context, LogLevel level, [MarshalAs(UnmanagedType.LPStr)] string file, int line, [MarshalAs(UnmanagedType.LPStr)] string message);

        public enum k4a_buffer_result_t
        {
            K4A_BUFFER_RESULT_SUCCEEDED = 0,
            K4A_BUFFER_RESULT_FAILED,
            K4A_BUFFER_RESULT_TOO_SMALL,
        }

        public enum k4a_wait_result_t
        {
            K4A_WAIT_RESULT_SUCCEEDED = 0,
            K4A_WAIT_RESULT_FAILED,
            K4A_WAIT_RESULT_TIMEOUT,
        }

        public enum k4a_result_t
        {
            K4A_RESULT_SUCCEEDED = 0,
            K4A_RESULT_FAILED,
        }

        public enum k4a_stream_result_t
        {
            K4A_STREAM_RESULT_SUCCEEDED = 0,
            K4A_STREAM_RESULT_FAILED,
            K4A_STREAM_RESULT_EOF,
        }

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_record_set_debug_message_handler(
            k4a_logging_message_cb_t message_cb,
            IntPtr message_cb_context,
            LogLevel min_level);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi)]
        public static extern k4a_result_t k4a_record_create(string path, IntPtr device, k4a_device_configuration_t deviceConfiguration, out k4a_record_t handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi)]
        public static extern k4a_result_t k4a_record_add_tag(k4a_record_t handle, string name, string value);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi)]
        public static extern k4a_result_t k4a_record_add_imu_track(k4a_record_t handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi)]
        public static extern k4a_result_t k4a_record_add_attachment(k4a_record_t handle, string attachment_name, byte[] buffer, UIntPtr buffer_size);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi)]
        public static extern k4a_result_t k4a_record_add_custom_video_track(k4a_record_t handle, string track_name, string codec_id, byte[] codec_context, UIntPtr codec_context_size, RecordVideoSettings track_settings);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi)]
        public static extern k4a_result_t k4a_record_add_custom_subtitle_track(k4a_record_t handle, string track_name, string codec_id, byte[] codec_context, UIntPtr codec_context_size, RecordSubtitleSettings track_settings);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_record_write_header(k4a_record_t handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_record_write_capture(k4a_record_t handle, IntPtr capture);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_record_write_imu_sample(k4a_record_t handle, k4a_imu_sample_t imu_sample);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_record_write_custom_track_data(k4a_record_t handle, string track_name, ulong device_timestamp_usec, byte[] custom_data, UIntPtr custom_data_size);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_record_flush(k4a_record_t handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern void k4a_record_close(IntPtr handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_playback_open(string path, out k4a_playback_t handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_buffer_result_t k4a_playback_get_raw_calibration(k4a_playback_t handle, byte[] data, ref UIntPtr data_size);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_playback_get_calibration(k4a_playback_t playback_handle, out Calibration calibration);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_playback_get_record_configuration(k4a_playback_t playback_handle, ref k4a_record_configuration_t configuration);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern bool k4a_playback_check_track_exists(k4a_playback_t playback_handle, string track_name);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern UIntPtr k4a_playback_get_track_count(k4a_playback_t playback_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_buffer_result_t k4a_playback_get_track_name(k4a_playback_t playback_handle, UIntPtr track_index, StringBuilder track_name, ref UIntPtr track_name_size);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern bool k4a_playback_track_is_builtin(k4a_playback_t playback_handle, string track_name);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_playback_track_get_video_settings(k4a_playback_t playback_handle, string track_name, out RecordVideoSettings video_settings);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_buffer_result_t k4a_playback_track_get_codec_id(k4a_playback_t playback_handle, string track_name, StringBuilder codec_id, ref UIntPtr codec_id_size);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_buffer_result_t k4a_playback_track_get_codec_context(
            k4a_playback_t playback_handle,
            string track_name,
            byte[] codec_context,
            ref UIntPtr codec_context_size);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi)]
        public static extern k4a_buffer_result_t k4a_playback_get_tag(
            k4a_playback_t playback_handle,
            string track_name,
            StringBuilder value,
            ref UIntPtr codec_context_size);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_playback_set_color_conversion(
            k4a_playback_t playback_handle,
            ImageFormat target_format);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi)]
        public static extern k4a_buffer_result_t k4a_playback_get_attachment(
            k4a_playback_t playback_handle,
            string file_name,
            byte[] data,
            ref UIntPtr data_size);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_stream_result_t k4a_playback_get_next_capture(
            k4a_playback_t playback_handle,
            out IntPtr capture_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_stream_result_t k4a_playback_get_previous_capture(
            k4a_playback_t playback_handle,
            out IntPtr capture_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_stream_result_t k4a_playback_get_next_imu_sample(
            k4a_playback_t playback_handle,
            [Out] k4a_imu_sample_t imu_sample);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_stream_result_t k4a_playback_get_previous_imu_sample(
            k4a_playback_t playback_handle,
            [Out] k4a_imu_sample_t imu_sample);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_stream_result_t k4a_playback_get_next_data_block(
            k4a_playback_t playback_handle,
            string track_name,
            out k4a_playback_data_block_t data_block);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_stream_result_t k4a_playback_get_previous_data_block(
            k4a_playback_t playback_handle,
            string track_name,
            out k4a_playback_data_block_t data_block_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern IntPtr k4a_playback_data_block_get_buffer(k4a_playback_data_block_t data_block_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern ulong k4a_playback_data_block_get_device_timestamp_usec(k4a_playback_data_block_t data_block_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern ulong k4a_playback_data_block_get_buffer_size(k4a_playback_data_block_t data_block_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern void k4a_playback_data_block_release(IntPtr data_block_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern k4a_result_t k4a_playback_seek_timestamp(k4a_playback_t playback_handle, ulong offset_usec, PlaybackSeekOrigin origin);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern ulong k4a_playback_get_recording_length_usec(k4a_playback_t playback_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern ulong k4a_playback_get_last_timestamp_usec(k4a_playback_t playback_handle);

        [DllImport("k4arecord", CallingConvention = k4aCallingConvention)]
        public static extern void k4a_playback_close(IntPtr playback_handle);

        [StructLayout(LayoutKind.Sequential)]
        public struct k4a_version_t
        {
            public int major;
            public int minor;
            public int revision;

            public Version ToVersion()
            {
                return new Version(this.major, this.minor, this.revision);
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct k4a_hardware_version_t
        {
            public k4a_version_t rgb;
            public k4a_version_t depth;
            public k4a_version_t audio;
            public k4a_version_t depth_sensor;
            public FirmwareBuild firmware_build;
            public FirmwareSignature firmware_signature;

            public HardwareVersion ToHardwareVersion()
            {
                return new HardwareVersion
                {
                    RGB = this.rgb.ToVersion(),
                    Depth = this.depth.ToVersion(),
                    Audio = this.audio.ToVersion(),
                    DepthSensor = this.depth_sensor.ToVersion(),
                    FirmwareBuild = this.firmware_build,
                    FirmwareSignature = this.firmware_signature,
                };
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct k4a_device_configuration_t
        {
            public ImageFormat color_format;
            public ColorResolution color_resolution;
            public DepthMode depth_mode;
            public FPS camera_fps;
            public bool synchronized_images_only;
            public int depth_delay_off_color_usec;
            public WiredSyncMode wired_sync_mode;
            public uint subordinate_delay_off_master_usec;
            public bool disable_streaming_indicator;

            public static k4a_device_configuration_t FromDeviceConfiguration(DeviceConfiguration configuration)
            {
                // Ticks are in 100ns units
                int depth_delay_off_color_usec = checked((int)(
                    configuration.DepthDelayOffColor.Ticks / 10));

                uint subordinate_delay_off_master_usec = checked((uint)(
                    configuration.SuboridinateDelayOffMaster.Ticks / 10));

                return new NativeMethods.k4a_device_configuration_t
                {
                    color_format = configuration.ColorFormat,
                    color_resolution = configuration.ColorResolution,
                    depth_mode = configuration.DepthMode,
                    camera_fps = configuration.CameraFPS,
                    synchronized_images_only = configuration.SynchronizedImagesOnly,
                    depth_delay_off_color_usec = depth_delay_off_color_usec,
                    wired_sync_mode = configuration.WiredSyncMode,
                    subordinate_delay_off_master_usec = subordinate_delay_off_master_usec,
                    disable_streaming_indicator = configuration.DisableStreamingIndicator,
                };
            }
        }

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct k4a_record_configuration_t
        {
            public ImageFormat color_format;
            public ColorResolution color_resolution;
            public DepthMode depth_mode;
            public FPS camera_fps;
            public byte color_track_enabled;
            public byte depth_track_enabled;
            public byte ir_track_enabled;
            public byte imu_track_enabled;
            public int depth_delay_off_color_usec;
            public WiredSyncMode wired_sync_mode;
            public uint subordinate_delay_off_master_usec;
            public uint start_timestamp_offset_usec;
        }

        public class k4a_record_t : Win32.SafeHandles.SafeHandleZeroOrMinusOneIsInvalid
        {
            private k4a_record_t()
                : base(true)
            {
            }

            protected override bool ReleaseHandle()
            {
                NativeMethods.k4a_record_close(this.handle);
                return true;
            }
        }

        public class k4a_playback_t : Win32.SafeHandles.SafeHandleZeroOrMinusOneIsInvalid
        {
            private k4a_playback_t()
                : base(true)
            {
            }

            protected override bool ReleaseHandle()
            {
                NativeMethods.k4a_playback_close(this.handle);
                return true;
            }
        }

        public class k4a_playback_data_block_t : Win32.SafeHandles.SafeHandleZeroOrMinusOneIsInvalid
        {
            private k4a_playback_data_block_t()
                : base(true)
            {
            }

            protected override bool ReleaseHandle()
            {
                NativeMethods.k4a_playback_data_block_release(this.handle);
                return true;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        public class k4a_imu_sample_t
        {
            public float temperature { get; set; }

            public Vector3 acc_sample { get; set; }

            public ulong acc_timestamp_usec { get; set; }

            public Vector3 gyro_sample { get; set; }

            public ulong gyro_timestamp_usec { get; set; }

            public ImuSample ToImuSample()
            {
                return new ImuSample
                {
                    Temperature = this.temperature,
                    AccelerometerSample = this.acc_sample,
                    AccelerometerTimestamp = TimeSpan.FromTicks(checked((long)this.acc_timestamp_usec) * 10),
                    GyroSample = this.gyro_sample,
                    GyroTimestamp = TimeSpan.FromTicks(checked((long)this.gyro_timestamp_usec) * 10),
                };
            }
        }
    }
#pragma warning restore CA2101 // Specify marshaling for P/Invoke string arguments
#pragma warning restore SA1602 // Enumeration items should be documented
#pragma warning restore SA1600 // Elements should be documented
#pragma warning restore IDE1006 // Naming Styles
}
