/****************************************************************
                       Copyright (c)
                    Microsoft Corporation
                    All Rights Reserved
               Licensed under the MIT License.
****************************************************************/

// Associated header
//
#include "k4arecordingdockcontrol.h"

// System headers
//
#include <memory>
#include <ratio>
#include <sstream>

// Library headers
//
#include "k4aimgui_all.h"

// Project headers
//
#include "k4aimguiextensions.h"
#include "k4atypeoperators.h"
#include "k4awindowmanager.h"

using namespace k4aviewer;

K4ARecordingDockControl::K4ARecordingDockControl(std::unique_ptr<K4ARecording> &&recording) :
    m_recording(std::move(recording))
{
    m_filenameLabel = m_recording->GetPath().filename().c_str();

    k4a_record_configuration_t recordConfig = m_recording->GetRecordConfiguation();
    std::stringstream fpsSS;
    fpsSS << recordConfig.camera_fps;
    m_fpsLabel = fpsSS.str();

    switch (recordConfig.camera_fps)
    {
    case K4A_FRAMES_PER_SECOND_5:
        m_timePerFrame = std::chrono::microseconds(std::micro::den / (std::micro::num * 5));
        break;

    case K4A_FRAMES_PER_SECOND_15:
        m_timePerFrame = std::chrono::microseconds(std::micro::den / (std::micro::num * 15));
        break;

    case K4A_FRAMES_PER_SECOND_30:
    default:
        m_timePerFrame = std::chrono::microseconds(std::micro::den / (std::micro::num * 30));
        break;
    }

    constexpr char noneStr[] = "(None)";

    // We don't record a depth track if the camera is started in passive IR mode
    //
    m_recordingHasDepth = recordConfig.depth_track_enabled;
    std::stringstream depthSS;
    if (m_recordingHasDepth)
    {
        depthSS << recordConfig.depth_mode;
    }
    else
    {
        depthSS << noneStr;
    }
    m_depthModeLabel = depthSS.str();

    m_recordingHasColor = recordConfig.color_track_enabled;
    std::stringstream colorResolutionSS;
    std::stringstream colorFormatSS;
    if (m_recordingHasColor)
    {
        colorFormatSS << recordConfig.color_format;
        colorResolutionSS << recordConfig.color_resolution;
    }
    else
    {
        colorFormatSS << noneStr;
        colorResolutionSS << noneStr;
    }

    m_colorFormatLabel = colorFormatSS.str();
    m_colorResolutionLabel = colorResolutionSS.str();

    SetViewType(K4AWindowSet::ViewType::Normal);
}

void K4ARecordingDockControl::Show()
{
    ImGui::Text("%s", m_filenameLabel.c_str());

    ImGuiExtensions::ButtonColorChanger cc(ImGuiExtensions::ButtonColor::Red);
    if (ImGui::SmallButton("Close"))
    {
        K4AWindowManager::Instance().ClearWindows();
        K4AWindowManager::Instance().PopDockControl();
        return;
    }
    cc.Clear();
    ImGui::Separator();

    ImGui::Text("FPS:              %s", m_fpsLabel.c_str());
    ImGui::Text("Depth mode:       %s", m_depthModeLabel.c_str());
    ImGui::Text("Color format:     %s", m_colorFormatLabel.c_str());
    ImGui::Text("Color resolution: %s", m_colorResolutionLabel.c_str());

    bool forceReadNext = false;

    if (ImGui::Button("<|"))
    {
        Step(true);
    }
    ImGui::SameLine();

    const int64_t seekMin = 0;
    const int64_t seekMax = static_cast<int64_t>(m_recording->GetRecordingLength());
    if (ImGui::SliderScalar("##seek", ImGuiDataType_S64, &m_currentTimestamp, &seekMin, &seekMax, ""))
    {
        m_recording->SeekTimestamp(static_cast<int64_t>(m_currentTimestamp.count()));
        forceReadNext = true;
    }
    ImGui::SameLine();

    if (ImGui::Button("|>"))
    {
        Step(false);
    }

    if (ImGui::Button("<<"))
    {
        m_recording->SeekTimestamp(0);
        forceReadNext = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(m_paused ? ">" : "||"))
    {
        m_paused = !m_paused;
    }
    ImGui::SameLine();
    if (ImGui::Button(">>"))
    {
        m_recording->SeekTimestamp(static_cast<int64_t>(m_recording->GetRecordingLength() - 1));
        m_paused = true;
        Step(true);
    }

    K4AWindowSet::ShowModeSelector(&m_viewType, true, m_recordingHasDepth, [this](K4AWindowSet::ViewType t) {
        return this->SetViewType(t);
    });

    ReadNext(forceReadNext);
}

