# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

# string(JOIN ...) was added in CMake 3.11 and thus can not be used.
# string_join was written to mimic string(JOIN ...) functionality and
# interface.
function(string_join GLUE OUTPUT VALUES)
    set(_TEMP_VALUES ${VALUES} ${ARGN})
    string(REPLACE ";" "${GLUE}" _TEMP_STR "${_TEMP_VALUES}")
    set(${OUTPUT} "${_TEMP_STR}" PARENT_SCOPE)
endfunction()

# Set the default version string if it wasn't defined in the build configuration
if (NOT DEFINED VERSION_STR)
    if (NOT PROJECT_VERSION_PATCH)
        set(PROJECT_VERSION_PATCH 0)
    endif()
    set(VERSION_STR "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-private")
endif()

set(SEMVER_REGEX "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(\\-([a-zA-Z0-9\\._]+))?(\\+([a-zA-Z0-9\\._]+))?$")
string(REGEX MATCH ${SEMVER_REGEX} VERSION_MATCH ${VERSION_STR})

if (NOT VERSION_MATCH)
    message(FATAL_ERROR "Contents of VERSION_STR ('${VERSION_STR}') are not a valid version")
endif()

if (NOT ("${CMAKE_MATCH_1}" STREQUAL "${PROJECT_VERSION_MAJOR}"))
    # You can update cmake project version in the project() command in the root CMakeLists.txt
    message(FATAL "CMake Project Major Version \"${PROJECT_VERSION_MAJOR}\" did not match VERSION_STR major version \"${CMAKE_MATCH_1}\"")
endif()

if (NOT ("${CMAKE_MATCH_2}" STREQUAL "${PROJECT_VERSION_MINOR}"))
    # You can update cmake project version in the project() command in the root CMakeLists.txt
    message(FATAL "CMake Project Minor Version \"${PROJECT_VERSION_MINOR}\" did not match VERSION_STR minor version \"${CMAKE_MATCH_2}\"")
endif()

set(K4A_VERSION_MAJOR ${CMAKE_MATCH_1})
set(K4A_VERSION_MINOR ${CMAKE_MATCH_2})
set(K4A_VERSION_PATCH ${CMAKE_MATCH_3})
set(K4A_VERSION_PRERELEASE ${CMAKE_MATCH_5})
set(K4A_VERSION_BUILD_METADATA ${CMAKE_MATCH_7})
set(K4A_VERSION_STR ${VERSION_STR})

if (NOT K4A_VERSION_REVISION)
    set(K4A_VERSION_REVISION "0")
endif()


set(K4A_COMPANYNAME "Microsoft")
set(K4A_PRODUCTNAME "Azure Kinect")
set(K4A_COPYRIGHT   "Copyright Microsoft Corporation. All rights reserved.")

set(
    K4A_VERSION
    "${K4A_VERSION_MAJOR}.${K4A_VERSION_MINOR}.${K4A_VERSION_PATCH}"
)
