// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#ifndef KAHELPERS_H
#define KAHELPERS_H

#include <k4a/k4a.hpp>
#include <k4arecord/playback.hpp>

#include "opencv2/core.hpp"
#include <opencv2/aruco/charuco.hpp>

namespace kahelpers
{

// to store charuco target size, number of squares, ..., etc.
struct charuco_target
{
    int squaresX;          // number of chessboard squares in X direction
    int squaresY;          // number of chessboard squares in Y direction
    float squareLength_mm; // chessboard square side length in mm
    float markerLength_mm; //  marker side length in mm
    float marginSize_mm;   // white margin around the board in mm
    int aruco_dict_name;   // Predefined markers dictionaries
    bool valid;
    charuco_target();
    // create a charuco target from a json template
    charuco_target(const std::string template_file);
    // read charuco board from a json template
    bool read_from_json(const std::string template_file);
    cv::Ptr<cv::aruco::CharucoBoard> create_board();
};

// convert k4a color image to opencv mat
cv::Mat color_to_opencv(const k4a::image &im);
// convert k4a depth image to opencv mat
cv::Mat depth_to_opencv(const k4a::image &im);
// convert k4a ir image to opencv mat
cv::Mat ir_to_opencv(const k4a::image &im);
// convert opencv mat to k4a color image
k4a::image color_from_opencv(const cv::Mat &M);
// convert opencv mat to k4a depth image
k4a::image depth_from_opencv(const cv::Mat &M);
// convert opencv mat to k4a ir image
k4a::image ir_from_opencv(const cv::Mat &M);

// calculate percentile
float cal_percentile(const cv::Mat &src, float percentile, float maxRange, int nbins = 100);
// get 8-bit gray scale image from 16-bit image.
float get_gray_gamma_img(const cv::Mat &inImg,
                         cv::Mat &outImg,
                         float gamma = 0.5f,
                         float maxInputValue = 5000.0,
                         float percentile = 99.0);

// write calibration blob to a json file
bool write_calibration_blob(const std::vector<uchar> calibration_buffer,
                            const std::string output_path,
                            const std::string calib_name);
// convert k4a calibration to opencv format i.e., camera matrix, dist coeffs
void calibration_to_opencv(const k4a_calibration_camera_t &camera_calibration,
                           cv::Mat &camera_matrix,
                           cv::Mat &dist_coeffs);
// convert k4a calibration to opencv format i.e., camera matrix, dist coeffs
// and write results to a yml file
void write_opencv_calib(const k4a_calibration_camera_t &camera_calibration,
                        const std::string output_path,
                        const std::string calib_name);

// get ir, depth and color images from a playback
// this can be used to get single image or the mean of all images in the playback
void get_images(k4a::playback &playback,
                int timestamp,
                cv::Mat &ir16,
                cv::Mat &depth16,
                cv::Mat &color8,
                bool single = false,
                bool get_ir = true,
                bool get_depth = true,
                bool get_color = true);

// interpolate depth value using bilinear interpolation
bool interpolate_depth(const cv::Mat &D, float x, float y, float &v);

// create xy table that stores the normalized coordinates (xn, yn)
void create_xy_table(const k4a_calibration_t &calibration, k4a::image &xy_table);
// write the xy table to two csv files "_x.csv" and "_y.csv"
void write_xy_table(const k4a::image &xy_table, const std::string output_dir, const std::string table_name);

// generate a checkered pattern from two imput images
bool gen_checkered_pattern(const cv::Mat &A, const cv::Mat &B, cv::Mat &C, int n = 11);

// detect charuco markers and corners
void detect_charuco(const cv::Mat &img,
                    const cv::Ptr<cv::aruco::CharucoBoard> &board,
                    const cv::Ptr<cv::aruco::DetectorParameters> &params,
                    std::vector<int> &markerIds,
                    std::vector<std::vector<cv::Point2f>> &markerCorners,
                    std::vector<int> &charucoIds,
                    std::vector<cv::Point2f> &charucoCorners,
                    bool show_results = false);

// find common markers between two sets
void find_common_markers(const std::vector<int> &id1,
                         const std::vector<cv::Point2f> &corners1,
                         const std::vector<int> &id2,
                         const std::vector<cv::Point2f> &corners2,
                         std::vector<int> &cid,
                         std::vector<cv::Point2f> &ccorners1,
                         std::vector<cv::Point2f> &ccorners2);

// generate markers 3d positions. Same as getBoardObjectAndImagePoints but for Charuco
void get_board_object_points_charuco(const cv::Ptr<cv::aruco::CharucoBoard> &board,
                                     const std::vector<int> &ids,
                                     std::vector<cv::Point3f> &corners3d);
} // namespace kahelpers
#endif