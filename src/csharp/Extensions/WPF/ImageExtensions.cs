﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
using System;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace Microsoft.Azure.Kinect.Sensor.WPF
{
    public static class ImageExtensions
    {
        public static BitmapSource CreateBitmapSource(this Image image, double dpiX = 300, double dpiY = 300)
        {
            PixelFormat pixelFormat;

            // Take a new reference on the image to ensure that the object
            // cannot be disposed by another thread while we have a copy of its UnsafeBufferPointer
            using (Image reference = image.Reference())
            {

                switch (reference.Format)
                {
                    case ImageFormat.ColorBGRA32:
                        pixelFormat = PixelFormats.Bgra32;
                        break;
                    case ImageFormat.Depth16:
                    case ImageFormat.IR16:
                        pixelFormat = PixelFormats.Gray16;
                        break;
                    default:
                        throw new AzureKinectException($"Pixel format {reference.Format} cannot be converted to a BitmapSource");
                }

                // BitmapSource.Create copies the unmanaged memory, so there is no need to keep
                // a reference after we have created the BitmapSource objects

                unsafe
                {
                    return BitmapSource.Create(
                                reference.WidthPixels,
                                reference.HeightPixels,
                                dpiX,
                                dpiY,
                                pixelFormat,
                                /* palette: */ null,
                                (IntPtr)reference.Buffer,
                                checked((int)reference.Size),
                                reference.StrideBytes);
                }
            }
        }
    }
}
