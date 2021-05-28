# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

find_package(OpenCV REQUIRED)
include_directories(
    .
	./inc/
	../kahelpers/
	../kahelpers/inc/
	)

add_executable(collect
    collect.cpp
	./inc/collect.h
	../kahelpers/kahelpers.cpp
	../kahelpers/inc/kahelpers.h
	)

target_link_libraries(collect PRIVATE
    k4a::k4a
	k4a::k4arecord
    ${OpenCV_LIBS}
	)