﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
using System.Threading.Tasks;
using System.Windows;
using Microsoft.Azure.Kinect.Sensor;
using Microsoft.Azure.Kinect.Sensor.WPF;

namespace Microsoft.Azure.Kinect.Sensor.Examples.WPFViewer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            using (Device device = Device.Open(0))
            {
                device.StartCameras(new DeviceConfiguration {
                    ColorFormat = ImageFormat.ColorBGRA32,
                    ColorResolution = ColorResolution.r1440p,
                    DepthMode = DepthMode.WFOV_2x2Binned,
                    SynchronizedImagesOnly = true
                });

                int colorWidth = device.GetCalibration().color_camera_calibration.resolution_width;
                int colorHeight = device.GetCalibration().color_camera_calibration.resolution_height;

                // Allocate image buffers for us to manipulate
                using (ArrayImage<BGRA> modifiedColor = new ArrayImage<BGRA>(ImageFormat.ColorBGRA32, colorWidth, colorHeight))
                using (ArrayImage<ushort> transformedDepth = new ArrayImage<ushort>(ImageFormat.Depth16, colorWidth, colorHeight))
                using (Transformation transform = device.GetCalibration().CreateTransformation())
                {
                    while (true)
                    {
                        // Wait for a capture on a thread pool thread
                        using (Capture capture = await Task.Run(() => { return device.GetCapture(); }))
                        {
                            // Update the color image preview
                            colorImageViewPane.Source = capture.Color.CreateBitmapSource();

                            // Compute the depth preview on a thread pool thread
                            depthImageViewPane.Source = (await Task.Run(() =>
                            {
                                // Transform the depth image
                                transform.DepthImageToColorCamera(capture, transformedDepth);
                                // Copy the color image
                                capture.Color.CopyBytesTo(modifiedColor, 0, 0, (int)modifiedColor.Size);

                                ushort[] depthBuffer = transformedDepth.Buffer;
                                BGRA[] colorBuffer = modifiedColor.Buffer;

                                // Modify the color image with data from the depth image
                                for (int i = 0; i < colorBuffer.Length; i++)
                                {
                                    if (depthBuffer[i] == 0)
                                    {
                                        colorBuffer[i].R = 255;
                                    }
                                    else if (depthBuffer[i] > 1500)
                                    {
                                        colorBuffer[i].G = 255;
                                    }
                                }

                                return modifiedColor;
                            })).CreateBitmapSource();
                        }
                    }
                }
            }
        }
    }
}
