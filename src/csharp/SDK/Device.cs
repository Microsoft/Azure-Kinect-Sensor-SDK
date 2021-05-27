//------------------------------------------------------------------------------
// <copyright file="Device.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Text;

namespace Microsoft.Azure.Kinect.Sensor
{
    /// <summary>
    /// Represents an Azure Kinect device.
    /// </summary>
    public class Device : IDisposable
    {
        // Cache these values so we don't need to re-marshal them for each
        // access since they are immutable.
        private string serialNum = null;
        private HardwareVersion version = null;

        // The native handle to the device.
        private NativeMethods.k4a_device_t handle;

        // To detect redundant calls to Dispose
        private bool disposedValue = false;

        private Device(NativeMethods.k4a_device_t handle)
        {
            // Hook the native allocator and register this object.
            // .Dispose() will be called on this object when the allocator is shut down.
            Allocator.Singleton.RegisterForDisposal(this);

            this.handle = handle;
        }

        /// <summary>
        /// Gets the devices serial number.
        /// </summary>
        public string SerialNum
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Device));
                    }

                    if (this.serialNum != null)
                    {
                        return this.serialNum;
                    }
                    else
                    {
                        // Determine the required string size
                        UIntPtr size = new UIntPtr(0);
                        if (NativeMethods.k4a_device_get_serialnum(this.handle, null, ref size) != NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_TOO_SMALL)
                        {
                            throw new InvalidOperationException($"Unexpected internal state calling {nameof(NativeMethods.k4a_device_get_serialnum)}");
                        }

                        // Allocate a string buffer
                        StringBuilder serialno = new StringBuilder((int)size.ToUInt32());

                        // Get the serial number
                        AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_get_serialnum(this.handle, serialno, ref size));

                        this.serialNum = serialno.ToString();

                        return this.serialNum;
                    }
                }
            }
        }

        /// <summary>
        /// Gets the depth mode the device is currently set to.
        /// </summary>
        public DepthMode CurrentDepthMode { get; private set; } = DepthMode.Off;

        /// <summary>
        /// Gets the color resolution the device is currently set to.
        /// </summary>
        public ColorResolution CurrentColorResolution { get; private set; } = ColorResolution.Off;

        /// <summary>
        /// Gets a value indicating whether gets the Sync In jack is connected.
        /// </summary>
        public bool SyncInJackConnected
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Device));
                    }

                    bool sync_in = default;
                    bool sync_out = default;
                    AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_get_sync_jack(
                        this.handle,
                        out sync_in,
                        out sync_out));

                    return sync_in;
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether gets the Sync Out jack is connected.
        /// </summary>
        public bool SyncOutJackConnected
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Device));
                    }

                    bool sync_in = default;
                    bool sync_out = default;
                    AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_get_sync_jack(
                        this.handle,
                        out sync_in,
                        out sync_out));

                    return sync_out;
                }
            }
        }

        /// <summary>
        /// Gets the hardware version of the device.
        /// </summary>
        public HardwareVersion Version
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Device));
                    }

                    if (this.version != null)
                    {
                        return this.version;
                    }

                    NativeMethods.k4a_hardware_version_t nativeVersion = default;
                    AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_get_version(
                        this.handle,
                        out nativeVersion));

                    this.version = nativeVersion.ToHardwareVersion();
                    return this.version;
                }
            }
        }

        /// <summary>
        /// Gets the native handle.
        /// </summary>
        /// <remarks>This is the value of the k4a_device_t handle of the native library.
        ///
        /// This handle value can be used to interoperate with other native libraries that use
        /// Azure Kinect objects.
        ///
        /// When using this handle value, the caller is responsible for ensuring that the
        /// Device object does not become disposed.</remarks>
        public IntPtr Handle
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Device));
                    }

                    return this.handle.DangerousGetHandle();
                }
            }
        }

        /// <summary>
        /// Gets the number of currently connected devices.
        /// </summary>
        /// <returns>The number of connected devices.</returns>
        public static int GetInstalledCount()
        {
            return (int)NativeMethods.k4a_device_get_installed_count();
        }

        /// <summary>
        /// Opens an Azure Kinect device.
        /// </summary>
        /// <param name="index">Index of the device to open if there are multiple connected.</param>
        /// <returns>A Device object representing that device.</returns>
        /// <remarks>The device will remain opened for exclusive access until the Device object is disposed.</remarks>
        public static Device Open(int index = 0)
        {
            NativeMethods.k4a_device_t handle = default;
            AzureKinectOpenDeviceException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_open((uint)index, out handle));
            return new Device(handle);
        }

        /// <summary>
        /// Gets the calibration of the device.
        /// </summary>
        /// <param name="depthMode">Depth mode for the calibration.</param>
        /// <param name="colorResolution">Color camera resolution for the calibration.</param>
        /// <returns>Calibration object.</returns>
        public Calibration GetCalibration(DepthMode depthMode, ColorResolution colorResolution)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                Calibration calibration = default;
                AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_get_calibration(this.handle, depthMode, colorResolution, out calibration));
                return calibration;
            }
        }

        /// <summary>
        /// Gets the calibration of the device for the current operating mode.
        /// </summary>
        /// <returns>Calibration object.</returns>
        public Calibration GetCalibration()
        {
            if (this.CurrentColorResolution == ColorResolution.Off && this.CurrentDepthMode == DepthMode.Off)
            {
                throw new AzureKinectException("Cameras not started");
            }

            return this.GetCalibration(this.CurrentDepthMode, this.CurrentColorResolution);
        }

        /// <summary>
        /// Gets the device raw calibration data.
        /// </summary>
        /// <returns>The raw data can be stored off-line for future use.</returns>
        public byte[] GetRawCalibration()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                // Determine the required calibration size
                UIntPtr size = new UIntPtr(0);
                if (NativeMethods.k4a_device_get_raw_calibration(this.handle, null, ref size) != NativeMethods.k4a_buffer_result_t.K4A_BUFFER_RESULT_TOO_SMALL)
                {
                    throw new AzureKinectException($"Unexpected result calling {nameof(NativeMethods.k4a_device_get_raw_calibration)}");
                }

                // Allocate a string buffer
                byte[] raw = new byte[size.ToUInt32()];

                // Get the raw calibration
                AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_get_raw_calibration(this.handle, raw, ref size));

                return raw;
            }
        }

        /// <summary>
        /// Reads a sensor capture.
        /// </summary>
        /// <param name="timeout">Time to wait for a capture.</param>
        /// <returns>A Capture object holding image data.</returns>
        /// <remarks>
        /// Gets the next capture in the streamed sequence of captures from the camera.
        /// If a new capture is not currently available, this function will block until the timeout is reached.
        /// The SDK will buffer at least two captures worth of data before dropping the oldest capture.
        /// Callers needing to capture all data need to ensure they read the data as fast as the data is being produced on average.
        /// </remarks>
        public Capture GetCapture(TimeSpan timeout)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

