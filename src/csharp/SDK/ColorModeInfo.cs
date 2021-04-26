﻿//------------------------------------------------------------------------------
// <copyright file="NativeMethods.cs" company="Microsoft">
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// </copyright>
//------------------------------------------------------------------------------
using System;
using System.Runtime.InteropServices;
using static Microsoft.Azure.Kinect.Sensor.NativeMethods;

namespace Microsoft.Azure.Kinect.Sensor
{
    /// <summary>
    /// Represents the configuration to run a color device in.
    /// </summary>
    /// <remarks>
    /// Gets or sets the Color Mode Info.
    /// </remarks>
    public class ColorModeInfo
    {
        /// <summary>
        /// Gets the Struct Size. Default set in the NativeMethods class and must be kept in sync with k4a_color_mode_info_t k4atypes.h.
        /// </summary>
        public int StructSize { get; private set; } = colorModeInfoStructSize;

        /// <summary>
        /// Gets the Struct Version.
        /// </summary>
        public int StructVersion { get; private set; } = 1;

        /// <summary>
        /// Gets the ModeId of the color camera.
        /// </summary>
        public int ModeId { get; private set; } = 0;

        /// <summary>
        /// Gets the image Width.
        /// </summary>
        public int Width { get; private set; } = 0;

        /// <summary>
        /// Gets the image Height.
        /// </summary>
        public int Height { get; private set; } = 0;

        /// <summary>
        /// Gets the Native Image Format.
        /// </summary>
        public ImageFormat NativeFormat { get; private set; } = ImageFormat.ColorMJPG;

        /// <summary>
        /// Gets the Horizontal FOV.
        /// </summary>
        public float HorizontalFOV { get; private set; } = 0;

        /// <summary>
        /// Gets the Vertical FOV.
        /// </summary>
        public float VerticalFOV { get; private set; } = 0;

        /// <summary>
        /// Gets the Min FPS.
        /// </summary>
        public int MinFPS { get; private set; } = 0;

        /// <summary>
        /// Gets the Max FPS.
        /// </summary>
        public int MaxFPS { get; private set; } = 0;

        /// <summary>
        /// Get the equivalent native configuration struct.
        /// </summary>
        /// <returns>k4a_color_mode_info_t.</returns>
        internal k4a_color_mode_info_t GetNativeConfiguration()
        {
            return new k4a_color_mode_info_t
            {
                struct_size = (uint)this.StructSize,
                struct_version = (uint)this.StructVersion,
                mode_id = (uint)this.ModeId,
                width = (uint)this.Width,
                height = (uint)this.Height,
                native_format = (k4a_image_format_t)this.NativeFormat,
                horizontal_fov = this.HorizontalFOV,
                vertical_fov = this.VerticalFOV,
                min_fps = (uint)this.MinFPS,
                max_fps = (uint)this.MaxFPS,
            };
        }

        /// <summary>
        /// Set properties using native configuration struct.
        /// </summary>
        /// <param name="colorModeInfo">colorModeInfo.</param>
        internal void SetUsingNativeConfiguration(k4a_color_mode_info_t colorModeInfo)
        {
            this.StructSize = (int)colorModeInfo.struct_size;
            this.StructVersion = (int)colorModeInfo.struct_version;
            this.ModeId = (int)colorModeInfo.mode_id;
            this.Width = (int)colorModeInfo.width;
            this.Height = (int)colorModeInfo.height;
            this.NativeFormat = (ImageFormat)colorModeInfo.native_format;
            this.HorizontalFOV = colorModeInfo.horizontal_fov;
            this.VerticalFOV = colorModeInfo.vertical_fov;
            this.MinFPS = (int)colorModeInfo.min_fps;
            this.MaxFPS = (int)colorModeInfo.max_fps;
        }
    }
}
