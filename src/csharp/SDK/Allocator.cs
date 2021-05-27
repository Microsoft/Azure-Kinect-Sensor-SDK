//------------------------------------------------------------------------------
// <copyright file="Allocator.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Buffers;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[assembly: InternalsVisibleTo("Microsoft.Azure.Kinect.Sensor.UnitTests, PublicKey=0024000004800000940000000602000000240000525341310004000001000100f54d726c53826f259c2372d3fe68e1a122f58989796aa52500b930ff33cb0e57431b25a780b0d55c45470c8835f7a335425ef6706f9dbcf7046fe6f479d723a51f6c2c9630d986b5cadb47094d5fd6c64cb144736af739fc071cb53c1296d27e38ee99a2ac2329ed141e645b4669b32568a71607c8f7e986418825f2d37c48cf")]

namespace Microsoft.Azure.Kinect.Sensor
{
    /// <summary>
    /// Manages buffer allocation.
    /// </summary>
    internal class Allocator
    {
        // Objects we are tracking for disposal prior to CLR shutdown
        private readonly HashSet<WeakReference<IDisposable>> disposables = new HashSet<WeakReference<IDisposable>>();

        // Allocations made by the managed code for the native library
        private readonly SortedDictionary<long, AllocationContext> allocations = new SortedDictionary<long, AllocationContext>();

        // A recyclable large array pool to prevent unnecessary managed allocations and clearing of memory
        private readonly ArrayPool<byte> pool = new LargeArrayPool();

        // Managed buffers used as caches for the native memory.
        private readonly Dictionary<IntPtr, BufferCacheEntry> bufferCache = new Dictionary<IntPtr, BufferCacheEntry>();

        // Native delegates
        private readonly NativeMethods.k4a_memory_allocate_cb_t allocateDelegate;
        private readonly NativeMethods.k4a_memory_destroy_cb_t freeDelegate;

        // Native allocator hook state
        private bool hooked = false;

        // This is set when the CLR is shutting down, causing an exception during
        // any new object registrations.
        private bool noMoreDisposalRegistrations = false;

        private Allocator()
        {
            this.allocateDelegate = new NativeMethods.k4a_memory_allocate_cb_t(this.AllocateFunction);
            this.freeDelegate = new NativeMethods.k4a_memory_destroy_cb_t(this.FreeFunction);

            // Register for ProcessExit and DomainUnload to allow us to unhook the native layer before
            // native to managed callbacks are no longer allowed.
            AppDomain.CurrentDomain.DomainUnload += this.ApplicationExit;
            AppDomain.CurrentDomain.ProcessExit += this.ApplicationExit;

            // Default to the safe and performant configuration

            // Use Managed Allocator will cause the native layer to allocate from managed byte[] arrays when possible
            // these can then be safely referenced with a Memory<T> and exposed to user code. This should have fairly
            // minimal performance impact, but provides strong memory safety.
            this.UseManagedAllocator = true;

            // When managed code needs to provide access to memory that didn't originate from the managed allocator
            // the SafeCopyNativeBuffers options causes the managed code to make a safe cache copy of the native buffer
            // in a managed byte[] array. This has a more significant performance impact, but generally is only needed for
            // media foundation (color image) or potentially custom buffers. When set to true, the Memory<T> objects are safe
            // copies of the native buffers. When set to false, the Memory<T> objects are direct pointers to the native buffers
            // and can therefore cause native memory corruption if a Memory<T> (or Span<T>) is used after the Image is disposed
            // or garbage collected.
            this.SafeCopyNativeBuffers = true;
        }

        /// <summary>
        /// Gets the Allocator.
        /// </summary>
        public static Allocator Singleton { get; } = new Allocator();

        /// <summary>
        /// Gets or sets a value indicating whether to have the native library use the managed allocator.
        /// </summary>
        public bool UseManagedAllocator
        {
            get => this.hooked;

            set
            {
                lock (this)
                {
                    if (value && !this.hooked)
                    {
                        try
                        {
                            AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_set_allocator(this.allocateDelegate, this.freeDelegate));
                            this.hooked = true;
                        }
#pragma warning disable CA1031 // Do not catch general exception types
                        catch (Exception)
#pragma warning restore CA1031 // Do not catch general exception types
                        {
                            // Don't fail if we can't set the allocator since this code path is called during the global type
                            // initialization. A failure to set the allocator is also not fatal, but will only cause a performance
                            // issue.
                            System.Diagnostics.Debug.WriteLine("Unable to hook native allocator");
                        }
                    }

                    if (!value && this.hooked)
                    {
                        // Disabling the hook once it has been enabled should not catch the exception
                        AzureKinectException.ThrowIfNotSuccess(() => NativeMethods.k4a_set_allocator(null, null));
                        this.hooked = false;
                    }
                }
            }
        }

