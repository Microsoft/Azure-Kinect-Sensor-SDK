//------------------------------------------------------------------------------
// <copyright file="Capture.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Diagnostics.CodeAnalysis;

namespace Microsoft.Azure.Kinect.Sensor
{
    /// <summary>
    /// A set of images captured in sync.
    /// </summary>
    public class Capture : IDisposable
    {
        // Image caches
        // These are the managed wrappers for the native images.
        // They are owned by this class and must be disposed when the capture is disposed.
        private Image cachedColor;
        private Image cachedDepth;
        private Image cachedIR;

        private NativeMethods.k4a_capture_t handle;

        private bool disposedValue = false; // To detect redundant calls

        /// <summary>
        /// Initializes a new instance of the <see cref="Capture"/> class.
        /// </summary>
        public Capture()
        {
            AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_capture_create(out this.handle));

            // Hook the native allocator and register this object.
            // .Dispose() will be called on this object when the allocator is shut down.
            Allocator.Singleton.RegisterForDisposal(this);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Capture"/> class from an existing native handle.
        /// </summary>
        /// <param name="takeOwnership">If false, the constructor will take a new reference on the handle.</param>
        /// <param name="handle">Native handle to a k4a_capture_t.</param>
        /// <remarks>
        /// Disposing this Capture will always release a reference on the handle.
        /// If <paramref name="takeOwnership"/> is true, the handle may not be owned by any existing Capture object.
        /// </remarks>
        public Capture(bool takeOwnership, IntPtr handle)
            : this(new NativeMethods.k4a_capture_t(takeOwnership, handle))
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="Capture"/> class.
        /// </summary>
        /// <param name="handle">Native handle of the Capture.</param>
        internal Capture(NativeMethods.k4a_capture_t handle)
        {
            // Hook the native allocator and register this object.
            // .Dispose() will be called on this object when the allocator is shut down.
            Allocator.Singleton.RegisterForDisposal(this);

            this.handle = handle;
        }

        /// <summary>
        /// Gets or sets the color image of this capture.
        /// </summary>
        /// <remarks>
        /// Images assigned to this capture are owned by the Capture. When the Capture is disposed, the
        /// Color Image will be disposed.
        ///
        /// By setting the Color image on a Capture, the Capture takes ownership and the Capture will
        /// dispose the Image when the Capture is disposed.
        ///
        /// To get an instance of an Image that lives longer than the capture, call Image.Reference().
        /// </remarks>
        [SuppressMessage("Design", "CA1062:Validate arguments of public methods", Justification = "SetImageWrapperAndDisposePrevious uses null to clear the stored image.")]
        public Image Color
        {
            get
            {
                this.UpdateImageWrapperAndDisposePrevious(NativeMethods.k4a_capture_get_color_image, ref this.cachedColor);

                return this.cachedColor;
            }

            set
            {
                this.SetImageWrapperAndDisposePrevious(NativeMethods.k4a_capture_set_color_image, ref this.cachedColor, value);
            }
        }

        /// <summary>
        /// Gets or sets the depth image of this capture.
        /// </summary>
        /// <remarks>
        /// Images assigned to this capture are owned by the Capture. When the Capture is disposed, the
        /// Depth Image will be disposed.
        ///
        /// By setting the Depth image on a Capture, the Capture takes ownership and the Capture will
        /// dispose the Image when the Capture is disposed.
        ///
        /// To get an instance of an Image that lives longer than the capture, call Image.Reference().
        /// </remarks>
        [SuppressMessage("Design", "CA1062:Validate arguments of public methods", Justification = "SetImageWrapperAndDisposePrevious uses null to clear the stored image.")]
        public Image Depth
        {
            get
            {
                this.UpdateImageWrapperAndDisposePrevious(NativeMethods.k4a_capture_get_depth_image, ref this.cachedDepth);

                return this.cachedDepth;
            }

            set
            {
                this.SetImageWrapperAndDisposePrevious(NativeMethods.k4a_capture_set_depth_image, ref this.cachedDepth, value);
            }
        }

