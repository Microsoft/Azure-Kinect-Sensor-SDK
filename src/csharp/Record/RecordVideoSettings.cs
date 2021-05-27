//------------------------------------------------------------------------------
// <copyright file="RecordVideoSettings.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System.Runtime.InteropServices;

namespace Microsoft.Azure.Kinect.Sensor.Record
{
    /// <summary>
    /// Structure containing additional metadata specific to custom video tracks.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public class RecordVideoSettings
    {
        /// <summary>
        /// Gets or sets frame width of the video.
        /// </summary>
        public long Width { get; set; }

        /// <summary>
        /// Gets or sets frame height of the video.
        /// </summary>
        public long Height { get; set; }

        /// <summary>
        /// Gets or sets frame rate of the video.
        /// </summary>
        public long FrameRate { get; set; }
    }
}
