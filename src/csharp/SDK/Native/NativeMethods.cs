//------------------------------------------------------------------------------
// <copyright file="NativeMethods.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Globalization;
using System.Linq.Expressions;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.Azure.Kinect.Sensor.Native;

namespace Microsoft.Azure.Kinect.Sensor
{
#pragma warning disable IDE1006 // Naming Styles
#pragma warning disable SA1600 // Elements should be documented
#pragma warning disable SA1602 // Enumeration items should be documented
    internal static class NativeMethods
    {
        private const CallingConvention k4aCallingConvention = CallingConvention.Cdecl;

        [UnmanagedFunctionPointer(k4aCallingConvention)]
        public delegate IntPtr k4a_memory_allocate_cb_t(int size, out IntPtr context);

        [UnmanagedFunctionPointer(k4aCallingConvention)]
        public delegate void k4a_memory_destroy_cb_t(IntPtr buffer, IntPtr context);

        [UnmanagedFunctionPointer(k4aCallingConvention)]
        public delegate void k4a_logging_message_cb_t(IntPtr context, LogLevel level, [MarshalAs(UnmanagedType.LPStr)] string file, int line, [MarshalAs(UnmanagedType.LPStr)] string message);

        [NativeReference]
        public enum k4a_buffer_result_t
        {
            K4A_BUFFER_RESULT_SUCCEEDED = 0,
            K4A_BUFFER_RESULT_FAILED,
            K4A_BUFFER_RESULT_TOO_SMALL,
        }

        [NativeReference]
        public enum k4a_wait_result_t
        {
            K4A_WAIT_RESULT_SUCCEEDED = 0,
            K4A_WAIT_RESULT_FAILED,
            K4A_WAIT_RESULT_TIMEOUT,
        }

        [NativeReference]
        public enum k4a_result_t
        {
            K4A_RESULT_SUCCEEDED = 0,
            K4A_RESULT_FAILED,
        }

