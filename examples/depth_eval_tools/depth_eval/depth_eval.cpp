// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <string>
#include <iostream> // std::cout
#include <fstream>  // std::ofstream

#include <k4a/k4a.hpp>
#include <k4arecord/playback.hpp>

#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco/charuco.hpp>

#include "inc/depth_eval.h"
#include "kahelpers.h"

using namespace kahelpers;

void help()
{
    std::cout << "\nDepth Evaluation Tool for K4A.\n";
    std::cout << "\nit uses 2 mkv files:\n";
    std::cout
        << "\t 1st is PASSIVE_IR recorded using: \n\t\t k4arecorder.exe -c 3072p -d PASSIVE_IR -l 3  board1.mkv\n";
    std::cout << "\t 2nd is WFOV_2X2BINNED recorded using: \n\t\t k4arecorder.exe -c 3072p -d WFOV_2X2BINNED -l 3  "
                 "board2.mkv\n";
    std::cout << "\t This version supports WFOV_2X2BINNED but can be easily generalized\n";
    std::cout << "Usage:\n"
                 "./depth_eval  -h or -help or -? print this help message\n"
                 "./depth_eval  -i=<passive_ir mkv file> -d=<depth mkv file> -t=<board json template>"
                 " -out=<output directory>  -s=<1:generate and save result images>\n"
                 " -gg=<gray_gamma used to convert ir data to 8bit gray image. default=0.5>\n"
                 " -gm=<gray_max used to convert ir data to 8bit gray image. default=4000.0>\n"
                 " -gp=<percentile used to convert ir data to 8bit gray image. default=99.0>\n"
                 " Example:\n"
                 "./depth_eval  -i=board1.mkv -d=board2.mkv -t=plane.json -out=c:/data\n";
}

//
// Timestamp in milliseconds. Defaults to 1 sec as the first couple frames don't contain color
static bool process_mkv(const std::string &passive_ir_mkv,
                        const std::string &depth_mkv,
                        const std::string &template_file,
                        int timestamp,
                        const std::string &output_dir,
                        float gray_gamma,
                        float gray_max,
                        float gray_percentile,
                        bool save_images)
{
    //
    // get passive ir
    k4a::playback playback_ir = k4a::playback::open(passive_ir_mkv.c_str());
    if (playback_ir.get_calibration().depth_mode != K4A_DEPTH_MODE_PASSIVE_IR)
    {
        std::cerr << "depth_mode != K4A_DEPTH_MODE_PASSIVE_IR";
        return false;
    }
    cv::Mat passive_ir;
    cv::Mat nullMat;
    get_images(playback_ir, timestamp, passive_ir, nullMat, nullMat, false, true, false, false);
    // generate a 8bit gray scale image from the passive ir so it can be used for marker detection
    cv::Mat passive_ir8;
    get_gray_gamma_img(passive_ir, passive_ir8, gray_gamma, gray_max, gray_percentile);

    //
    // get depth
    k4a::playback playback = k4a::playback::open(depth_mkv.c_str());
    // K4A_DEPTH_MODE_WFOV_2X2BINNED : Depth captured at 512x512
    // this version of the code supports WFOV_2X2BINNED but can be easily generalized, see switch statement below
    k4a_depth_mode_t depth_mode = playback.get_calibration().depth_mode;
    if (depth_mode != K4A_DEPTH_MODE_WFOV_2X2BINNED)
    {
        std::cerr << "depth_mode != K4A_DEPTH_MODE_WFOV_2X2BINNED";
        return false;
    }

    cv::Mat ir16, depth16;
    get_images(playback, timestamp, ir16, depth16, nullMat, false, true, true, false);

    cv::Mat ir8;
    get_gray_gamma_img(ir16, ir8, gray_gamma, gray_max, gray_percentile);

    if (save_images)
    {
        // save images as png files
        if (!ir16.empty())
            cv::imwrite(output_dir + "/ir16.png", ir16);
        if (!depth16.empty())
            cv::imwrite(output_dir + "/depth16.png", depth16);
        if (!passive_ir8.empty())
            cv::imwrite(output_dir + "/color8.png", passive_ir8);
        if (!ir8.empty())
            cv::imwrite(output_dir + "/ir8.png", ir8);
    }

    //
    // create a charuco target from a json template
    charuco_target charuco(template_file);
    cv::Ptr<cv::aruco::CharucoBoard> board = charuco.create_board();
    //
    // detect markers in passive_ir8
    cv::Ptr<cv::aruco::DetectorParameters> params = cv::aruco::DetectorParameters::create();
    // Parameter not in OpenCV 3.2.0 (used by build pipeline)
    // params->cornerRefinementMethod = cv::aruco::CORNER_REFINE_NONE; // best option as my limited testing indicated
    std::vector<int> markerIds_ir;
    std::vector<std::vector<cv::Point2f>> markerCorners_ir;
    std::vector<int> charucoIds_ir;
    std::vector<cv::Point2f> charucoCorners_ir;

    detect_charuco(passive_ir8, board, params, markerIds_ir, markerCorners_ir, charucoIds_ir, charucoCorners_ir, false);

    std::cout << "\n board has " << board->chessboardCorners.size() << " charuco corners";
    std::cout << "\n number of detected corners in ir = " << charucoIds_ir.size();

    //
    // get camera calibration
    k4a::calibration calibration = playback_ir.get_calibration();
    cv::Mat camera_matrix;
    cv::Mat dist_coeffs;
    cv::Mat rvec, tvec;
    calibration_to_opencv(calibration.depth_camera_calibration, camera_matrix, dist_coeffs);
    //
    // estimate pose of the board
    bool converged = cv::aruco::estimatePoseCharucoBoard(
        charucoCorners_ir, charucoIds_ir, board, camera_matrix, dist_coeffs, rvec, tvec);

    //
    // use camera intrinsics and pose to generate ground truth points
    std::vector<cv::Point3f> corners3d;
    std::vector<cv::Point3f> corners3d_cam; // 3d corners in camera coord. = ground truth
    get_board_object_points_charuco(board, charucoIds_ir, corners3d);

    if (converged)
    {
        cv::Mat R;
        cv::Rodrigues(rvec, R);
        for (int i = 0; i < corners3d.size(); i++)
        {
            cv::Mat Pw = (cv::Mat_<double>(3, 1) << corners3d[i].x, corners3d[i].y, corners3d[i].z);

            cv::Mat Pcam = R * Pw + tvec;
            corners3d_cam.push_back(
                cv::Point3f((float)Pcam.at<double>(0, 0), (float)Pcam.at<double>(1, 0), (float)Pcam.at<double>(2, 0)));
        }
    }

    std::vector<float> dz;

    for (int i = 0; i < corners3d_cam.size(); i++)
    {
        float d_mm;
        cv::Point2f corner_depth;
        // modify this to support other depth modes
        switch (depth_mode)
        {
        case K4A_DEPTH_MODE_WFOV_2X2BINNED:
            // for passive ir res (1024) to depth res (512)
            corner_depth.x = charucoCorners_ir[i].x / 2;
            corner_depth.y = charucoCorners_ir[i].y / 2;
            break;
        default:
            corner_depth.x = charucoCorners_ir[i].x / 2;
            corner_depth.y = charucoCorners_ir[i].y / 2;
        }

        if (interpolate_depth(depth16, corner_depth.x, corner_depth.y, d_mm))
        {
            // bias = measured depth - ground truth
            if (d_mm > 0)
                dz.push_back(d_mm - 1000.0f * corners3d_cam[i].z);
        }
    }

    float dz_mean = 0.0;
    float dz_rms = 0.0;
    for (int i = 0; i < dz.size(); i++)
    {
        // std::cout << std::endl << dz[i];
        dz_mean += dz[i];
        dz_rms += dz[i] * dz[i];
    }
    dz_mean /= dz.size();
    dz_rms = sqrt(dz_rms / dz.size());
    std::cout << std::endl << "Mean of Z deph bias = " << dz_mean << " mm";
    std::cout << std::endl << "RMS of Z depth bias = " << dz_rms << " mm";

    return true;
}

