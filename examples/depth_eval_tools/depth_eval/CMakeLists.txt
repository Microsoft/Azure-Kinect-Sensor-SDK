# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

find_package(OpenCV REQUIRED)
include_directories(
    .
	./inc/
	../kahelpers/
	../kahelpers/inc/
	)

add_executable(depth_eval
    depth_eval.cpp
	./inc/depth_eval.h
	../kahelpers/kahelpers.cpp
	../kahelpers/inc/kahelpers.h
	)

target_link_libraries(depth_eval PRIVATE
    k4a::k4a
	k4a::k4arecord
    ${OpenCV_LIBS}
	)