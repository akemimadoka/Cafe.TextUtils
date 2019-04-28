set(CAFE_INCLUDE_TEXT_UTILS_MISC ON CACHE BOOL "Include Cafe.TextUtils.Misc")
set(CAFE_INCLUDE_TEXT_UTILS_FORMAT ON CACHE BOOL "Include Cafe.TextUtils.Format")
set(CAFE_INCLUDE_TEXT_UTILS_TEST ON CACHE BOOL "Include Cafe.TextUtils.Test")

if(CAFE_INCLUDE_TEXT_UTILS_MISC)
    add_subdirectory(Misc)
endif()

if(CAFE_INCLUDE_TEXT_UTILS_FORMAT)
    if(NOT CAFE_INCLUDE_TEXT_UTILS_MISC)
        message(SEND_ERROR "Cafe.TextUtils.Format depends on Cafe.TextUtils.Misc, which is not included")
    endif()
    add_subdirectory(Format)
endif()

if(CAFE_INCLUDE_TEXT_UTILS_TEST)
    add_subdirectory(Test)
endif()