        /// <summary>
        /// Gets or sets the IR image of this capture.
        /// </summary>
        /// <remarks>
        /// Images assigned to this capture are owned by the Capture. When the Capture is disposed, the
        /// IR Image will be disposed.
        ///
        /// By setting the IR image on a Capture, the Capture takes ownership and the Capture will
        /// dispose the Image when the Capture is disposed.
        ///
        /// To get an instance of an Image that lives longer than the capture, call Image.Reference().
        /// </remarks>
        [SuppressMessage("Design", "CA1062:Validate arguments of public methods", Justification = "SetImageWrapperAndDisposePrevious uses null to clear the stored image.")]
        public Image IR
        {
            get
            {
                this.UpdateImageWrapperAndDisposePrevious(NativeMethods.k4a_capture_get_ir_image, ref this.cachedIR);

                return this.cachedIR;
            }

            set
            {
                this.SetImageWrapperAndDisposePrevious(NativeMethods.k4a_capture_set_ir_image, ref this.cachedIR, value);
            }
        }

        /// <summary>
        /// Gets or sets the device temperature at the time of the capture.
        /// </summary>
        /// <remarks>
        /// Temperature is represented in degrees Celsius.
        /// </remarks>
        public float Temperature
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Capture));
                    }

                    return NativeMethods.k4a_capture_get_temperature_c(this.handle);
                }
            }

            set
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Capture));
                    }

                    NativeMethods.k4a_capture_set_temperature_c(this.handle, value);
                }
            }
        }

        /// <summary>
        /// Gets the native handle.
        /// </summary>
        /// <remarks>This is the value of the k4a_capture_t handle of the native library.
        ///
        /// This handle value can be used to interoperate with other native libraries that use
        /// Azure Kinect objects.
        ///
        /// When using this handle value, the caller is responsible for ensuring that the
        /// Capture object does not become disposed.</remarks>
        public IntPtr Handle
        {
            get
            {
                lock (this)
                {
                    if (this.disposedValue)
                    {
                        throw new ObjectDisposedException(nameof(Capture));
                    }

                    return this.handle.DangerousGetHandle();
                }
            }
        }

        /// <summary>
        /// Creates a duplicate reference to the same Capture.
        /// </summary>
        /// <returns>A new Capture object representing the same data.</returns>
        /// <remarks>Creating a reference to the same capture does not copy the capture,
        /// or the image data stored in the capture, but creates a new managed object representing
        /// the same capture data. Each object must be independently disposed. The lifetime of the
        /// underlying capture data will be equal or greater than all of the referenced Capture objects.</remarks>
        public Capture Reference()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Capture));
                }

#pragma warning disable CA2000 // Dispose objects before losing scope
                return new Capture(this.handle.DuplicateReference());