        /// <summary>
        /// Gets or sets a value indicating whether to make a safe copy of native buffers.
        /// </summary>
        public bool SafeCopyNativeBuffers { get; set; } = true;

        /// <summary>
        /// Register the object for disposal when the CLR shuts down.
        /// </summary>
        /// <param name="disposable">Object to dispose before native hooks are disconnected.</param>
        /// <remarks>
        /// When the CLR shuts down, native callbacks in to the CLR result in an application crash. The allocator free method
        /// is a native callback to the managed layer that is called whenever the hooked native API needs to free memory.
        ///
        /// To avoid this callback after the CLR shuts down, the native library must be completely cleaned up prior CLR shutdown.
        ///
        /// Any object that may hold references to the native library (and will therefore generate native to manged callbacks when it
        /// gets cleaned up) should register with the RegisterForDisposal method to ensure it is cleaned up in the correct order.
        /// during shutdown.
        /// </remarks>
        public void RegisterForDisposal(IDisposable disposable)
        {
            lock (this)
            {
                if (this.noMoreDisposalRegistrations)
                {
                    throw new InvalidOperationException("New objects may not be registered during shutdown.");
                }

                // Track the object as one we may need to dispose during shutdown.
                // Use a weak reference to allow the object to be garbage collected earlier if possible.
                _ = this.disposables.Add(new WeakReference<IDisposable>(disposable));
            }
        }

        /// <summary>
        /// Unregister the object for disposal.
        /// </summary>
        /// <param name="disposable">Object to unregister.</param>
        /// <remarks>
        /// This does not unhook the native allocator, but only unregisters the object for
        /// disposal.
        /// </remarks>
        public void UnregisterForDisposal(IDisposable disposable)
        {
            lock (this)
            {
                // Remove the object and clean up any dead weak references.
                _ = this.disposables.RemoveWhere((r) =>
                      {
                          bool alive = r.TryGetTarget(out IDisposable target);

                          return !alive || target == disposable;
                      });
            }
        }

        /// <summary>
        /// Get a Memory reference to the managed memory that was used by the hooked native
        /// allocator.
        /// </summary>
        /// <param name="address">Native address of the memory.</param>
        /// <param name="size">Size of the memory region.</param>
        /// <returns>Reference to the memory, or an empty memory reference.</returns>
        /// <remarks>
        /// If the address originally came from a managed array that was provided to the native
        /// API through the allocator hook, this function will return a Memory reference to the managed
        /// memory. Since this is a reference to the managed memory and not the native pointer, it
        /// is safe and not subject to use after free bugs.
        ///
        /// The address and size do not need to reference the exact pointer provided to the native layer
        /// by the allocator, but can refer to any region in the allocated memory.
        /// </remarks>
        public Memory<byte> GetManagedAllocatedMemory(IntPtr address, long size)
        {
            lock (this)
            {
                AllocationContext allocation = this.FindNearestContext(address);
                if (allocation == null)
                {
                    return null;
                }

                long offset = (long)address - (long)allocation.BufferAddress;

                // Check that the beginning of the memory is in this allocation
                if (offset > allocation.Buffer.LongLength)
                {
                    return null;
                }

                // Check that the end of the memory is in this allocation
                if (checked(offset + size) > allocation.Buffer.LongLength)
                {
                    return null;
                }

                // Return a reference to this memory
                return new Memory<byte>(allocation.Buffer, checked((int)offset), checked((int)size));
            }
        }

        /// <summary>
        /// Get a managed array to cache the contents of a native buffer.
        /// </summary>
        /// <param name="nativeAddress">Native buffer to mirror.</param>
        /// <param name="size">Size of the native memory.</param>
        /// <returns>A managed array populated with the content of the native buffer.</returns>
        /// <remarks>Multiple callers asking for the same address will get the same buffer.
        /// When done with the buffer the caller must call <seealso cref="ReturnBufferCache(IntPtr)"/>.
        /// </remarks>
        public byte[] GetBufferCache(IntPtr nativeAddress, int size)
        {
            BufferCacheEntry entry;

            lock (this)
            {
                if (this.bufferCache.ContainsKey(nativeAddress))
                {
                    entry = this.bufferCache[nativeAddress];
                    entry.ReferenceCount++;

                    if (entry.UsedSize != size)
                    {
                        throw new AzureKinectException("Multiple image buffers sharing the same address cannot have the same size");
                    }
                }
                else
                {
                    entry = new BufferCacheEntry
                    {
                        ManagedBufferCache = this.pool.Rent(size),
                        UsedSize = size,
                        ReferenceCount = 1,
                        Initialized = false,
                    };

                    this.bufferCache.Add(nativeAddress, entry);
                }
            }

            lock (entry)
            {
                if (!entry.Initialized)
                {
                    Marshal.Copy(nativeAddress, entry.ManagedBufferCache, 0, entry.UsedSize);
                    entry.Initialized = true;
                }
            }

            return entry.ManagedBufferCache;
        }

