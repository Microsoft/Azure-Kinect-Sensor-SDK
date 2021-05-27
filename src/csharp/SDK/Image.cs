//------------------------------------------------------------------------------
// <copyright file="Image.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Buffers;
using System.Diagnostics;
using System.Numerics;
using System.Runtime.InteropServices;

namespace Microsoft.Azure.Kinect.Sensor
{
    /// <summary>
    /// An Azure Kinect Image referencing its buffer and meta-data.
    /// </summary>
    public class Image : IMemoryOwner<byte>, IDisposable
    {
        // The pointer to the underlying native memory
        private IntPtr buffer = IntPtr.Zero;

        // If we have made a managed copy of the native memory, this is the managed array
        private byte[] managedBufferCache = null;

        // If we have wrapped the native memory in a IMemoryOwner<T>, this is the memory owner
        private IMemoryOwner<byte> nativeBufferWrapper = null;

        // The native handle to the image
        private NativeMethods.k4a_image_t handle;

        // Immutable properties of the image
        // These are retrieved from the native code once, and then stored in the managed layer.
        private ImageFormat? format = null;
        private int heightPixels = -1;
        private int widthPixels = -1;
        private int strideBytes = -1;
        private long bufferSize = -1;

        // To detect redundant calls to Dispose
        private bool disposedValue = false;

