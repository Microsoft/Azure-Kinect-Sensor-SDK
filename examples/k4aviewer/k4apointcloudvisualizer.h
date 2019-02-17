/****************************************************************
                       Copyright (c)
                    Microsoft Corporation
                    All Rights Reserved
               Licensed under the MIT License.
****************************************************************/

#ifndef K4APOINTCLOUDVISUALIZER_H
#define K4APOINTCLOUDVISUALIZER_H

// System headers
//
#include <vector>

// Library headers
//

// Project headers
//
#include "k4acalibrationtransformdata.h"
#include "k4apointcloudrenderer.h"
#include "k4apointcloudtypes.h"
#include "k4apointcloudviewcontrol.h"
#include "k4aviewerutil.h"
#include "opengltexture.h"

namespace k4aviewer
{
class K4APointCloudVisualizer
{
public:
    GLenum InitializeTexture(std::shared_ptr<OpenGlTexture> &texture) const;
    GLenum UpdateTexture(std::shared_ptr<OpenGlTexture> &texture, const k4a::image &frame);

    void ProcessPositionalMovement(ViewMovement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset);
    void ProcessMouseScroll(float yoffset);

    void ResetPosition();

    void EnableShading(bool enable);

    K4APointCloudVisualizer(k4a_depth_mode_t depthMode, std::unique_ptr<K4ACalibrationTransformData> &&calibrationData);
    virtual ~K4APointCloudVisualizer();

    K4APointCloudVisualizer(const K4APointCloudVisualizer &) = delete;
    K4APointCloudVisualizer &operator=(const K4APointCloudVisualizer &) = delete;
    K4APointCloudVisualizer(const K4APointCloudVisualizer &&) = delete;
    K4APointCloudVisualizer &operator=(const K4APointCloudVisualizer &&) = delete;

private:
    void UpdatePointClouds(const k4a::image &frame);

    ExpectedValueRange m_expectedValueRange;
    ImageDimensions m_dimensions;

    PointCloudRenderer m_pointCloudRenderer;
    bool m_pointCloudRendererBufferInitialized = false;
    ViewControl m_viewControl;

    linmath::mat4x4 m_projection{};
    linmath::mat4x4 m_view{};

    GLuint m_frameBuffer = 0;
    GLuint m_depthBuffer = 0;

    std::unique_ptr<K4ACalibrationTransformData> m_calibrationTransformData;

    std::vector<Vertex> m_vertexBuffer;
};
} // namespace k4aviewer

#endif