        /// <summary>
        /// Return the buffer cache.
        /// </summary>
        /// <param name="nativeAddress">Address of the native buffer.</param>
        /// <remarks>Must be called exactly once for each buffer provided by <see cref="GetBufferCache(IntPtr, int)"/>.</remarks>
        public void ReturnBufferCache(IntPtr nativeAddress)
        {
            lock (this)
            {
                BufferCacheEntry entry = this.bufferCache[nativeAddress];
                entry.ReferenceCount--;
                if (entry.ReferenceCount == 0)
                {
                    this.pool.Return(entry.ManagedBufferCache);
                    _ = this.bufferCache.Remove(nativeAddress);
                }
            }
        }

        // Find the allocation context who's address is closest but not
        // greater than the search address
        private AllocationContext FindNearestContext(IntPtr address)
        {
            lock (this)
            {
                long[] keys = new long[this.allocations.Count];
                this.allocations.Keys.CopyTo(keys, 0);

                int searchIndex = Array.BinarySearch(keys, (long)address);
                if (searchIndex >= 0)
                {
                    return this.allocations[keys[searchIndex]];
                }
                else
                {
                    int nextLowestIndex = ~searchIndex - 1;
                    if (nextLowestIndex < 0)
                    {
                        return null;
                    }

                    AllocationContext allocation = this.allocations[keys[nextLowestIndex]];

                    return allocation;
                }
            }
        }

        // This function is called by the native layer to allocate memory
        private IntPtr AllocateFunction(int size, out IntPtr context)
        {
            byte[] buffer = this.pool.Rent(size);
            AllocationContext allocationContext = new AllocationContext()
            {
                Buffer = buffer,
                BufferPin = GCHandle.Alloc(buffer, GCHandleType.Pinned),
                CallbackDelegate = this.freeDelegate,
            };

            allocationContext.BufferAddress = allocationContext.BufferPin.AddrOfPinnedObject();

            context = (IntPtr)GCHandle.Alloc(allocationContext);

            lock (this)
            {
                this.allocations.Add((long)allocationContext.BufferAddress, allocationContext);
            }

            return allocationContext.BufferAddress;
        }

        // This function is called by the native layer to free memory
        private void FreeFunction(IntPtr buffer, IntPtr context)
        {
            GCHandle contextPin = (GCHandle)context;
            AllocationContext allocationContext = (AllocationContext)contextPin.Target;

            lock (this)
            {
                System.Diagnostics.Debug.Assert(object.ReferenceEquals(this.allocations[(long)buffer], allocationContext), "Allocation context does not match expected value");
                _ = this.allocations.Remove((long)buffer);
            }

            allocationContext.BufferPin.Free();
            this.pool.Return(allocationContext.Buffer);
            contextPin.Free();
        }

        // Called when the AppDomain is unloaded or the application exits
        private void ApplicationExit(object sender, EventArgs e)
        {
            lock (this)
            {
                // Disable the managed allocator hook to ensure no new allocations
                this.UseManagedAllocator = false;

                // Prevent more disposal registrations while we are cleaning up
                this.noMoreDisposalRegistrations = true;

                System.Diagnostics.Debug.WriteLine($"Disposable count {this.disposables.Count} (Allocation Count {this.allocations.Count})");

                // First dispose of all the registered objects

                // Don't dispose of the objects during this loop since a
                // side effect of disposing of an object may be the object unregistering itself
                // and causing the collection to be modified.
                List<IDisposable> disposeList = new List<IDisposable>();
                foreach (WeakReference<IDisposable> r in this.disposables)
                {
                    if (r.TryGetTarget(out IDisposable disposable))
                    {
                        disposeList.Add(disposable);
                    }
                }

                this.disposables.Clear();

                foreach (IDisposable disposable in disposeList)
                {
                    System.Diagnostics.Debug.WriteLine($"Disposed {disposable.GetType().FullName} (Allocation Count {this.allocations.Count})");
                    disposable.Dispose();
                }

                // If the allocation count is not zero, we will be called again with a free function
                // if this happens after the CLR has entered shutdown, the CLR will generate an exception.
                if (this.allocations.Count > 0)
                {
                    throw new AzureKinectException("Not all native allocations have been freed before managed shutdown");
                }
            }
        }

        private class AllocationContext
        {
            public byte[] Buffer { get; set; }

            public IntPtr BufferAddress { get; set; }

            public GCHandle BufferPin { get; set; }

            public NativeMethods.k4a_memory_destroy_cb_t CallbackDelegate { get; set; }
        }

        private class BufferCacheEntry
        {
            public byte[] ManagedBufferCache { get; set; }

            public int UsedSize { get; set; }

            public int ReferenceCount { get; set; }

            public bool Initialized { get; set; }
        }
    }
}