        /// <summary>
        /// Initializes a new instance of the <see cref="Image"/> class.
        /// </summary>
        /// <param name="format">The pixel format of the image. Must be a format with a constant pixel size.</param>
        /// <param name="widthPixels">Width of the image in pixels.</param>
        /// <param name="heightPixels">Height of the image in pixels.</param>
        /// <param name="strideBytes">Stride of the image in bytes. Must be as large as the width times the size of a pixel. Set to zero for the default if available for that format.</param>
        public Image(ImageFormat format, int widthPixels, int heightPixels, int strideBytes)
        {
            // Hook the native allocator and register this object.
            // .Dispose() will be called on this object when the allocator is shut down.
            Allocator.Singleton.RegisterForDisposal(this);

            AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_image_create(
                format,
                widthPixels,
                heightPixels,
                strideBytes,
                out this.handle));
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Image"/> class.
        /// </summary>
        /// <param name="format">The pixel format of the image. Must be a format with a constant pixel size.</param>
        /// <param name="widthPixels">Width of the image in pixels.</param>
        /// <param name="heightPixels">Height of the image in pixels.</param>
        public Image(ImageFormat format, int widthPixels, int heightPixels)
        {
            // Hook the native allocator and register this object.
            // .Dispose() will be called on this object when the allocator is shut down.
            Allocator.Singleton.RegisterForDisposal(this);

#pragma warning disable CA2000 // Dispose objects before losing scope
            AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_image_create(
                format,
                widthPixels,
                heightPixels,
                0,
                image_handle: out this.handle));
#pragma warning restore CA2000 // Dispose objects before losing scope
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Image"/> class from an existing native handle.
        /// </summary>
        /// <param name="takeOwnership">If false, the constructor will take a new reference on the handle.</param>
        /// <param name="handle">Native handle to a k4a_image_t.</param>
        /// <remarks>
        /// Disposing this Image will always release a reference on the handle.
        /// If <paramref name="takeOwnership"/> is true, the handle may not be owned by any existing Image object.
        /// </remarks>
        public Image(bool takeOwnership, IntPtr handle)
            : this(new NativeMethods.k4a_image_t(takeOwnership, handle))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Image"/> class.
        /// </summary>
        /// <param name="handle">Handle to initialize the image from.</param>
        /// <remarks>The handle will be owned by the new image.</remarks>
        internal Image(NativeMethods.k4a_image_t handle)
        {
            // Hook the native allocator and register this object.
            // .Dispose() will be called on this object when the allocator is shut down.
            Allocator.Singleton.RegisterForDisposal(this);

            this.handle = handle;
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="Image"/> class.
        /// </summary>
        ~Image()
        {
            this.Dispose(false);
        }

        /// <summary>
        /// Gets the Memory containing the image data.
        /// </summary>
        public unsafe Memory<byte> Memory
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    // If we previously copied the native memory to a managed array, return that array's memory
                    if (this.managedBufferCache != null)
                    {
                        return new Memory<byte>(this.managedBufferCache, 0, (int)this.Size);
                    }

                    // If we previously wrapped the native memory in a IMemoryOwner<T>, return that memory object
                    if (this.nativeBufferWrapper != null)
                    {
                        return this.nativeBufferWrapper.Memory;
                    }

                    IntPtr bufferAddress = (IntPtr)this.GetUnsafeBuffer();

                    // If the native buffer is within a memory block that the managed allocator provided,
                    // return that memory.
                    Memory<byte> memory = Allocator.Singleton.GetManagedAllocatedMemory(bufferAddress, this.Size);
                    if (!memory.IsEmpty)
                    {
                        return memory;
                    }

                    // The underlying buffer is not in managed memory.
                    // We can use one of two strategies to return a Memory<T>

                    // If we use the CopyNativeBuffers method, we will allocate and then copy the native memory
                    // to a managed array, ensuring memory-safe access to the contents of memory, at the expense of
                    // a memcpy each time we transition the buffer from native to managed, or from managed to native.

                    // If we don't copy the native buffers, we can construct a MemoryManager<T> that wraps that native
                    // buffer. This has no memcpy cost, but exposes the possibility of use after free bugs to consumers
                    // of the library. This is therefore not enabled by default.
                    if (Allocator.Singleton.SafeCopyNativeBuffers)
                    {
                        // Create a copy
                        int bufferSize = checked((int)this.Size);
                        this.managedBufferCache = Allocator.Singleton.GetBufferCache((IntPtr)this.GetUnsafeBuffer(), bufferSize);

                        return new Memory<byte>(this.managedBufferCache, 0, (int)this.Size);
                    }
                    else
                    {
                        // Provide a Memory<T> object that wraps a native pointer
                        // This is UNSAFE. Callers in safe code may access unowned native
                        // memory if they use the Memory<T> or Span<T> after the Image has
                        // been disposed.
                        this.nativeBufferWrapper = new AzureKinectMemoryManager(this);

                        return this.nativeBufferWrapper.Memory;
                    }
                }
            }
        }

        /// <summary>
        /// Gets or sets the image exposure time.
        /// </summary>
        public TimeSpan Exposure
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    ulong exposure = NativeMethods.k4a_image_get_exposure_usec(this.handle);
                    return TimeSpan.FromTicks(checked((long)exposure) * 10);
                }
            }

            set
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    NativeMethods.k4a_image_set_exposure_time_usec(this.handle, checked((ulong)value.Ticks / 10));
                }
            }
        }

        /// <summary>
        /// Gets the image pixel format.
        /// </summary>
        public ImageFormat Format
        {
            get
            {
                if (this.format.HasValue)
                {
                    return this.format.Value;
                }

                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    this.format = NativeMethods.k4a_image_get_format(this.handle);
                    return this.format.Value;
                }
            }
        }

        /// <summary>
        /// Gets the image height in pixels.
        /// </summary>
        public int HeightPixels
        {
            get
            {
                if (this.heightPixels >= 0)
                {
                    return this.heightPixels;
                }

                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    this.heightPixels = NativeMethods.k4a_image_get_height_pixels(this.handle);
                    return this.heightPixels;
                }
            }
        }

        /// <summary>
        /// Gets the image width in pixels.
        /// </summary>
        public int WidthPixels
        {
            get
            {
                if (this.widthPixels >= 0)
                {
                    return this.widthPixels;
                }

                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    this.widthPixels = NativeMethods.k4a_image_get_width_pixels(this.handle);
                    return this.widthPixels;
                }
            }
        }

        /// <summary>
        /// Gets the image stride in bytes.
        /// </summary>
        public int StrideBytes
        {
            get
            {
                if (this.strideBytes >= 0)
                {
                    return this.strideBytes;
                }

                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    this.strideBytes = NativeMethods.k4a_image_get_stride_bytes(this.handle);

                    return this.strideBytes;
                }
            }
        }

        /// <summary>
        /// Gets the image buffer size in bytes.
        /// </summary>
        public long Size
        {
            get
            {
                if (this.bufferSize >= 0)
                {
                    return this.bufferSize;
                }

                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    this.bufferSize = checked((long)NativeMethods.k4a_image_get_size(this.handle).ToUInt64());

                    return this.bufferSize;
                }
            }
        }

        /// <summary>
        /// Gets or sets the image time-stamp in the device's clock.
        /// </summary>
        public TimeSpan DeviceTimestamp
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    ulong timestamp = NativeMethods.k4a_image_get_device_timestamp_usec(this.handle);
                    return TimeSpan.FromTicks(checked((long)timestamp) * 10);
                }
            }

            set
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    NativeMethods.k4a_image_set_device_timestamp_usec(this.handle, checked((ulong)value.Ticks / 10));
                }
            }
        }

        /// <summary>
        /// Gets or sets the image timestamp in nanoseconds.
        /// </summary>
        /// <remarks>
        /// The base of the timestamp clock is the same as Stopwatch.GetTimestamp(). Units need to be
        /// converted between nanoseconds and the Stopwatch frequency to make comparisons.
        /// </remarks>
        public long SystemTimestampNsec
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    ulong timestamp = NativeMethods.k4a_image_get_system_timestamp_nsec(this.handle);

                    return (long)timestamp;
                }
            }

            set
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    NativeMethods.k4a_image_set_system_timestamp_nsec(this.handle, (ulong)value);
                }
            }
        }

        /// <summary>
        /// Gets or sets the ISO speed.
        /// </summary>
        public int ISOSpeed
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    return checked((int)NativeMethods.k4a_image_get_iso_speed(this.handle));
                }
            }

            set
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    NativeMethods.k4a_image_set_iso_speed(this.handle, checked((uint)value));
                }
            }
        }

        /// <summary>
        /// Gets or sets the white balance.
        /// </summary>
        public int WhiteBalance
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    return checked((int)NativeMethods.k4a_image_get_white_balance(this.handle));
                }
            }

            set
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    NativeMethods.k4a_image_set_white_balance(this.handle, checked((uint)value));
                }
            }
        }

        /// <summary>
        /// Gets the native handle.
        /// </summary>
        /// <remarks>This is the value of the k4a_image_t handle of the native library.
        ///
        /// This handle value can be used to interoperate with other native libraries that use
        /// Azure Kinect objects.
        ///
        /// When using this handle value, the caller is responsible for ensuring that the
        /// Image object does not become disposed.</remarks>
        public IntPtr Handle
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Image));
                    }

                    return this.handle.DangerousGetHandle();
                }
            }
        }

        /// <summary>
        /// Gets the pixels of the image.
        /// </summary>
        /// <typeparam name="TPixel">The type of the pixel.</typeparam>
        /// <remarks>If the image pixels are not in contiguous memory, this method will throw an exception.</remarks>
        /// <returns>The contiguous memory of the image pixels.</returns>
        public Memory<TPixel> GetPixels<TPixel>()
            where TPixel : unmanaged
        {
            if (this.StrideBytes != Marshal.SizeOf(typeof(TPixel)) * this.WidthPixels)
            {
                throw new AzureKinectException("Pixels not aligned to stride of each line");
            }

            return new AzureKinectMemoryCast<byte, TPixel>(this.Memory).Memory;
        }

        /// <summary>
        /// Gets the pixels of the image.
        /// </summary>
        /// <typeparam name="TPixel">The type of the pixel.</typeparam>
        /// <param name="row">The row of pixels to get.</param>
        /// <returns>The contiguous memory of the image pixel row.</returns>
        public Memory<TPixel> GetPixels<TPixel>(int row)
            where TPixel : unmanaged
        {
            if (row > this.HeightPixels || row < 0)
            {
                throw new ArgumentOutOfRangeException(nameof(row));
            }

            int activeLineBytes = Marshal.SizeOf(typeof(TPixel)) * this.WidthPixels;
            if (this.StrideBytes < activeLineBytes)
            {
                throw new AzureKinectException("The image stride is not large enough for pixels of this size");
            }

            return new AzureKinectMemoryCast<byte, TPixel>(this.Memory.Slice(row * this.StrideBytes, activeLineBytes)).Memory;
        }

        /// <summary>
        /// Gets a pixel value in the image.
        /// </summary>
        /// <typeparam name="TPixel">The type of the pixel.</typeparam>
        /// <param name="row">The image row.</param>
        /// <param name="col">The image column.</param>
        /// <returns>The pixel value at the row and column.</returns>
        public unsafe TPixel GetPixel<TPixel>(int row, int col)
            where TPixel : unmanaged
        {
            if (row < 0 || row > this.HeightPixels)
            {
                throw new ArgumentOutOfRangeException(nameof(row));
            }

            if (col < 0 || col > this.WidthPixels)
            {
                throw new ArgumentOutOfRangeException(nameof(col));
            }

            return *(TPixel*)((byte*)this.GetUnsafeBuffer() + (this.StrideBytes * row) + (col * sizeof(TPixel)));
        }

        /// <summary>
        /// Sets a pixel value in the image.
        /// </summary>
        /// <typeparam name="TPixel">The type of the pixel.</typeparam>
        /// <param name="row">The image row.</param>
        /// <param name="col">The image column.</param>
        /// <param name="pixel">The value of the pixel.</param>
        public unsafe void SetPixel<TPixel>(int row, int col, TPixel pixel)
            where TPixel : unmanaged
        {
            if (row < 0 || row > this.HeightPixels)
            {
                throw new ArgumentOutOfRangeException(nameof(row));
            }

            if (col < 0 || col > this.WidthPixels)
            {
                throw new ArgumentOutOfRangeException(nameof(col));
            }

            *(TPixel*)((byte*)this.GetUnsafeBuffer() + (this.StrideBytes * row) + (col * sizeof(TPixel))) = pixel;
        }

        /// <summary>
        /// Creates a duplicate reference to the same Image.
        /// </summary>
        /// <returns>A new Image object representing the same data.</returns>
        /// <remarks>Creating a reference to the same image does not copy the image, but
        /// creates two managed objects representing the same image data. Each object
        /// must be independently disposed. The lifetime of the underlying image data
        /// will be equal or greater than all of the referenced image objects.</remarks>
        public Image Reference()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Image));
                }