int main(int argc, char **argv)
{

    cv::CommandLineParser parser(argc,
                                 argv,
                                 "{help h usage ?| |print this message}"
                                 "{i| | full path of the passive_ir mkv file}"
                                 "{d| | full path of the wfov_binned mkv file}"
                                 "{t| | full path of the board json file e.g., <fullpath>/plane.json}"
                                 "{out| | full path of the output dir}"
                                 "{s|1| generate and save result images}"
                                 "{gg|0.5| gray_gamma used to convert ir data to 8bit gray image}"
                                 "{gm|4000.0| gray_max used to convert ir data to 8bit gray image}"
                                 "{gp|99.0| percentile used to convert ir data to 8bit gray image}");
    // get input argument
    std::string passive_ir_mkv = parser.get<std::string>("i");
    std::string depth_mkv = parser.get<std::string>("d");
    std::string template_file = parser.get<std::string>("t");
    std::string output_dir = parser.get<std::string>("out");

    if (passive_ir_mkv.empty() || depth_mkv.empty() || template_file.empty() || output_dir.empty())
    {
        help();
        return 1;
    }
    bool save_images = parser.get<int>("s") > 0;

    float gray_gamma = parser.get<float>("gg");
    float gray_max = parser.get<float>("gm");
    float gray_percentile = parser.get<float>("gp");

    if (parser.has("help"))
    {
        help();
        return 0;
    }

    // Timestamp in milliseconds. Defaults to 1 sec as the first couple frames don't contain color
    int timestamp = 1000;

    bool stat = process_mkv(passive_ir_mkv,
                            depth_mkv,
                            template_file,
                            timestamp,
                            output_dir,
                            gray_gamma,
                            gray_max,
                            gray_percentile,
                            save_images);

    return (stat) ? 0 : 1;
}
