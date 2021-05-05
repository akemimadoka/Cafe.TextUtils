cmake_minimum_required(VERSION 3.15)

project(Cafe.TextUtils CXX)

set(CAFE_INCLUDE_TEXT_UTILS_MISC ON CACHE BOOL "Include Cafe.TextUtils.Misc")
set(CAFE_INCLUDE_TEXT_UTILS_FORMAT ON CACHE BOOL "Include Cafe.TextUtils.Format")
set(CAFE_INCLUDE_TEXT_UTILS_STREAM_HELPERS ON CACHE BOOL "Include Cafe.TextUtils.StreamHelpers")

list(APPEND CAFE_OPTIONS
    CAFE_INCLUDE_TEXT_UTILS_MISC
    CAFE_INCLUDE_TEXT_UTILS_FORMAT
    CAFE_INCLUDE_TEXT_UTILS_STREAM_HELPERS
)

include(${CMAKE_CURRENT_SOURCE_DIR}/CafeCommon/cmake/CafeCommon.cmake)

if(CAFE_INCLUDE_TEXT_UTILS_MISC)
    add_subdirectory(Misc)
endif()

if(CAFE_INCLUDE_TEXT_UTILS_FORMAT)
    if(NOT CAFE_INCLUDE_TEXT_UTILS_MISC)
        message(SEND_ERROR "Cafe.TextUtils.Format depends on Cafe.TextUtils.Misc, which is not included")
    endif()
    add_subdirectory(Format)
endif()

if(CAFE_INCLUDE_TEXT_UTILS_STREAM_HELPERS)
    add_subdirectory(StreamHelpers)
endif()

if(CAFE_INCLUDE_TESTS)
    add_subdirectory(Test)
endif()
