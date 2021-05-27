//------------------------------------------------------------------------------
// <copyright file="PlaybackSeekOrigin.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
namespace Microsoft.Azure.Kinect.Sensor.Record
{
    /// <summary>
    /// The origin for relative seek operations.
    /// </summary>
    public enum PlaybackSeekOrigin
    {
        /// <summary>
        /// The seek operation is relative to the beginning of the file.
        /// </summary>
        Begin = 0,

        /// <summary>
        /// The seek operation is relative to the end of the file.
        /// </summary>
        End,

        /// <summary>
        /// The seek operation is specified in the device time.
        /// </summary>
        DeviceTime,
    }
}
