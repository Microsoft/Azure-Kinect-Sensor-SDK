﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
using System;
using System.Drawing;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Azure.Kinect.Sensor.WinForms;

namespace Microsoft.Azure.Kinect.Sensor.Examples.WinForms
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private async void Form1_Load(object sender, EventArgs e)
        {
            using (Device device = Device.Open(0))
            {
                device.StartCameras(new DeviceConfiguration
                {
                    ColorFormat = ImageFormat.ColorBGRA32,
                    ColorResolution = ColorResolution.r1080p,
                    DepthMode = DepthMode.NFOV_2x2Binned,
                    SynchronizedImagesOnly = true
                });

                while (true)
                {
                    using (Capture capture = await Task.Run(() => { return device.GetCapture(); }))
                    {
                        pictureBoxColor.Image = capture.Color.CreateBitmap();

                        pictureBoxDepth.Image = await Task.Run(() =>
                        {
                            Bitmap depthVisualization = new Bitmap(capture.Depth.WidthPixels, capture.Depth.HeightPixels, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

                            
                            //BitmapData d = depthVisualization.LockBits(new Rectangle(0, 0, depthVisualization.Width, depthVisualization.Height), System.Drawing.Imaging.ImageLockMode.ReadWrite, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                            try
                            {
                                UInt16[] depthValues = new UInt16[capture.Depth.WidthPixels * capture.Depth.HeightPixels];


                                capture.Depth.CopyTo(depthValues, 0, 0, depthValues.Length);
                                for (int y = 0; y < capture.Depth.HeightPixels; y++)
                                {
                                    for (int x = 0; x < capture.Depth.WidthPixels; x++)
                                    {
                                        ushort depthValue = depthValues[y * capture.Depth.WidthPixels + x];

                                        if (depthValue == 0)
                                        {
                                            depthVisualization.SetPixel(x, y, Color.Red);
                                        }
                                        else if (depthValue == ushort.MaxValue)
                                        {
                                            depthVisualization.SetPixel(x, y, Color.Green);
                                        }
                                        else
                                        {
                                            float brightness = (float)depthValue / (float)2000;

                                            if (brightness > 1.0f)
                                                depthVisualization.SetPixel(x, y, Color.White);
                                            else
                                            {
                                                int c = (int)(brightness * 250);
                                                depthVisualization.SetPixel(x, y, Color.FromArgb(c, c, c));
                                            }

                                        }
                                    }
                                }
                            }
                            finally
                            {
                                //depthVisualization.UnlockBits(d);
                            }
                            return depthVisualization;
                        });

                        this.Invalidate();
                    }
                }
            }
        }
    }
}
