//------------------------------------------------------------------------------
// <copyright file="DataBlock.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Buffers;
using System.Runtime.InteropServices;

namespace Microsoft.Azure.Kinect.Sensor.Record
{
    /// <summary>
    /// Represents a block of data from a custom recording track.
    /// </summary>
    public class DataBlock : IDisposable, IMemoryOwner<byte>
    {
        // The native handle for this data block.
        private readonly NativeMethods.k4a_playback_data_block_t handle;

        // To detect redundant calls to Dispose
        private bool disposedValue = false;

        private byte[] buffer = null;

        /// <summary>
        /// Initializes a new instance of the <see cref="DataBlock"/> class.
        /// </summary>
        /// <param name="handle">Native handle to the data block.</param>
        internal DataBlock(NativeMethods.k4a_playback_data_block_t handle)
        {
            this.handle = handle;
        }

        /// <summary>
        /// Gets the memory with the custom data.
        /// </summary>
        public Memory<byte> Memory
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(DataBlock));
                    }

                    if (this.buffer == null)
                    {
                        ulong bufferSize = NativeMethods.k4a_playback_data_block_get_buffer_size(this.handle);

                        this.buffer = new byte[bufferSize];

                        IntPtr bufferPtr = NativeMethods.k4a_playback_data_block_get_buffer(this.handle);

                        if (bufferPtr != IntPtr.Zero)
                        {
                            Marshal.Copy(bufferPtr, this.buffer, 0, checked((int)bufferSize));
                        }
                        else
                        {
                            this.buffer = null;
                        }
                    }

                    return this.buffer;
                }
            }
        }

        /// <summary>
        /// Gets the device timestamp associated with the data.
        /// </summary>
        public TimeSpan DeviceTimestamp
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(DataBlock));
                    }

                    ulong timeStamp = NativeMethods.k4a_playback_data_block_get_device_timestamp_usec(this.handle);

                    return TimeSpan.FromTicks(checked((long)timeStamp) * 10);
                }
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
