// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <string>
#include <iostream> // std::cout
#include <fstream>  // std::ofstream

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco/charuco.hpp>

#include "inc/kahelpers.h"

namespace kahelpers
{

charuco_target::charuco_target()
{
    valid = false;
    squaresX = 0;
    squaresY = 0;
    squareLength_mm = 0.0f;
    markerLength_mm = 0.0f;
    marginSize_mm = 0.0f;
    aruco_dict_name = 0;
}
charuco_target::charuco_target(const std::string template_file)
{
    read_from_json(template_file);
}
// read charuco board from a json file
bool charuco_target::read_from_json(const std::string template_file)
{
    valid = false;
    // std::string template_file = "C:/data/k4a/test_charuco/plane.json";
    cv::FileStorage fs(template_file, cv::FileStorage::READ);

    if (!fs.isOpened())
    {
        std::cerr << "failed to open " << template_file << std::endl;
        return false;
    }

    std::string target_name;
    // second method: use FileNode::operator >>
    fs["target"] >> target_name;

    cv::FileNode shapes = fs["shapes"];

    if (shapes.type() == cv::FileNode::SEQ)
    {
        for (unsigned long i = 0; i < shapes.size(); i++)
        {
            // cv::FileNode val1 = shapes[i]["shape"]; // we only want the content of val1
            // int j = 0;
            // std::cout << val1[j];
            std::string s;
            // second method: use FileNode::operator >>
            shapes[i]["shape"] >> s;
            if (s.compare("charuco") == 0)
            {
                if (!shapes[i]["squares_x"].empty())
                    squaresX = (int)shapes[i]["squares_x"];
                if (!shapes[i]["squares_y"].empty())
                    squaresY = (int)shapes[i]["squares_y"];
                if (!shapes[i]["square_length"].empty())
                    squareLength_mm = (float)shapes[i]["square_length"];
                if (!shapes[i]["marker_length"].empty())
                    markerLength_mm = (float)shapes[i]["marker_length"];
                if (!shapes[i]["margin_size"].empty())
                    marginSize_mm = (float)shapes[i]["margin_size"];
                if (!shapes[i]["aruco_dict_name"].empty())
                    aruco_dict_name = (int)shapes[i]["aruco_dict_name"];
                valid = true;
                break;
            }
        }
    }
    fs.release();
    return valid;
}
cv::Ptr<cv::aruco::CharucoBoard> charuco_target::create_board()
{
    // cv::aruco::PREDEFINED_DICTIONARY_NAME dict_name = (cv::aruco::PREDEFINED_DICTIONARY_NAME)charuco.aruco_dict_name;

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(aruco_dict_name);
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(squaresX,
                                                                             squaresY,
                                                                             squareLength_mm / 1000.0f,
                                                                             markerLength_mm / 1000.0f,
                                                                             dictionary);
    return board;
}

float cal_percentile(const cv::Mat &src, float percentile, float maxRange, int nbins)
{

    int histSize[] = { nbins };
    float hranges[] = { 0, maxRange };
    const float *ranges[] = { hranges };

    cv::Mat hist;
    // we compute the histogram from the 0-th and 1-st channels
    int channels[] = { 0 };
    cv::calcHist(&src,
                 1,
                 channels,
                 cv::Mat(), // do not use mask
                 hist,
                 1,
                 histSize,
                 ranges);

    float sum = 0;
    int np = src.cols * src.rows;
    int binIdx;
    for (binIdx = 0; binIdx < nbins; binIdx++)
    {
        if ((100 * sum / np) >= percentile)
        {
            break;
        }
        sum += hist.at<float>(binIdx);
    }
    float v = binIdx * maxRange / nbins;
    return v;
}

cv::Mat color_to_opencv(const k4a::image &im)
{
    // return (3 channels)
    cv::Mat M3;
    if (im.get_format() == k4a_image_format_t::K4A_IMAGE_FORMAT_COLOR_MJPG)
    {
        // turboJPEG and cv::imdecode produce slightly different results

        std::vector<uint8_t> buffer(im.get_buffer(), im.get_buffer() + im.get_size());
        M3 = cv::imdecode(buffer, cv::IMREAD_ANYCOLOR);
    }
    else if (im.get_format() == k4a_image_format_t::K4A_IMAGE_FORMAT_COLOR_BGRA32)
    {
        cv::Mat M4(im.get_height_pixels(), im.get_width_pixels(), CV_8UC4, (void *)im.get_buffer());
        cv::cvtColor(M4, M3, cv::COLOR_BGRA2BGR);
    }
    else
    {
        std::cerr << "\nThis version supports only COLOR_MJPG and COLOR_BGRA32 format\n";
        M3 = cv::Mat();
    }

    return M3;
}

cv::Mat depth_to_opencv(const k4a::image &im)
{
    return cv::Mat(im.get_height_pixels(),
                   im.get_width_pixels(),
                   CV_16U,
                   (void *)im.get_buffer(),
                   static_cast<size_t>(im.get_stride_bytes()))
        .clone();
}

cv::Mat ir_to_opencv(const k4a::image &im)
{
    return depth_to_opencv(im);
}

k4a::image color_from_opencv(const cv::Mat &M)
{
    cv::Mat M4;
    if (M.type() == CV_8UC4)
        M4 = M;
    else if (M.type() == CV_8UC3)
        cv::cvtColor(M, M4, cv::COLOR_BGR2BGRA);
    else if (M.type() == CV_8UC1)
        cv::cvtColor(M, M4, cv::COLOR_GRAY2BGRA);
    else // (M.type() != CV_8UC4 && M.type() != CV_8UC3 && M.type() != CV_8UC1)
    {
        std::cerr << "Only CV_8UC4, CV_8UC3 and CV_8UC1 are supported \n";
        return NULL;
    }

    k4a::image img = k4a::image::create(K4A_IMAGE_FORMAT_COLOR_BGRA32, M4.cols, M4.rows, static_cast<int>(M4.step));
    memcpy(img.get_buffer(), M4.data, static_cast<size_t>(M4.total() * M4.elemSize()));

    return img;
}

k4a::image depth_from_opencv(const cv::Mat &M)
{

    if (M.type() != CV_16U)
    {
        std::cerr << "Only CV_16U is supported \n";
        return NULL;
    }
    k4a::image img = k4a::image::create(K4A_IMAGE_FORMAT_DEPTH16, M.cols, M.rows, static_cast<int>(M.step));
    memcpy((void *)img.get_buffer(), M.data, static_cast<size_t>(M.total() * M.elemSize()));

    return img;
}

k4a::image ir_from_opencv(const cv::Mat &M)
{

    if (M.type() != CV_16U)
    {
        std::cerr << "Only CV_16U is supported \n";
        return NULL;
    }
    k4a::image img = k4a::image::create(K4A_IMAGE_FORMAT_IR16, M.cols, M.rows, static_cast<int>(M.step));
    memcpy(img.get_buffer(), M.data, static_cast<size_t>(M.total() * M.elemSize()));

    return img;
}

float get_gray_gamma_img(const cv::Mat &inImg, cv::Mat &outImg, float gamma, float maxInputValue, float percentile)
{
    cv::Mat floatMat;
    inImg.convertTo(floatMat, CV_32F);
    // floatMat = abs(floatMat) + 1;
    double minVal, maxVal;
    cv::minMaxLoc(floatMat, &minVal, &maxVal);
    maxInputValue = cv::min(maxInputValue, (float)maxVal);

    float v = cal_percentile(floatMat, percentile, maxInputValue + 1, 1000);

    float scale = 255.0f / static_cast<float>(pow(v, gamma)); // gamma

    floatMat = cv::max(floatMat, 1);
    // cv::log(floatMat, logImg);
    cv::pow(floatMat, gamma, floatMat);
    // 8.8969e-11 -1.9062e-06 0.0191 12.6882

    // cv::Mat(min(scale*floatMat, 255)).convertTo(outImg, CV_8UC1);
    cv::Mat(scale * floatMat).convertTo(outImg, CV_8UC1);
    return scale;
}

bool write_calibration_blob(const std::vector<uchar> calibration_buffer,
                            const std::string output_path,
                            const std::string calib_name)
{
    std::string file_name = output_path + "\\" + calib_name + ".json";
    std::ofstream ofs(file_name, std::ofstream::binary);
    if (!ofs.is_open())
    {
        std::cout << "Error opening file";
        return false;
    }

    ofs.write(reinterpret_cast<const char *>(&calibration_buffer[0]),
              static_cast<std::streamsize>(calibration_buffer.size() - 1));
    ofs.close();

    return true;
}

void calibration_to_opencv(const k4a_calibration_camera_t &camera_calibration,
                           cv::Mat &camera_matrix,
                           cv::Mat &dist_coeffs)
{
    k4a_calibration_intrinsic_parameters_t intrinsics = camera_calibration.intrinsics.parameters;

    std::vector<float> _camera_matrix =
        { intrinsics.param.fx, 0.f, intrinsics.param.cx, 0.f, intrinsics.param.fy, intrinsics.param.cy, 0.f, 0.f, 1.f };
    camera_matrix = cv::Mat(3, 3, CV_32F, &_camera_matrix[0]).clone();
    std::vector<float> _dist_coeffs = { intrinsics.param.k1, intrinsics.param.k2, intrinsics.param.p1,
                                        intrinsics.param.p2, intrinsics.param.k3, intrinsics.param.k4,
                                        intrinsics.param.k5, intrinsics.param.k6 };
    dist_coeffs = cv::Mat(8, 1, CV_32F, &_dist_coeffs[0]).clone();
}

void write_opencv_calib(const k4a_calibration_camera_t &camera_calibration,
                        const std::string output_path,
                        const std::string calib_name)
{
    cv::Mat camera_matrix;
    cv::Mat dist_coeffs;
    calibration_to_opencv(camera_calibration, camera_matrix, dist_coeffs);
    // save opencv cal
    std::string file_name = output_path + "\\" + calib_name + ".yml";
    cv::FileStorage fs(file_name.c_str(), cv::FileStorage::WRITE);
    int s[2] = { camera_calibration.resolution_width, camera_calibration.resolution_height };
    cv::Mat image_size(2, 1, CV_32S, s);

    fs << "K" << camera_matrix;
    fs << "dist" << dist_coeffs;
    fs << "img_size" << image_size;
    fs.release();
}

void get_images(k4a::playback &playback,
                int timestamp,
                cv::Mat &ir16,
                cv::Mat &depth16,
                cv::Mat &color8,
                bool single,
                bool get_ir,
                bool get_depth,
                bool get_color)
{
    playback.seek_timestamp(std::chrono::microseconds(timestamp * 1000), K4A_PLAYBACK_SEEK_BEGIN);

    // std::chrono::microseconds length = playback.get_recording_length();
    // printf("Seeking to timestamp: %d/%d (ms)\n", timestamp, (int)(length.count() / 1000));

    int n_ir = 0, n_depth = 0, n_color = 0;
    cv::Mat mean_ir, mean_depth;
    cv::Mat mean_color[3];

    k4a::capture cap;
    int count = 0;
    while (playback.get_next_capture(&cap))
    {
        std::cout << "Processing frame # " << count++ << std::endl;

        k4a::image depth_img;
        k4a::image ir_img;
        k4a::image color_img;

        if (get_depth)
            depth_img = cap.get_depth_image();
        if (get_ir)
            ir_img = cap.get_ir_image();
        if (get_color)
            color_img = cap.get_color_image();

        if (ir_img.is_valid())
        {
            cv::Mat frame32;
            ir_to_opencv(ir_img).convertTo(frame32, CV_32F);
            if (n_ir < 1)
                mean_ir = frame32.clone();
            else
                mean_ir += frame32.clone();
            n_ir++;
        }
        if (depth_img.is_valid())
        {
            cv::Mat frame32;
            depth_to_opencv(depth_img).convertTo(frame32, CV_32F);
            if (n_depth < 1)
                mean_depth = frame32.clone();
            else
                mean_depth += frame32.clone();
            n_depth++;
        }
        if (color_img.is_valid())
        {
            cv::Mat frame32[3];
            cv::Mat bands[3];
            cv::split(color_to_opencv(color_img), bands);
            for (int ch = 0; ch < 3; ch++)
            {
                bands[ch].convertTo(frame32[ch], CV_32F);
                if (n_color < 1)
                    mean_color[ch] = frame32[ch].clone();
                else
                    mean_color[ch] += frame32[ch].clone();
            }
            n_color++;
        }
        // get single frame
        if (single)
            break;
    }
    if (n_ir > 1)
        mean_ir /= n_ir;
    if (n_depth > 1)
        mean_depth /= n_depth;
    if (n_color > 1)
    {
        for (int ch = 0; ch < 3; ch++)
        {
            mean_color[ch] /= n_color;
        }
    }

    if (!mean_ir.empty())
        mean_ir.convertTo(ir16, CV_16U);
    if (!mean_depth.empty())
        mean_depth.convertTo(depth16, CV_16U);
    if (!mean_color[0].empty())
    {
        cv::Mat channels[3];
        for (int ch = 0; ch < 3; ch++)
        {
            mean_color[ch].convertTo(channels[ch], CV_8U);
        }
        cv::merge(channels, 3, color8);
    }
}

bool interpolate_depth(const cv::Mat &D, float x, float y, float &v)
{
    int xi = (int)x;
    int yi = (int)y;

    if (xi < 1 || xi >= (D.cols - 1) || yi < 1 || yi >= (D.rows - 1))
        return false;

    float a = (float)D.at<unsigned short>(yi, xi);
    float b = (float)D.at<unsigned short>(yi, xi + 1);
    float c = (float)D.at<unsigned short>(yi + 1, xi);
    float d = (float)D.at<unsigned short>(yi + 1, xi + 1);

    float s = x - xi;
    float t = y - yi;

    float v1 = (1.0f - s) * a + s * b;
    float v2 = (1.0f - s) * c + s * d;
    v = (1.0f - t) * v1 + t * v2;

    return true;
}

void create_xy_table(const k4a_calibration_t &calibration, k4a::image &xy_table)
{
    k4a_float2_t *table_data = (k4a_float2_t *)(void *)xy_table.get_buffer();

    int width = calibration.depth_camera_calibration.resolution_width;
    int height = calibration.depth_camera_calibration.resolution_height;

    k4a_float2_t p;
    k4a_float3_t ray;
    int valid;

    for (int y = 0, idx = 0; y < height; y++)
    {
        p.xy.y = (float)y;
        for (int x = 0; x < width; x++, idx++)
        {
            p.xy.x = (float)x;

            k4a_calibration_2d_to_3d(
                &calibration, &p, 1.f, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_DEPTH, &ray, &valid);

            if (valid)
            {
                table_data[idx].xy.x = ray.xyz.x;
                table_data[idx].xy.y = ray.xyz.y;
            }
            else
            {
                table_data[idx].xy.x = nanf("");
                table_data[idx].xy.y = nanf("");
            }
        }
    }
}

void write_xy_table(const k4a::image &xy_table, const std::string output_dir, const std::string table_name)
{
    std::string xfile_name = output_dir + "\\" + table_name + "_x.csv";
    std::string yfile_name = output_dir + "\\" + table_name + "_y.csv";

    k4a_float2_t *xy_table_data = (k4a_float2_t *)(void *)(xy_table.get_buffer());

    int width = xy_table.get_width_pixels();
    int height = xy_table.get_height_pixels();

    std::ofstream ofsx(xfile_name, std::ofstream::out);
    std::ofstream ofsy(yfile_name, std::ofstream::out);

    if (!ofsx.is_open() || !ofsy.is_open())
    {
        std::cout << "Error opening file";
        return;
    }

    // xy_table.
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int idx = y * width + x;
            ofsx << xy_table_data[idx].xy.x;
            ofsy << xy_table_data[idx].xy.y;
            if (x < width - 1)
            {
                ofsx << ",";
                ofsy << ",";
            }
        }
        ofsx << std::endl;
        ofsy << std::endl;
    }

    ofsx.close();
    ofsy.close();
}