#pragma warning restore CA2000 // Dispose objects before losing scope
            }
        }

        /// <inheritdoc/>
        public void Dispose()
        {
            this.Dispose(true);

            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Gets the native handle.
        /// </summary>
        /// <returns>The native handle that is wrapped by this capture.</returns>
        /// <remarks>The function is dangerous because there is no guarantee that the
        /// handle will not be disposed once it is retrieved. This should only be called
        /// by code that can ensure that the Capture object will not be disposed on another
        /// thread.</remarks>
        internal NativeMethods.k4a_capture_t DangerousGetHandle()
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Capture));
                }

                return this.handle;
            }
        }

        /// <summary>
        /// Checks two captures to determine if they represent the same native capture object.
        /// </summary>
        /// <param name="other">Another Capture to compare against.</param>
        /// <returns>true if the Captures represent the same native k4a_capture_t.</returns>
        internal bool NativeEquals(Capture other)
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
        /// Handle the disposing of the object.
        /// </summary>
        /// <param name="disposing">true when called by Dispose(), false when called by the finalizer.</param>
        protected virtual void Dispose(bool disposing)
        {
            lock (this)
            {
                if (!this.disposedValue && disposing)
                {
                    this.cachedColor?.Dispose();
                    this.cachedDepth?.Dispose();
                    this.cachedIR?.Dispose();

                    this.cachedColor = null;
                    this.cachedDepth = null;
                    this.cachedIR = null;

                    this.handle.Close();
                    this.handle = null;

                    Allocator.Singleton.UnregisterForDisposal(this);
                }

                this.disposedValue = true;
            }
        }

        /// <summary>
        /// Retrieves a native image handle from the native API.
        /// </summary>
        /// <param name="nativeMethod">Native method to retrieve the image.</param>
        /// <param name="cachedImage">A cached instance of the Image that we return to callers. (Callers may dispose this image, although they shouldn't).</param>
        /// <remarks>
        /// If this is the first time calling the property, we construct a new wrapper.
        /// If the handle is for an Image we have already constructed a wrapper for, we return the existing wrapper.
        /// If the handle is for a different Image, we construct a new wrapper and dispose the old one.
        /// If existing wrapper has been disposed, we throw an exception.
        /// </remarks>
        private void UpdateImageWrapperAndDisposePrevious(
            Func<NativeMethods.k4a_capture_t, NativeMethods.k4a_image_t> nativeMethod,
            ref Image cachedImage)
        {
            // Lock must be held to ensure the Image in cache is not replaced while we are inspecting it
            // It is still possible for that Image to be accessed or Disposed while this lock is held
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Capture));
                }

                // Get the image object from the native SDK
                // We now own a reference on this image and must eventually release it.
                NativeMethods.k4a_image_t nativeImageHandle = nativeMethod(this.handle);

                // This try block ensures that we close image (and release our reference) if we fail
                // to create a new Image.
                try
                {
                    // If this Capture previously had an image
                    if (cachedImage != null)
                    {
                        // Get the native pointer
                        IntPtr imageHandleValue = nativeImageHandle.DangerousGetHandle();

                        // Get the handle value from the cached image. If it has been disposed, this
                        // will throw an exception, which will be passed to the caller.

                        // Take an extra reference on the cachedImage to ensure it doesn't get disposed
                        // after getting the IntPtr from the handle.
                        using (Image reference = cachedImage.Reference())
                        {
                            if (reference.DangerousGetHandle().DangerousGetHandle() != imageHandleValue)
                            {
                                // The image has changed, invalidate the current image and construct new wrappers
                                cachedImage.Dispose();
                                cachedImage = null;
                            }
                        }
                    }

                    if (cachedImage == null && !nativeImageHandle.IsInvalid)
                    {
                        // Construct a new wrapper and return it
                        // The native function may have returned
#pragma warning disable CA2000 // Dispose objects before losing scope
                        cachedImage = new Image(nativeImageHandle);
#pragma warning restore CA2000 // Dispose objects before losing scope

                        // Since we have wrapped image, it is now owned by Image and we should no longer close it
                        nativeImageHandle = null;
                    }
                }
                finally
                {
                    // Ensure the native handle is closed if we have a failure creating the Image object
                    if (nativeImageHandle != null)
                    {
                        nativeImageHandle.Close();
                    }
                }
            }
        }

        /// <summary>
        /// Sets the image wrapper provided to a property.
        /// </summary>
        /// <param name="nativeMethod">Native set method.</param>
        /// <param name="cachedImage">Reference to the cached image wrapper used by this class.</param>
        /// <param name="value">Value to assign the image wrapper to.</param>
        /// <remarks>
        /// This function takes ownership of the wrapper and stores it in the class. If there was
        /// a previous wrapper owned by the class, this function will dispose it.
        /// </remarks>
        private void SetImageWrapperAndDisposePrevious(
            Action<NativeMethods.k4a_capture_t, NativeMethods.k4a_image_t> nativeMethod,
            ref Image cachedImage,
            Image value)
        {
            lock (this)
            {
                if (this.disposedValue)
                {
                    throw new ObjectDisposedException(nameof(Capture));
                }

                // If the assignment is a new managed wrapper we need
                // to release the reference to the old wrapper.
                // If it is the same object though, we should not dispose it.
                if (cachedImage != null && !object.ReferenceEquals(
                    cachedImage, value))
                {
                    cachedImage.Dispose();
                }

                cachedImage = value;

                // Take an extra reference on the image to ensure it isn't disposed
                // prior while we have the handle.
                using (Image reference = cachedImage.Reference())
                {
                    nativeMethod(this.handle, cachedImage.DangerousGetHandle());
                }
            }
        }
    }
}
