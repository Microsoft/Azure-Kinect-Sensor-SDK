// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef K4ACOLORIMAGECONVERTER_H
#define K4ACOLORIMAGECONVERTER_H

// System headers
//
#include <memory>
#include <vector>

// Library headers
//
#include "k4aimgui_all.h"

// Project headers
//
#include "ik4aimageconverter.h"
#include "k4aviewerutil.h"

namespace k4aviewer
{

class K4AColorImageConverterFactory
{
public:
    template<k4a_image_format_t ImageFormat>
    static std::unique_ptr<IK4AImageConverter<ImageFormat>> Create(uint32_t color_mode_id);
};

template<>
std::unique_ptr<IK4AImageConverter<K4A_IMAGE_FORMAT_COLOR_YUY2>>
K4AColorImageConverterFactory::Create<K4A_IMAGE_FORMAT_COLOR_YUY2>(uint32_t color_mode_id);

template<>
std::unique_ptr<IK4AImageConverter<K4A_IMAGE_FORMAT_COLOR_NV12>>
K4AColorImageConverterFactory::Create<K4A_IMAGE_FORMAT_COLOR_NV12>(uint32_t color_mode_id);

template<>
std::unique_ptr<IK4AImageConverter<K4A_IMAGE_FORMAT_COLOR_BGRA32>>
K4AColorImageConverterFactory::Create<K4A_IMAGE_FORMAT_COLOR_BGRA32>(uint32_t color_mode_id);

template<>
std::unique_ptr<IK4AImageConverter<K4A_IMAGE_FORMAT_COLOR_MJPG>>
K4AColorImageConverterFactory::Create<K4A_IMAGE_FORMAT_COLOR_MJPG>(uint32_t color_mode_id);
} // namespace k4aviewer

#endif