bool gen_checkered_pattern(const cv::Mat &A, const cv::Mat &B, cv::Mat &C, int n)
{
    // gen checkered pattern from A & B
    cv::Mat Ma, Mb;
    if (A.depth() != 0 || B.depth() != 0)
    {
        std::cerr << "A.depth()!=0 || B.depth()!=0 \n";
        return false;
    }
    if (A.channels() == B.channels())
    {
        Ma = A;
        Mb = B;
    }
    else if (A.channels() == 1 && B.channels() == 3)
    {
        cv::cvtColor(A, Ma, cv::COLOR_GRAY2BGR);
        Mb = B;
    }
    else if (A.channels() == 3 && B.channels() == 1)
    {
        Ma = A;
        cv::cvtColor(B, Mb, cv::COLOR_GRAY2BGR);
    }
    else
    {
        std::cerr << "number of channels is not supported \n";
        return false;
    }

    if (n <= 0)
    {
        std::cerr << "Invalid value for n, must be greater than 0 \n";
        return false;
    }

    int sx = Ma.cols / n;
    int sy = Ma.rows / n;
    for (int i = 0; i < n; i++)
    {
        for (int j = (i) % 2; j < n; j += 2)
        {
            Mb(cv::Rect(j * sx, i * sy, sx, sy)).copyTo(Ma(cv::Rect(j * sx, i * sy, sx, sy)));
        }
    }
    C = Ma.clone();
    return true;
}

