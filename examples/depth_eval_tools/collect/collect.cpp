// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <k4a/k4a.h>
#include <vector>
#include <math.h>
#include <string>
#include <algorithm>

#include <iostream> // std::cout
#include <fstream>  // std::ofstream

#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "inc/collect.h"
#include "kahelpers.h"

using namespace kahelpers;

void help()
{
    std::cout
        << "\nCollect depth and color images from K4A.\n"
           "Usage:\n"
           "./collect  -h or -help or -? print this help message\n"
           "./collect  -mode=<depth mode> -res=<color resolution> -nv=<num of views> -nc=<num of captures per view> "
           " -fps=<frame rate enum> -cal=<dump cal file> -xy=<dump xytable> -d=<capture depth> -i=<capture ir> "
           "-c=<capture color>"
           " -out=<output directory>\n"
           " -gg=<gray_gamma used to convert ir data to 8bit gray image. default=0.5>\n"
           " -gm=<gray_max used to convert ir data to 8bit gray image. default=4000.0>\n"
           " -gp=<percentile used to convert ir data to 8bit gray image. default=99.0>\n"
           " -av=<0:dump mean images only, 1:dump all images, 2:dump all images and their mean>\n"
           " Example:\n"
           "./collect  -mode=3 -res=1 -nv=2 -nc=10 -cal=1 -out=c:/data\n";
    std::cout << "\n---\ndepth mode can be [0, 1, 2, 3, 4 or 5] as follows\n";
    std::cout << "K4A_DEPTH_MODE_OFF = 0,         0:Depth sensor will be turned off with this setting.\n";
    std::cout
        << "K4A_DEPTH_MODE_NFOV_2X2BINNED,  1:Depth captured at 320x288. Passive IR is also captured at 320x288.\n";
    std::cout
        << "K4A_DEPTH_MODE_NFOV_UNBINNED,   2:Depth captured at 640x576. Passive IR is also captured at 640x576.\n";
    std::cout
        << "K4A_DEPTH_MODE_WFOV_2X2BINNED,  3:Depth captured at 512x512. Passive IR is also captured at 512x512.\n";
    std::cout
        << "K4A_DEPTH_MODE_WFOV_UNBINNED,   4:Depth captured at 1024x1024. Passive IR is also captured at 1024x1024.\n";
    std::cout << "K4A_DEPTH_MODE_PASSIVE_IR,      5:Passive IR only, captured at 1024x1024. \n";
    std::cout << "\n---\ncolor resolution can be [0, 1, 2, 3, 4, 5, or 6] as follows\n";
    std::cout << "K4A_COLOR_RESOLUTION_OFF = 0,  0: Color camera will be turned off.\n";
    std::cout << "K4A_COLOR_RESOLUTION_720P,     1: 1280 * 720  16:9. \n";
    std::cout << "K4A_COLOR_RESOLUTION_1080P,    2: 1920 * 1080 16:9. \n";
    std::cout << "K4A_COLOR_RESOLUTION_1440P,    3: 2560 * 1440 16:9. \n";
    std::cout << "K4A_COLOR_RESOLUTION_1536P,    4: 2048 * 1536 4:3. \n";
    std::cout << "K4A_COLOR_RESOLUTION_2160P,    5: 3840 * 2160 16:9. \n";
    std::cout << "K4A_COLOR_RESOLUTION_3072P,    6: 4096 * 3072 4:3. \n";
    std::cout << "\n---\nfps can be [0, 1, or 2] as follows\n";
    std::cout << "K4A_FRAMES_PER_SECOND_5 = 0,   0: FPS=5. \n";
    std::cout << "K4A_FRAMES_PER_SECOND_15,  1: FPS=15. \n";
    std::cout << "K4A_FRAMES_PER_SECOND_30,  2: FPS=30. \n";
}
int main(int argc, char **argv)
{

    cv::CommandLineParser
        parser(argc,
               argv,
               "{help h usage ?| |print this message}"
               "{mode| | depth mode:0, 1, 2, 3, 4 or 5}"
               "{res| | color res:0, 1, 2, 3, 4, 5 or 6}"
               "{out| | output dir}"
               "{nv|1| number of views}"
               "{nc|1| number of captures per view}"
               "{fps|0| frame rate per sec}"
               "{cal|0| dump calibration}"
               "{xy|0| dump xy calibration table}"
               "{d|1| capture depth}"
               "{i|1| capture ir}"
               "{c|1| capture color}"
               "{gg|0.5| gray_gamma used to convert ir data to 8bit gray image}"
               "{gm|4000.0| gray_max used to convert ir data to 8bit gray image}"
               "{gp | 99.0 | percentile used to convert ir data to 8bit gray image}"
               "{av|0| 0:dump mean images only, 1:dump all images, 2:dump all images and their mean}");
    // get input argument

    if (parser.has("help") || !parser.has("mode") || !parser.has("res") || !parser.has("out"))
    {
        help();
        return 0;
    }

    int mode = parser.get<int>("mode");
    int res = parser.get<int>("res");
    std::string output_dir = parser.get<std::string>("out");

    int num_views = parser.get<int>("nv");
    int num_caps_per_view = parser.get<int>("nc");

    int fps = parser.get<int>("fps");

    bool dump_calibration = parser.get<int>("cal") > 0;
    bool dump_xytable = parser.get<int>("xy") > 0;

    bool dump_depth = parser.get<int>("d") > 0;
    bool dump_ir = parser.get<int>("i") > 0;
    bool dump_color = parser.get<int>("c") > 0;

    int av = parser.get<int>("av");

    float gray_gamma = parser.get<float>("gg");
    float gray_max = parser.get<float>("gm");
    float gray_percentile = parser.get<float>("gp");

    uint32_t device_count = k4a::device::get_installed_count();

    if (device_count == 0)
    {
        std::cerr << "No K4A devices found\n";
        return 1;
    }

    k4a::device device = k4a::device::open(K4A_DEVICE_DEFAULT);

    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.depth_mode = (k4a_depth_mode_t)mode;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_MJPG;
    config.color_resolution = (k4a_color_resolution_t)res;
    config.camera_fps = (k4a_fps_t)fps;

    device.start_cameras(&config);

    k4a::calibration calibration = device.get_calibration(config.depth_mode, config.color_resolution);

    if (dump_calibration)
    {
        write_calibration_blob(device.get_raw_calibration(), output_dir, "calibration_blob");
        write_opencv_calib(calibration.depth_camera_calibration, output_dir, "cal_depth_mode" + std::to_string(mode));
        write_opencv_calib(calibration.color_camera_calibration, output_dir, "cal_color");
    }

    if (dump_xytable)
    {
        k4a::image xy_table = k4a::image::create(K4A_IMAGE_FORMAT_CUSTOM,
                                                 calibration.depth_camera_calibration.resolution_width,
                                                 calibration.depth_camera_calibration.resolution_height,
                                                 calibration.depth_camera_calibration.resolution_width *
                                                     (int)sizeof(k4a_float2_t));

        create_xy_table(calibration, xy_table);
        write_xy_table(xy_table, output_dir, "xy_table_mode" + std::to_string(mode));
    }

    k4a::capture capture;

    std::chrono::milliseconds timeout_msec(1000);

    std::cout << "Capturing " << num_caps_per_view << " frames per view \n";
    for (int view_idx = 0; view_idx < num_views; view_idx++)
    {
        std::cout << "Ready to capture next frame . Press Enter? \n";
        // system("pause");
        getchar();
        cv::Mat mean_ir, mean_depth, mean_color[3];
        int n_ir = 0, n_depth = 0, n_color = 0;

        for (int cap_idx = 0; cap_idx < num_caps_per_view; cap_idx++)
        {
            // Get a capture
            if (!device.get_capture(&capture, timeout_msec))
            {
                std::cerr << "Timed out waiting for a capture\n";
                break;
            }
            k4a::image depth_img;
            k4a::image ir_img;
            k4a::image color_img;

            if (dump_color)
            {
                // get color image
                color_img = capture.get_color_image();
                if (color_img.is_valid())
                {
                    std::cout << " | Color res: " << color_img.get_height_pixels() << "x"
                              << color_img.get_width_pixels();
                    cv::Mat frame32[3];
                    cv::Mat bands[3];
                    cv::Mat M = color_to_opencv(color_img);
                    cv::split(M, bands);
                    for (int ch = 0; ch < 3; ch++)
                    {
                        bands[ch].convertTo(frame32[ch], CV_32F);
                        if (n_color < 1)
                            mean_color[ch] = frame32[ch].clone();
                        else
                            mean_color[ch] += frame32[ch].clone();
                    }

                    if (av > 0)
                    {
                        std::string tstr = std::to_string(color_img.get_device_timestamp().count());
                        cv::imwrite(output_dir + "/" + tstr + "-color.png", M);
                    }
                    n_color++;
                }
            }

            if (dump_depth)
            {
                // get depth image
                depth_img = capture.get_depth_image();
                if (depth_img.is_valid())
                {
                    std::cout << " | Depth16 res: " << depth_img.get_height_pixels() << "x"
                              << depth_img.get_width_pixels();

                    cv::Mat frame32;
                    cv::Mat M = depth_to_opencv(depth_img);
                    M.convertTo(frame32, CV_32F);
                    if (n_depth < 1)
                        mean_depth = frame32.clone();
                    else
                        mean_depth += frame32.clone();

                    if (av > 0)
                    {
                        std::string tstr = std::to_string(depth_img.get_device_timestamp().count());
                        cv::imwrite(output_dir + "/" + tstr + "-depth16.png", M);
                    }
                    n_depth++;
                }
            }

            if (dump_ir)
            {
                // get ir image
                ir_img = capture.get_ir_image();
                if (ir_img.is_valid())
                {
                    std::cout << " | Ir16 res: " << ir_img.get_height_pixels() << "x" << ir_img.get_width_pixels();
                    cv::Mat frame32;
                    cv::Mat M = ir_to_opencv(ir_img);
                    M.convertTo(frame32, CV_32F);
                    if (n_ir < 1)
                        mean_ir = frame32.clone();
                    else
                        mean_ir += frame32.clone();

                    if (av > 0)
                    {
                        std::string tstr = std::to_string(ir_img.get_device_timestamp().count());
                        cv::imwrite(output_dir + "/" + tstr + "-ir16.png", M);
                    }
                    n_ir++;
                }
            }
            std::cout << std::endl;
            capture.reset();
        }

        if (n_ir > 0 && av != 1)
        {
            mean_ir /= n_ir;
            cv::Mat ir8, ir16;
            get_gray_gamma_img(mean_ir, ir8, gray_gamma, gray_max, gray_percentile);
            mean_ir.convertTo(ir16, CV_16U);
            cv::imwrite(output_dir + "\\" + "ir8-" + std::to_string(view_idx) + ".png", ir8);
            cv::imwrite(output_dir + "\\" + "ir16-" + std::to_string(view_idx) + ".png", ir16);
        }
        if (n_depth > 0 && av != 1)
        {
            mean_depth /= n_depth;
            cv::Mat depth16;
            mean_depth.convertTo(depth16, CV_16U);
            cv::imwrite(output_dir + "\\" + "depth16-" + std::to_string(view_idx) + ".png", depth16);
        }
        if (n_color > 1 && av != 1)
        {
            cv::Mat channels[3];
            for (int ch = 0; ch < 3; ch++)
            {
                mean_color[ch] /= n_color;
                mean_color[ch].convertTo(channels[ch], CV_8U);
            }
            cv::Mat color8;
            cv::merge(channels, 3, color8);
            cv::imwrite(output_dir + "\\" + "color-" + std::to_string(view_idx) + ".png", color8);
        }
    }

    device.stop_cameras();
    device.close();

    return 0;
}