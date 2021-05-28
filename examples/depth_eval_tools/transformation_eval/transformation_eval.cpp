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

#include "inc/transformation_eval.h"
#include "kahelpers.h"

using namespace kahelpers;

void help()
{
    std::cout << "\nTransformation Evaluation Tool for K4A.\n";
    std::cout << "\nit uses 2 mkv files:\n";
    std::cout
        << "\t 1st is PASSIVE_IR recorded using: \n\t\t k4arecorder.exe -c 3072p -d PASSIVE_IR -l 3  board1.mkv\n";
    std::cout << "\t 2nd is WFOV_2X2BINNED recorded using: \n\t\t k4arecorder.exe -c 3072p -d WFOV_2X2BINNED -l 3  "
                 "board2.mkv\n";
    std::cout << "\t This version supports WFOV_2X2BINNED but can be easily generalized\n";
    std::cout << "Usage:\n"
                 "./transformation_eval  -h or -help or -? print this help message\n"
                 "./transformation_eval  -i=<passive_ir mkv file> -d=<depth mkv file> -t=<board json template>"
                 " -out=<output directory>  -s=<1:generate and save result images>\n"
                 " -gg=<gray_gamma used to convert ir data to 8bit gray image. default=0.5>\n"
                 " -gm=<gray_max used to convert ir data to 8bit gray image. default=4000.0>\n"
                 " -gp=<percentile used to convert ir data to 8bit gray image. default=99.0>\n"
                 " Example:\n"
                 "./transformation_eval  -i=board1.mkv -d=board2.mkv -t=plane.json -out=c:/data\n";
}