void detect_charuco(const cv::Mat &img,
                    const cv::Ptr<cv::aruco::CharucoBoard> &board,
                    const cv::Ptr<cv::aruco::DetectorParameters> &params,
                    std::vector<int> &markerIds,
                    std::vector<std::vector<cv::Point2f>> &markerCorners,
                    std::vector<int> &charucoIds,
                    std::vector<cv::Point2f> &charucoCorners,
                    bool show_results)
{

    markerIds.clear();
    markerCorners.clear();
    charucoIds.clear();
    charucoCorners.clear();

    cv::aruco::detectMarkers(img, board->dictionary, markerCorners, markerIds, params);
    if (markerIds.size() > 0)
    {
        cv::aruco::interpolateCornersCharuco(markerCorners, markerIds, img, board, charucoCorners, charucoIds);

        if (show_results)
        {
            cv::Mat img_copy, img_copy2;
            if (img.depth() == CV_8U && img.channels() == 1)
                cv::cvtColor(img, img_copy, cv::COLOR_GRAY2BGR);
            else
                img.copyTo(img_copy);
            img_copy.copyTo(img_copy2);
            cv::aruco::drawDetectedMarkers(img_copy, markerCorners, markerIds);

            // if at least one charuco corner detected
            if (charucoIds.size() > 0)
                cv::aruco::drawDetectedCornersCharuco(img_copy2, charucoCorners, charucoIds, cv::Scalar(255, 0, 0));

            if (MAX(img.rows, img.cols) > 1024)
            {
                double sf = 1024. / MAX(img.rows, img.cols);
                cv::resize(img_copy, img_copy, cv::Size(), sf, sf);
                cv::resize(img_copy2, img_copy2, cv::Size(), sf, sf);
            }
            cv::imshow("aruco", img_copy);
            cv::imshow("charuco", img_copy2);
            cv::waitKey(0);
        }
    }
}

