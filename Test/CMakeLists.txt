set(SOURCE_FILES Main.cpp)

if(CAFE_INCLUDE_TEXT_UTILS_MISC)
    list(APPEND SOURCE_FILES Misc.Test.cpp)
endif()

if(CAFE_INCLUDE_TEXT_UTILS_FORMAT)
    list(APPEND SOURCE_FILES Format.Test.cpp)
endif()

if(CAFE_INCLUDE_TEXT_UTILS_STREAM_HELPERS)
    list(APPEND SOURCE_FILES StreamHelpers.Test.cpp)
endif()

add_executable(Cafe.TextUtils.Test ${SOURCE_FILES})

target_link_libraries(Cafe.TextUtils.Test PRIVATE
    CONAN_PKG::Catch2)

if(CAFE_INCLUDE_TEXT_UTILS_MISC)
    target_link_libraries(Cafe.TextUtils.Test PRIVATE
        Cafe.TextUtils.Misc)
endif()

if(CAFE_INCLUDE_TEXT_UTILS_FORMAT)
    target_link_libraries(Cafe.TextUtils.Test PRIVATE
        Cafe.TextUtils.Format)
endif()

if(CAFE_INCLUDE_TEXT_UTILS_STREAM_HELPERS)
    target_link_libraries(Cafe.TextUtils.Test PRIVATE
        Cafe.TextUtils.StreamHelpers)
endif()