// map markers from depth image to color image and calculate the reprojection error rms
int calculate_transformation_error(const cv::Mat &depth16,
                                   const cv::Mat &color8,
                                   const std::vector<cv::Point2f> &corners_d,
                                   const std::vector<cv::Point2f> &corners_c,
                                   const k4a::calibration &calibration,
                                   float &rms,
                                   cv::Mat &err_img,
                                   bool gen_err_img)
{
    rms = 0;
    int nValid = 0;

    cv::Mat xpc_predict((int)corners_c.size(), 2, CV_32F, -1.0);
    for (int i = 0; i < corners_d.size(); i++)
    {
        float d_mm;
        interpolate_depth(depth16, corners_d[i].x, corners_d[i].y, d_mm);
        if (d_mm <= 0)
            continue;

        // 1. given detections in depth and color corners_d & corners_c
        k4a_float2_t pd, pc;
        pd.v[0] = corners_d[i].x;
        pd.v[1] = corners_d[i].y;

        // 2. map pixel from depth image to color image
        bool valid =
            calibration.convert_2d_to_2d(pd, d_mm, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_COLOR, &pc);
        // 3. if valid, calculate the reprojection error between:
        //		a) prediction: transformed markers from depth to color using intrinsics of both cameras and R&T between
        // them
        //      b) detection: detected markers in color
        // err = prediction - detection
        if (valid)
        {
            xpc_predict.at<float>(i, 0) = pc.v[0];
            xpc_predict.at<float>(i, 1) = pc.v[1];
            float dx = pc.v[0] - corners_c[i].x;
            float dy = pc.v[1] - corners_c[i].y;
            rms += dx * dx + dy * dy;
            nValid++;
        }
    }

    if (nValid > 0)
        rms = sqrt(rms / nValid);

    // show the error on the color image
    if (nValid > 0 && gen_err_img)
    {
        color8.copyTo(err_img);
        for (int i = 0; i < xpc_predict.rows; i++)
        {
            if (xpc_predict.at<float>(i, 0) >= 0) // valid
            {
                cv::Point2f p1(corners_c[i].x, corners_c[i].y);
                cv::Point2f p2(xpc_predict.at<float>(i, 0), xpc_predict.at<float>(i, 1));

                cv::line(err_img, p1, p2, cv::Scalar(0, 0, 255), 1);
                cv::drawMarker(err_img, p1, cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 2);
                cv::drawMarker(err_img, p2, cv::Scalar(255, 0, 0), cv::MARKER_CROSS, 2);
            }
        }
    }

    return nValid;
}

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
    // get depth & color
    k4a::playback playback = k4a::playback::open(depth_mkv.c_str());
    // K4A_DEPTH_MODE_WFOV_2X2BINNED : Depth captured at 512x512
    // this version of the code supports WFOV_2X2BINNED but can be easily generalized, see switch statement below
    k4a_depth_mode_t depth_mode = playback.get_calibration().depth_mode;
    if (depth_mode != K4A_DEPTH_MODE_WFOV_2X2BINNED)
    {
        std::cerr << "depth_mode != K4A_DEPTH_MODE_WFOV_2X2BINNED";
        return false;
    }

    cv::Mat ir16, depth16, color8;
    // getting mean depth, ir and color (single=false)
    get_images(playback, timestamp, ir16, depth16, color8, false, true, true, true);
    // generate a 8bit gray scale image from ir
    cv::Mat ir8;
    get_gray_gamma_img(ir16, ir8, gray_gamma, gray_max, gray_percentile);

    //
    // create a charuco target from a json template
    charuco_target charuco(template_file);
    cv::Ptr<cv::aruco::CharucoBoard> board = charuco.create_board();

    //
    // detect markers in passive_ir8
    cv::Ptr<cv::aruco::DetectorParameters> params = cv::aruco::DetectorParameters::create();
    params->cornerRefinementMethod = cv::aruco::CORNER_REFINE_NONE; // best option as my limited testing indicated

    std::vector<int> markerIds_ir;
    std::vector<std::vector<cv::Point2f>> markerCorners_ir;
    std::vector<int> charucoIds_ir;
    std::vector<cv::Point2f> charucoCorners_ir;

    detect_charuco(passive_ir8, board, params, markerIds_ir, markerCorners_ir, charucoIds_ir, charucoCorners_ir, false);
    //
    // detect markers in color
    std::vector<int> markerIds_color;
    std::vector<std::vector<cv::Point2f>> markerCorners_color;
    std::vector<int> charucoIds_color;
    std::vector<cv::Point2f> charucoCorners_color;

    detect_charuco(
        color8, board, params, markerIds_color, markerCorners_color, charucoIds_color, charucoCorners_color, false);

    std::vector<int> common_id;
    std::vector<cv::Point2f> common_corners_ir;
    std::vector<cv::Point2f> common_corners_color;

    // find common markers between ir and color sets
    find_common_markers(charucoIds_ir,
                        charucoCorners_ir,
                        charucoIds_color,
                        charucoCorners_color,
                        common_id,
                        common_corners_ir,
                        common_corners_color);

    std::cout << "\n board has " << board->chessboardCorners.size() << " charuco corners";
    std::cout << "\n corners detected in ir = " << charucoIds_ir.size();
    std::cout << "\n corners detected in color = " << charucoIds_color.size();
    std::cout << "\n number of common corners = " << common_id.size();

    // convert opencv mat back to k4a images
    k4a::image ir_img, depth_img, color_img;
    if (!ir16.empty())
        ir_img = ir_from_opencv(ir16);
    if (!depth16.empty())
        depth_img = depth_from_opencv(depth16);
    if (!color8.empty())
        color_img = color_from_opencv(color8);

    k4a::calibration calibration = playback.get_calibration();
    k4a::transformation transformation(calibration);

    if (save_images)
    {
        // transform color image into depth camera geometry
        k4a::image transformed_color = transformation.color_image_to_depth_camera(depth_img, color_img);
        cv::Mat color8t = color_to_opencv(transformed_color);
        cv::Mat mixM;
        if (gen_checkered_pattern(ir8, color8t, mixM, 17))
            cv::imwrite(output_dir + "/" + "checkered_pattern.png", mixM);
    }

    std::vector<cv::Point2f> common_corners_d;
    // 1024 --> 512
    common_corners_d = common_corners_ir;
    for (size_t i = 0; i < common_corners_d.size(); i++)
    {
        // modify this to support other depth modes
        switch (depth_mode)
        {
        case K4A_DEPTH_MODE_WFOV_2X2BINNED:
            // for passive ir res (1024) to depth res (512)
            common_corners_d[i].x /= 2;
            common_corners_d[i].y /= 2;
            break;
        default:
            common_corners_d[i].x /= 2;
            common_corners_d[i].y /= 2;
        }
    }

    float rms = 1e6;
    cv::Mat err_img;
    int n_valid = calculate_transformation_error(
        depth16, color8, common_corners_d, common_corners_color, calibration, rms, err_img, save_images);

    if (n_valid > 0)
    {
        std::cout << std::endl << " rms = " << rms << " pixels" << std::endl;
        if (save_images)
            cv::imwrite(output_dir + "/" + "transformation_error.png", err_img);
    }

    std::string file_name = output_dir + "/" + "results.txt";
    std::ofstream ofs(file_name);
    if (!ofs.is_open())
        std::cerr << "Error opening " << file_name;
    else
    {
        ofs << " rms = " << rms << " pixels" << std::endl;
        ofs.close();
    }

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