#pragma warning disable CA2000 // Dispose objects before losing scope

                // The new image takes ownership of the duplicated handle.
                return new Image(this.handle.DuplicateReference());
#pragma warning restore CA2000 // Dispose objects before losing scope
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
        /// Gets a native pointer to the underlying memory.
        /// </summary>
        /// <remarks>
        /// This property may only be accessed by unsafe code.
        ///
        /// This returns an unsafe pointer to the image's memory. It is important that the
        /// caller ensures the Image is not Disposed or garbage collected while this pointer is
        /// in use, since it may become invalid when the Image is disposed or finalized.
        ///
        /// If this method needs to be used in a context where the caller cannot guarantee that the
        /// Image will be disposed by another thread, the caller can call <see cref="Reference"/>
        /// to create a duplicate reference to the Image which can be disposed separately.
        ///
        /// For safe buffer access <see cref="Memory"/>.
        /// </remarks>
        /// <returns>A pointer to the native buffer.</returns>
        internal unsafe void* GetUnsafeBuffer()
        {
            if (this.buffer != IntPtr.Zero)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Image));
                }

                return (void*)this.buffer;
            }

            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Image));
                }

                this.buffer = NativeMethods.k4a_image_get_buffer(this.handle);
                if (this.buffer == IntPtr.Zero)
                {
                    throw new AzureKinectException("Image has NULL buffer");
                }

                return (void*)this.buffer;
            }
        }

        /// <summary>
        /// Flush the managed cache of native memory to the native buffer.
        /// </summary>
        internal void FlushMemory()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Image));
                }

                if (this.managedBufferCache != null)
                {
                    unsafe
                    {
                        System.Runtime.InteropServices.Marshal.Copy(this.managedBufferCache, 0, (IntPtr)this.GetUnsafeBuffer(), checked((int)this.Size));
                    }
                }
            }
        }

        /// <summary>
        /// Invalidate the managed cache of native memory by reading from the native buffer.
        /// </summary>
        internal void InvalidateMemory()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Image));
                }

                if (this.managedBufferCache != null)
                {
                    unsafe
                    {
                        // We can't wait until a call to Image.Memory to do the copy since user code may already
                        // have a reference to the managed buffer array.
                        System.Runtime.InteropServices.Marshal.Copy((IntPtr)this.GetUnsafeBuffer(), this.managedBufferCache, 0, checked((int)this.Size));
                    }
                }
            }
        }

        /// <summary>
        /// Gets the native handle.
        /// </summary>
        /// <returns>The native handle that is wrapped by this image.</returns>
        /// <remarks>The function is dangerous because there is no guarantee that the
        /// handle will not be disposed once it is retrieved. This should only be called
        /// by code that can ensure that the Image object will not be disposed on another
        /// thread.</remarks>
        internal NativeMethods.k4a_image_t DangerousGetHandle()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Image));
                }

                return this.handle;
            }
        }

        /// <summary>
        /// Checks two Images to determine if they represent the same native image object.
        /// </summary>
        /// <param name="other">Another Image to compare against.</param>
        /// <returns>true if the Images represent the same native k4a_image_t.</returns>
        internal bool NativeEquals(Image other)
        {
            lock (this)
            {
                lock (other)
                {
                    if (this.disposedValue || other.disposedValue)
                    {
                        return false;
                    }

                    IntPtr myHandleValue = this.handle.DangerousGetHandle();
                    IntPtr otherHandleValue = other.handle.DangerousGetHandle();

                    // If both images represent the same native handle, consider them equal
                    return myHandleValue == otherHandleValue;
                }
            }
        }

        /// <summary>
        /// Disposes the resources held by the image.
        /// </summary>
        /// <param name="disposing"><c>True</c> to release both managed and unmanaged resources; <c>False</c> to release only unmanaged resources.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (!this.disposedValue)
            {
                lock (this)
                {
                    if (disposing)
                    {
                        // If we have a native buffer wrapper, dispose it since we
                        // are a wrapper for it's IMemoryOwner<T> interface
                        this.nativeBufferWrapper?.Dispose();

                        Allocator.Singleton.UnregisterForDisposal(this);

                        this.handle.Close();
                        this.handle = null;
                    }

                    // Return the buffer during finalization to ensure that the pool
                    // can clean up its references. If the Image was garbage collected, the pool
                    // will continue to hold a reference.
                    if (this.managedBufferCache != null)
                    {
                        unsafe
                        {
                            Allocator.Singleton.ReturnBufferCache((IntPtr)this.GetUnsafeBuffer());
                        }

                        this.managedBufferCache = null;
                    }

                    this.disposedValue = true;
                }
            }
        }
    }
}