#pragma warning disable CA1508 // Avoid dead conditional code
                using (LoggingTracer tracer = new LoggingTracer())
#pragma warning restore CA1508 // Avoid dead conditional code
                {
                    NativeMethods.k4a_wait_result_t result = NativeMethods.k4a_device_get_capture(this.handle, out NativeMethods.k4a_capture_t capture, (int)timeout.TotalMilliseconds);

                    if (result == NativeMethods.k4a_wait_result_t.K4A_WAIT_RESULT_TIMEOUT)
                    {
                        throw new TimeoutException("Timed out waiting for capture");
                    }

                    AzureKinectException.ThrowIfNotSuccess(tracer, result);

                    if (capture.IsInvalid)
                    {
                        throw new AzureKinectException("k4a_device_get_capture did not return a valid capture handle");
                    }

                    return new Capture(capture);
                }
            }
        }

        /// <summary>
        /// Reads a sensor capture.
        /// </summary>
        /// <returns>A Capture object holding image data.</returns>
        /// <remarks>
        /// Gets the next capture in the streamed sequence of captures from the camera.
        /// If a new capture is not currently available, this function will block until the timeout is reached.
        /// The SDK will buffer at least two captures worth of data before dropping the oldest capture.
        /// Callers needing to capture all data need to ensure they read the data as fast as the data is being produced on average.
        /// </remarks>
        public Capture GetCapture()
        {
            return this.GetCapture(TimeSpan.FromMilliseconds(-1));
        }

        /// <summary>
        /// Reads an IMU sample from the device.
        /// </summary>
        /// <param name="timeout">Time to wait for an IMU sample.</param>
        /// <returns>The next unread IMU sample from the device.</returns>
        /// <remarks>Gets the next sample in the streamed sequence of IMU samples from the device.
        /// If a new sample is not currently available, this function will block until the timeout is reached.
        /// The API will buffer at least two camera capture intervals worth of samples before dropping the oldest sample. Callers needing to capture all data need to ensure they read the data as fast as the data is being produced on average.
        /// </remarks>
        public ImuSample GetImuSample(TimeSpan timeout)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

#pragma warning disable CA1508 // Avoid dead conditional code
                using (LoggingTracer tracer = new LoggingTracer())