void K4ARecordingDockControl::ReadNext(bool force)
{
    if (m_paused && !force)
    {
        return;
    }

    if (m_nextCapture == nullptr)
    {
        k4a::capture nextCapture(m_recording->GetNextCapture());
        if (nextCapture == nullptr)
        {
            // Recording ended
            //
            m_paused = true;
            m_recording->SeekTimestamp(0);
            return;
        }
        else
        {
            m_nextCapture = std::move(nextCapture);
        }
    }

    if (m_nextCapture != nullptr)
    {
        // Only show the next frame if enough time has elapsed since we showed the last one
        //
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::high_resolution_clock::duration timeSinceLastFrame = now - m_lastFrameShownTime;

        if (timeSinceLastFrame >= m_timePerFrame)
        {
            m_currentTimestamp = GetCaptureTimestamp(m_nextCapture);
            m_cameraDataSource.NotifyObservers(m_nextCapture);
            m_nextCapture = m_recording->GetNextCapture();
            m_lastFrameShownTime = now;
        }
    }
}

void K4ARecordingDockControl::Step(bool backward)
{
    m_paused = true;
    m_nextCapture = nullptr;
    k4a::capture capture = backward ? m_recording->GetPreviousCapture() : m_recording->GetNextCapture();
    if (capture)
    {
        m_currentTimestamp = GetCaptureTimestamp(capture);
        m_cameraDataSource.NotifyObservers(capture);
    }
}

std::chrono::microseconds K4ARecordingDockControl::GetCaptureTimestamp(const k4a::capture &capture)
{
    // Captures don't actually have timestamps, images do, so we have to look at all the images
    // associated with the capture.  We only need an approximate timestamp for seeking, so we just
    // return the first one we get back (we don't have to care if a capture has multiple images but
    // the timestamps are slightly off).
    //
    // We check the IR capture instead of the depth capture because if the depth camera is started
    // in passive IR mode, it only has an IR image (i.e. no depth image), but there is no mode
    // where a capture will have a depth image but not an IR image.
    //
    const auto irImage = capture.get_ir_image();
    if (irImage != nullptr)
    {
        return irImage.get_timestamp();
    }

    const auto depthImage = capture.get_depth_image();
    if (depthImage != nullptr)
    {
        return depthImage.get_timestamp();
    }

    const auto colorImage = capture.get_color_image();
    if (colorImage != nullptr)
    {
        return colorImage.get_timestamp();
    }

    return std::chrono::microseconds::zero();
}

void K4ARecordingDockControl::SetViewType(K4AWindowSet::ViewType viewType)
{
    K4AWindowManager::Instance().ClearWindows();
    k4a_record_configuration_t recordConfig = m_recording->GetRecordConfiguation();

    switch (viewType)
    {
    case K4AWindowSet::ViewType::Normal:
        K4AWindowSet::StartNormalWindows(m_filenameLabel.c_str(),
                                         &m_cameraDataSource,
                                         nullptr, // IMU playback not supported yet
                                         nullptr, // Audio source - sound is not supported in recordings
                                         m_recordingHasDepth,
                                         recordConfig.depth_mode,
                                         m_recordingHasColor,
                                         recordConfig.color_format,
                                         recordConfig.color_resolution);
        break;

    case K4AWindowSet::ViewType::PointCloudViewer:
        k4a::calibration calibration;
        const k4a_result_t result = m_recording->GetCalibration(calibration);
        if (result != K4A_RESULT_SUCCEEDED)
        {
            return;
        }

        K4AWindowSet::StartPointCloudWindow(m_filenameLabel.c_str(),
                                            std::move(calibration),
                                            m_cameraDataSource,
                                            m_recording->GetRecordConfiguation().depth_mode);
        break;
    }

    m_viewType = viewType;
}