void find_common_markers(const std::vector<int> &id1,
                         const std::vector<cv::Point2f> &corners1,
                         const std::vector<int> &id2,
                         const std::vector<cv::Point2f> &corners2,
                         std::vector<int> &cid,
                         std::vector<cv::Point2f> &ccorners1,
                         std::vector<cv::Point2f> &ccorners2)
{
    cid.clear();
    ccorners1.clear();
    ccorners2.clear();

    for (size_t i = 0; i < id1.size(); i++)
    {
        auto it = std::find(id2.begin(), id2.end(), id1[i]);
        if (it != id2.end())
        {
            int j = (int)(it - id2.begin());
            // std::cout << std::endl << v1[i] << "   " << *it << "       " << v2[j] << std::endl;
            cid.push_back(id1[i]);
            ccorners1.push_back(corners1[i]);
            ccorners2.push_back(corners2[j]);
        }
    }
    return;
}

// generate markers 3d positions. Same as getBoardObjectAndImagePoints but for Charuco
void get_board_object_points_charuco(const cv::Ptr<cv::aruco::CharucoBoard> &board,
                                     const std::vector<int> &ids,
                                     std::vector<cv::Point3f> &corners3d)
{
    corners3d.clear();

    for (size_t i = 0; i < ids.size(); i++)
    {
        int id = ids[i];
        corners3d.push_back(board->chessboardCorners[id]);
    }
    return;
}

} // namespace kahelpers