#pragma warning restore CA1508 // Avoid dead conditional code
                {
                    NativeMethods.k4a_imu_sample_t sample = new NativeMethods.k4a_imu_sample_t();
                    NativeMethods.k4a_wait_result_t result = NativeMethods.k4a_device_get_imu_sample(this.handle, sample, (int)timeout.TotalMilliseconds);

                    if (result == NativeMethods.k4a_wait_result_t.K4A_WAIT_RESULT_TIMEOUT)
                    {
                        throw new TimeoutException("Timed out waiting for IMU sample");
                    }

                    AzureKinectException.ThrowIfNotSuccess(tracer, result);

                    return sample.ToImuSample();
                }
            }
        }

        /// <summary>
        /// Reads an IMU sample from the device.
        /// </summary>
        /// <returns>The next unread IMU sample from the device.</returns>
        /// <remarks>Gets the next sample in the streamed sequence of IMU samples from the device.
        /// If a new sample is not currently available, this function will block until one is available.
        /// The API will buffer at least two camera capture intervals worth of samples before dropping the oldest sample. Callers needing to capture all data need to ensure they read the data as fast as the data is being produced on average.
        /// </remarks>
        public ImuSample GetImuSample()
        {
            return this.GetImuSample(TimeSpan.FromMilliseconds(-1));
        }

        /// <summary>
        /// Get the Azure Kinect color sensor control value.
        /// </summary>
        /// <param name="command">Color sensor control command.</param>
        /// <returns>The value of the color control option.</returns>
        public int GetColorControl(ColorControlCommand command)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                return this.GetColorControl(command, out ColorControlMode mode);
            }
        }

        /// <summary>
        /// Get the Azure Kinect color sensor control value.
        /// </summary>
        /// <param name="command">Color sensor control command.</param>
        /// <param name="mode">The mode of the color control option.</param>
        /// <returns>The value of the color control option.</returns>
        public int GetColorControl(ColorControlCommand command, out ColorControlMode mode)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                using (LoggingTracer tracer = new LoggingTracer())
                {
                    AzureKinectException.ThrowIfNotSuccess(tracer, NativeMethods.k4a_device_get_color_control(this.handle, command, out mode, out int value));
                    return value;
                }
            }
        }

        /// <summary>
        /// Sets the Azure Kinect color sensor control value.
        /// </summary>
        /// <param name="command">Color sensor control command.</param>
        /// <param name="mode">The mode of the color control option.</param>
        /// <param name="value">The value of the color control option.</param>
        public void SetColorControl(ColorControlCommand command, ColorControlMode mode, int value)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_set_color_control(this.handle, command, mode, value));
            }
        }

        /// <summary>
        /// Starts color and depth camera capture.
        /// </summary>
        /// <param name="configuration">The configuration we want to run the device in.</param>
        public void StartCameras(DeviceConfiguration configuration)
        {
            if (configuration == null)
            {
                throw new ArgumentNullException(nameof(configuration));
            }

            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                NativeMethods.k4a_device_configuration_t nativeConfig = configuration.GetNativeConfiguration();
                AzureKinectStartCamerasException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_start_cameras(this.handle, ref nativeConfig));

                this.CurrentDepthMode = configuration.DepthMode;
                this.CurrentColorResolution = configuration.ColorResolution;
            }
        }

        /// <summary>
        /// Stops the color and depth camera capture.
        /// </summary>
        public void StopCameras()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                NativeMethods.k4a_device_stop_cameras(this.handle);

                this.CurrentDepthMode = DepthMode.Off;
                this.CurrentColorResolution = ColorResolution.Off;
            }
        }

        /// <summary>
        /// Starts the IMU sample stream.
        /// </summary>
        public void StartImu()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                AzureKinectStartImuException.ThrowIfNotSuccess(() => NativeMethods.k4a_device_start_imu(this.handle));
            }
        }

        /// <summary>
        /// Stops the IMU sample stream.
        /// </summary>
        public void StopImu()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                NativeMethods.k4a_device_stop_imu(this.handle);
            }
        }

        /// <inheritdoc/>
        public void Dispose()
        {
            // Do not change this code. Put cleanup code in Dispose(disposing) below.
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Gets the native handle.
        /// </summary>
        /// <returns>The native handle that is wrapped by this device.</returns>
        /// <remarks>The function is dangerous because there is no guarantee that the
        /// handle will not be disposed once it is retrieved. This should only be called
        /// by code that can ensure that the Capture object will not be disposed on another
        /// thread.</remarks>
        internal NativeMethods.k4a_device_t DangerousGetHandle()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Device));
                }

                return this.handle;
            }
        }

        /// <summary>
        /// Releases unmanaged and - optionally - managed resources.
        /// </summary>
        /// <param name="disposing"><c>True</c> to release both managed and unmanaged resources; <c>False</c> to release only unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposedValue)
            {
                if (disposing)
                {
                    // Callers of DangerousGetHandle will lock this Device object
                    // to ensure the handle isn't disposed while in use.
                    lock (this)
                    {
                        Allocator.Singleton.UnregisterForDisposal(this);

                        this.handle.Close();
                        this.handle = null;

                        this.disposedValue = true;
                    }
                }
            }
        }
    }
}
