using System;
using Microsoft.Azure.Kinect.Sensor;
using Microsoft.Azure.Kinect.Sensor.Record;
using NUnit.Framework;

namespace Tests
{
    /// <summary>
    /// Loopback Tests write to a recording, and then read the recording back to verify the API.
    /// </summary>
    public class LoopbackTests
    {
        private string recordingPath;

        /// <summary>
        /// Allocate a path for the recording.
        /// </summary>
        [SetUp]
        public void Setup()
        {
            this.recordingPath = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "testfile.mkv");
        }

        /// <summary>
        /// Delete the temporary recording.
        /// </summary>
        [TearDown]
        public void TearDown()
        {
            System.IO.File.Delete(this.recordingPath);
        }

        /// <summary>
        /// Writes each of the data types to a file and reads them back.
        /// Verfies as many properties as possible.
        /// </summary>
        [Test]
        public void LoopbackTest1()
        {
            DeviceConfiguration deviceConfiguration = new DeviceConfiguration()
            {
                CameraFPS = FPS.FPS30,
                ColorFormat = ImageFormat.ColorNV12,
                ColorResolution = ColorResolution.R720p,
                DepthDelayOffColor = TimeSpan.FromMilliseconds(123),
                DepthMode = DepthMode.NFOV_2x2Binned,
                DisableStreamingIndicator = true,
                SuboridinateDelayOffMaster = TimeSpan.FromMilliseconds(456),
                SynchronizedImagesOnly = true,
                WiredSyncMode = WiredSyncMode.Subordinate,
            };

            using (Recorder record = Recorder.Create(this.recordingPath, null, deviceConfiguration))
#pragma warning restore CA1508 // Avoid dead conditional code
            {
                record.AddImuTrack();
                record.AddCustomVideoTrack("CUSTOM_VIDEO", "V_CUSTOM1", new byte[] { 1, 2, 3 }, new RecordVideoSettings() { FrameRate = 1, Height = 10, Width = 20 });
                record.AddCustomSubtitleTrack("CUSTOM_SUBTITLE", "S_CUSTOM1", new byte[] { 4, 5, 6, 7 }, new RecordSubtitleSettings() { HighFrequencyData = false });
                record.AddTag("MYTAG1", "one");
                record.AddTag("MYTAG2", "two");

                record.WriteHeader();

                for (int i = 0; i < 10; i++)
                {
                    double timeStamp = 10.0 + (i * 1.0);

#pragma warning disable CA1508 // Avoid dead conditional code
                    using (Capture c = new Capture())
#pragma warning restore CA1508 // Avoid dead conditional code
                    {
                        c.Color = new Image(ImageFormat.ColorNV12, 1280, 720);
                        c.IR = new Image(ImageFormat.IR16, 320, 288);
                        c.Depth = new Image(ImageFormat.Depth16, 320, 288);
                        c.Temperature = 25.0f;
                        c.Color.DeviceTimestamp = TimeSpan.FromSeconds(timeStamp);
                        c.Depth.DeviceTimestamp = TimeSpan.FromSeconds(timeStamp) + deviceConfiguration.DepthDelayOffColor;
                        c.IR.DeviceTimestamp = TimeSpan.FromSeconds(timeStamp) + deviceConfiguration.DepthDelayOffColor;

                        c.Color.Exposure = TimeSpan.FromMilliseconds(12);
                        c.Color.ISOSpeed = 100;
                        c.Color.SystemTimestampNsec = System.Diagnostics.Stopwatch.GetTimestamp();
                        c.Color.WhiteBalance = 2;

                        c.Depth.SystemTimestampNsec = System.Diagnostics.Stopwatch.GetTimestamp();

                        c.IR.SystemTimestampNsec = System.Diagnostics.Stopwatch.GetTimestamp();

                        record.WriteCapture(c);
                    }

                    for (int y = 0; y < 10; y++)
                    {
                        ImuSample imuSample = new ImuSample()
                        {
                            AccelerometerSample = new System.Numerics.Vector3(1.0f, 2.0f, 3.0f),
                            GyroSample = new System.Numerics.Vector3(4.0f, 5.0f, 6.0f),
                            AccelerometerTimestamp = TimeSpan.FromSeconds(timeStamp + (0.1 * y)),
                            GyroTimestamp = TimeSpan.FromSeconds(timeStamp + (0.1 * y)),
                            Temperature = 26.0f,
                        };

                        record.WriteImuSample(imuSample);
                    }

                    byte[] customData = new byte[i + 1];
                    for (int x = 0; x < customData.Length; x++)
                    {
                        customData[x] = (byte)(i + x);
                    }

                    record.WriteCustomTrackData("CUSTOM_VIDEO", TimeSpan.FromSeconds(timeStamp), customData);

                    for (int x = 0; x < customData.Length; x++)
                    {
                        customData[x] = (byte)(i + x + 1);
                    }

                    record.WriteCustomTrackData("CUSTOM_SUBTITLE", TimeSpan.FromSeconds(timeStamp), customData);

                    record.Flush();
                }
            }

            using (Playback playback = Playback.Open(this.recordingPath))
            {
                Assert.IsTrue(playback.CheckTrackExists("CUSTOM_VIDEO"));
                Assert.IsTrue(playback.CheckTrackExists("CUSTOM_SUBTITLE"));
                Assert.AreEqual("V_CUSTOM1", playback.GetTrackCodecId("CUSTOM_VIDEO"));
                Assert.AreEqual(new byte[] { 1, 2, 3 }, playback.GetTrackCodecContext("CUSTOM_VIDEO"));

                Assert.AreEqual("one", playback.GetTag("MYTAG1"));
                Assert.AreEqual("two", playback.GetTag("MYTAG2"));

                Assert.AreEqual(FPS.FPS30, playback.RecordConfiguration.CameraFPS);
                Assert.AreEqual(ImageFormat.ColorNV12, playback.RecordConfiguration.ColorFormat);
                Assert.AreEqual(TimeSpan.FromMilliseconds(456), playback.RecordConfiguration.SubordinateDelayOffMaster);

                Assert.IsFalse(playback.Calibration.HasValue);

                for (int i = 0; i < 10; i++)
                {
                    double timeStamp = 10.0 + (i * 1.0);

                    using (Capture c = playback.GetNextCapture())
                    {
                        // Not captured in recording
                        // Assert.AreEqual(25.0f, c.Temperature);
                        Assert.AreEqual(ImageFormat.ColorNV12, c.Color.Format);
                        Assert.AreEqual(1280, c.Color.WidthPixels);
                        Assert.AreEqual(720, c.Color.HeightPixels);

                        Assert.AreEqual(TimeSpan.FromSeconds(timeStamp), c.Color.DeviceTimestamp);
                        Assert.AreEqual(TimeSpan.FromSeconds(timeStamp) + deviceConfiguration.DepthDelayOffColor, c.Depth.DeviceTimestamp);
                        Assert.AreEqual(TimeSpan.FromSeconds(timeStamp) + deviceConfiguration.DepthDelayOffColor, c.IR.DeviceTimestamp);

                        // Not captured in recording
                        // Assert.AreEqual(TimeSpan.FromMilliseconds(12), c.Color.Exposure);

                        // Not captured in recording
                        // Assert.AreEqual(100, c.Color.ISOSpeed);
                        Assert.AreEqual(0, c.Color.SystemTimestampNsec);

                        // Not captured in recording
                        // Assert.AreEqual(2, c.Color.WhiteBalance);
                    }

                    for (int y = 0; y < 10; y++)
                    {
                        ImuSample imuSample = new ImuSample()
                        {
                            AccelerometerSample = new System.Numerics.Vector3(1.0f, 2.0f, 3.0f),
                            GyroSample = new System.Numerics.Vector3(4.0f, 5.0f, 6.0f),
                            AccelerometerTimestamp = TimeSpan.FromSeconds(timeStamp + 0.1 * y),
                            GyroTimestamp = TimeSpan.FromSeconds(timeStamp + 0.1 * y),
                            Temperature = 26.0f,
                        };

                        ImuSample readSample = playback.GetNextImuSample();

                        Assert.AreEqual(imuSample.AccelerometerSample, readSample.AccelerometerSample);
                        Assert.AreEqual(imuSample.GyroSample, readSample.GyroSample);
                        Assert.AreEqual(imuSample.AccelerometerTimestamp, readSample.AccelerometerTimestamp);
                        Assert.AreEqual(imuSample.GyroTimestamp, readSample.GyroTimestamp);

                        // Not captured in recording
                        // Assert.AreEqual(imuSample.Temperature, readSample.Temperature);
                    }

                    byte[] customData = new byte[i + 1];
                    for (int x = 0; x < customData.Length; x++)
                    {
                        customData[x] = (byte)(i + x);
                    }

                    using (DataBlock videoBlock = playback.GetNextDataBlock("CUSTOM_VIDEO"))
                    {
                        Assert.AreEqual(customData, videoBlock.Memory.ToArray());
                        Assert.AreEqual(TimeSpan.FromSeconds(timeStamp), videoBlock.DeviceTimestamp);
                    }

                    for (int x = 0; x < customData.Length; x++)
                    {
                        customData[x] = (byte)(i + x + 1);
                    }

                    using (DataBlock subtitleBlock = playback.GetNextDataBlock("CUSTOM_SUBTITLE"))
                    {
                        Assert.AreEqual(customData, subtitleBlock.Memory.ToArray());
                        Assert.AreEqual(TimeSpan.FromSeconds(timeStamp), subtitleBlock.DeviceTimestamp);
                    }
                }
            }
        }
    }
}