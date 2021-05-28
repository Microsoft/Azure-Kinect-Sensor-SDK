// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#ifndef TRANSFORMATION_EVAL_H
#define TRANSFORMATION_EVAL_H

#include <string>

#include <k4a/k4a.hpp>
#include <opencv2/core.hpp>

void help();

int calculate_transformation_error(const cv::Mat &depth16,
                                   const cv::Mat &color8,
                                   const std::vector<cv::Point2f> &corners_d,
                                   const std::vector<cv::Point2f> &corners_c,
                                   const k4a::calibration &calibration,
                                   float &rms,
                                   cv::Mat &err_img,
                                   bool gen_err_img = true);

static bool process_mkv(const std::string &passive_ir_mkv,
                        const std::string &depth_mkv,
                        const std::string &template_file,
                        int timestamp,
                        const std::string &output_dir,
                        float gray_gamma,
                        float gray_max,
                        float gray_percentile,
                        bool save_images);

#endif
