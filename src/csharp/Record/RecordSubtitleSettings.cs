//------------------------------------------------------------------------------
// <copyright file="RecordSubtitleSettings.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Runtime.InteropServices;

namespace Microsoft.Azure.Kinect.Sensor.Record
{
    /// <summary>
    /// Settings for a recording subtitle track.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class RecordSubtitleSettings
    {
        /// <summary>
        /// Gets or sets a value indicating whether data will be grouped together to reduce overhead.
        /// </summary>
        /// <remarks>
        /// If set, only a single timestamp will be stored per batch, and an estimated timestamp will be use by <see cref="Playback.Seek(TimeSpan, PlaybackSeekOrigin)"/> and <see cref="DataBlock.DeviceTimestamp"/>.
        /// The estimated timestamp is calculated with the assumption that blocks are evenly spaced within a batch.</remarks>
        public bool HighFrequencyData { get; set; }
    }
}