        [NativeReference]
        public enum k4a_stream_result_t
        {
            K4A_STREAM_RESULT_SUCCEEDED = 0,
            K4A_STREAM_RESULT_FAILED,
            K4A_STREAM_RESULT_EOF,
        }

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_set_allocator(
            k4a_memory_allocate_cb_t allocate,
            k4a_memory_destroy_cb_t free);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_calibration_2d_to_2d(
            [In] ref Calibration calibration,
            ref Vector2 source_point2d,
            float source_depth,
            CalibrationDeviceType source_camera,
            CalibrationDeviceType target_camera,
            out Vector2 target_point2d,
            out bool valid);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_calibration_2d_to_3d(
            [In] ref Calibration calibration,
            ref Vector2 source_point2d,
            float source_depth,
            CalibrationDeviceType source_camera,
            CalibrationDeviceType target_camera,
            out Vector3 target_point3d,
            out bool valid);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_calibration_3d_to_2d(
            [In] ref Calibration calibration,
            ref Vector3 source_point3d,
            CalibrationDeviceType source_camera,
            CalibrationDeviceType target_camera,
            out Vector2 target_point2d,
            out bool valid);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_calibration_3d_to_3d(
            [In] ref Calibration calibration,
            ref Vector3 source_point3d,
            CalibrationDeviceType source_camera,
            CalibrationDeviceType target_camera,
            out Vector3 target_point3d);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_calibration_color_2d_to_depth_2d(
            [In] ref Calibration calibration,
            ref Vector2 source_point2d,
            k4a_image_t depth_image,
            out Vector2 target_point2d,
            out bool valid);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_calibration_get_from_raw(
            byte[] raw_calibration,
            UIntPtr raw_calibration_size,
            DepthMode depth_mode,
            ColorResolution color_resolution,
            out Calibration calibration);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_transformation_t k4a_transformation_create([In] ref Calibration calibration);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_transformation_destroy(IntPtr transformation_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_transformation_depth_image_to_color_camera(
            k4a_transformation_t transformation_handle,
            k4a_image_t depth_image,
            k4a_image_t transformed_depth_image);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_transformation_depth_image_to_color_camera_custom(
            k4a_transformation_t transformation_handle,
            k4a_image_t depth_image,
            k4a_image_t custom_image,
            k4a_image_t transformed_depth_image,
            k4a_image_t transformed_custom_image,
            TransformationInterpolationType interpolation_type,
            uint invalid_custom_value);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_transformation_color_image_to_depth_camera(
            k4a_transformation_t transformation_handle,
            k4a_image_t depth_image,
            k4a_image_t color_image,
            k4a_image_t transformed_color_image);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_transformation_depth_image_to_point_cloud(
                k4a_transformation_t transformation_handle,
                k4a_image_t depth_image,
                CalibrationDeviceType camera,
                k4a_image_t xyz_image);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_device_close(IntPtr device_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_capture_create(out k4a_capture_t capture_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_image_t k4a_capture_get_color_image(k4a_capture_t capture_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_image_t k4a_capture_get_depth_image(k4a_capture_t capture_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_image_t k4a_capture_get_ir_image(k4a_capture_t capture_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern float k4a_capture_get_temperature_c(k4a_capture_t capture_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_capture_set_color_image(k4a_capture_t capture_handle, k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_capture_set_depth_image(k4a_capture_t capture_handle, k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_capture_set_ir_image(k4a_capture_t capture_handle, k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_capture_set_temperature_c(k4a_capture_t capture_handle, float temperature_c);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_capture_reference(IntPtr capture_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_capture_release(IntPtr capture_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_image_create(
            ImageFormat format,
            int width_pixels,
            int height_pixels,
            int stride_bytes,
            out k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_image_create_from_buffer(
            ImageFormat format,
            int width_pixels,
            int height_pixels,
            int stride_bytes,
            IntPtr buffer,
            UIntPtr buffer_size,
            k4a_memory_destroy_cb_t buffer_release_cb,
            IntPtr buffer_release_cb_context,
            out k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_image_reference(IntPtr image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_image_release(IntPtr image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern uint k4a_device_get_installed_count();

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_device_get_calibration(
            k4a_device_t device_handle,
            DepthMode depth_mode,
            ColorResolution color_resolution,
            out Calibration calibration);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_wait_result_t k4a_device_get_capture(
            k4a_device_t device_handle,
            out k4a_capture_t capture_handle,
            int timeout_in_ms);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_wait_result_t k4a_device_get_imu_sample(
            k4a_device_t device_handle,
            [Out] k4a_imu_sample_t imu_sample,
            int timeout_in_ms);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_device_get_sync_jack(
            k4a_device_t device_handle,
            out bool sync_in_jack_connected,
            out bool sync_out_jack_connected);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_device_get_version(
            k4a_device_t device_handle,
            out k4a_hardware_version_t version);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_buffer_result_t k4a_device_get_raw_calibration(k4a_device_t device_handle, [Out] byte[] data, ref UIntPtr data_size);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_device_set_color_control(k4a_device_t device_handle, ColorControlCommand command, ColorControlMode mode, int value);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_device_get_color_control(k4a_device_t device_handle, ColorControlCommand command, out ColorControlMode mode, out int value);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_device_start_cameras(k4a_device_t device_handle, [In] ref k4a_device_configuration_t config);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_device_stop_cameras(k4a_device_t device_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_device_start_imu(k4a_device_t device_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_device_stop_imu(k4a_device_t device_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_device_open(uint index, out k4a_device_t device_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention, CharSet = CharSet.Ansi, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        [NativeReference]
        public static extern k4a_buffer_result_t k4a_device_get_serialnum(k4a_device_t device_handle, StringBuilder serial_number, ref UIntPtr data_size);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern ulong k4a_image_get_exposure_usec(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_image_set_exposure_time_usec(k4a_image_t image_handle, ulong value);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern ImageFormat k4a_image_get_format(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern int k4a_image_get_height_pixels(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern int k4a_image_get_width_pixels(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern int k4a_image_get_stride_bytes(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern UIntPtr k4a_image_get_size(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern uint k4a_image_get_iso_speed(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_image_set_iso_speed(k4a_image_t image_handle, uint value);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern uint k4a_image_get_white_balance(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_image_set_white_balance(k4a_image_t image_handle, uint value);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern ulong k4a_image_get_device_timestamp_usec(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = CallingConvention.Cdecl)]
        [NativeReference]
        public static extern void k4a_image_set_device_timestamp_usec(k4a_image_t image_handle, ulong value);

        [DllImport("k4a", CallingConvention = CallingConvention.Cdecl)]
        [NativeReference]
        public static extern ulong k4a_image_get_system_timestamp_nsec(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern void k4a_image_set_system_timestamp_nsec(k4a_image_t image_handle, ulong value);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern IntPtr k4a_image_get_buffer(k4a_image_t image_handle);

        [DllImport("k4a", CallingConvention = k4aCallingConvention)]
        [NativeReference]
        public static extern k4a_result_t k4a_set_debug_message_handler(
            k4a_logging_message_cb_t message_cb,
            IntPtr message_cb_context,
            LogLevel min_level);

        [NativeReference]
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

        [NativeReference]
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

        [NativeReference]
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
        }

        public class k4a_device_t : Win32.SafeHandles.SafeHandleZeroOrMinusOneIsInvalid
        {
            private k4a_device_t()
                : base(true)
            {
            }

            protected override bool ReleaseHandle()
            {
                NativeMethods.k4a_device_close(this.handle);
                return true;
            }
        }

        public class k4a_capture_t : Win32.SafeHandles.SafeHandleZeroOrMinusOneIsInvalid
        {
            public k4a_capture_t(bool takeOwnership, IntPtr handle)
                : base(true)
            {
                if (!takeOwnership)
                {
                    NativeMethods.k4a_image_reference(handle);
                }

                this.handle = handle;
            }

            private k4a_capture_t()
                : base(true)
            {
            }

            public k4a_capture_t DuplicateReference()
            {
                k4a_capture_t duplicate = new k4a_capture_t();

                k4a_capture_reference(this.handle);

                duplicate.handle = this.handle;
                return duplicate;
            }

            protected override bool ReleaseHandle()
            {
                k4a_capture_release(this.handle);
                return true;
            }
        }

        public class k4a_image_t : Win32.SafeHandles.SafeHandleZeroOrMinusOneIsInvalid
        {
            public k4a_image_t(bool takeOwnership, IntPtr handle)
                : base(true)
            {
                if (!takeOwnership)
                {
                    NativeMethods.k4a_image_reference(handle);
                }

                this.handle = handle;
            }

            private k4a_image_t()
                : base(true)
            {
            }

            private k4a_image_t(k4a_image_t original)
                : base(true)
            {
                k4a_image_reference(original.handle);
                this.handle = original.handle;
            }

            public k4a_image_t DuplicateReference()
            {
                return new k4a_image_t(this);
            }

            protected override bool ReleaseHandle()
            {
                k4a_image_release(this.handle);
                return true;
            }
        }

        public class k4a_transformation_t : Microsoft.Win32.SafeHandles.SafeHandleZeroOrMinusOneIsInvalid
        {
            private k4a_transformation_t()
                : base(true)
            {
            }

            protected override bool ReleaseHandle()
            {
                k4a_transformation_destroy(this.handle);
                return true;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        [Native.NativeReference("k4a_imu_sample_t")]
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
#pragma warning restore SA1602 // Enumeration items should be documented
#pragma warning restore SA1600 // Elements should be documented
#pragma warning restore IDE1006 // Naming Styles
}
