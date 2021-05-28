// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <string>
#include <iostream> // std::cout
#include <fstream>  // std::ofstream

#include <k4a/k4a.hpp>
#include <k4arecord/playback.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco/charuco.hpp>

#include "inc/mkv2images.h"
#include "kahelpers.h"

using namespace kahelpers;

void help()
{
    std::cout << "\nDump mkv as png images.\n";
    std::cout
        << "Usage:\n"
           "./mkv2images  -h or -help or -? print this help message\n"
           "./mkv2images  -in=<input mkv file> -out=<output directory> -d=<dump depth> -i=<dump ir> -c=<dump color>"
           " -f=<0:dump mean images only, 1 : dump first frame>\n"
           " -gg=<gray_gamma used to convert ir data to 8bit gray image. default=0.5>\n"
           " -gm=<gray_max used to convert ir data to 8bit gray image. default=4000.0>\n"
           " -gp=<percentile used to convert ir data to 8bit gray image. default=99.0>\n"
           " Example:\n"
           "./mkv2images  -in=board1.mkv -out=c:/data -c=0 -f=0\n";
}

// extract filename from the full path
std::string extract_filename(const std::string &full_filename)
{
    int index = (int)full_filename.find_last_of("/\\");
    std::string filename = full_filename.substr(index + 1);
    return filename;
}

int main(int argc, char **argv)
{

    cv::CommandLineParser parser(argc,
                                 argv,
                                 "{help h usage ?| |print this message}"
                                 "{in| | full path of the wfov_binned mkv file}"
                                 "{out| | full path of the output dir}"
                                 "{d|1| dump depth}"
                                 "{i|1| dump ir}"
                                 "{c|1| dump color}"
                                 "{f|0| 0:dump mean images only, 1:dump first frame}"
                                 "{gg|0.5| gray_gamma used to convert ir data to 8bit gray image}"
                                 "{gm|4000.0| gray_max used to convert ir data to 8bit gray image}"
                                 "{gp|99.0| percentile used to convert ir data to 8bit gray image}");
    // get input argument

    std::string depth_mkv = parser.get<std::string>("in");
    std::string output_dir = parser.get<std::string>("out");
    bool dump_depth = parser.get<int>("d") > 0;
    bool dump_ir = parser.get<int>("i") > 0;
    bool dump_color = parser.get<int>("c") > 0;
    int f = parser.get<int>("f");
    float gray_gamma = parser.get<float>("gg");
    float gray_max = parser.get<float>("gm");
    float gray_percentile = parser.get<float>("gp");

    if (depth_mkv.empty() || output_dir.empty())
    {
        help();
        return 1;
    }

    if (parser.has("help"))
    {
        help();
        return 0;
    }

    // Timestamp in milliseconds. Defaults to 1 sec as the first couple frames don't contain color
    int timestamp = 1000;

    k4a::playback playback = k4a::playback::open(depth_mkv.c_str());

    cv::Mat ir16, depth16, color8;
    // get all images
    get_images(playback, timestamp, ir16, depth16, color8, f, dump_ir, dump_depth, dump_color);

    cv::Mat ir8;
    if (!ir16.empty())
        get_gray_gamma_img(ir16, ir8, gray_gamma, gray_max, gray_percentile);

    // save images as png files
    std::string filename = extract_filename(depth_mkv);
    if (!ir16.empty())
        cv::imwrite(output_dir + "/" + filename + "-ir16.png", ir16);
    if (!depth16.empty())
        cv::imwrite(output_dir + "/" + filename + "-depth16.png", depth16);
    if (!color8.empty())
        cv::imwrite(output_dir + "/" + filename + "-color8.png", color8);
    if (!ir8.empty())
        cv::imwrite(output_dir + "/" + filename + "-ir8.png", ir8);

    return 0;
}
