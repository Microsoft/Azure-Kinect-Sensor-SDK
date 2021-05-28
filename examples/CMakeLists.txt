# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_subdirectory(calibration)
add_subdirectory(enumerate)
add_subdirectory(playback_external_sync)
add_subdirectory(k4arecord_custom_track)
add_subdirectory(fastpointcloud)
add_subdirectory(streaming)
add_subdirectory(transformation)
add_subdirectory(undistort)
add_subdirectory(viewer)

if (OpenCV_REQUIRED)
    find_package(OpenCV REQUIRED)
else()
    find_package(OpenCV)
endif ()

if (OpenCV_FOUND)
    add_subdirectory(green_screen)
    add_subdirectory(opencv_compatibility)
	add_subdirectory(depth_eval_tools)

    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        file(COPY ${OpenCV_DIR}/../bin/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} FILES_MATCHING PATTERN "*.dll")
    endif ()
endif ()
