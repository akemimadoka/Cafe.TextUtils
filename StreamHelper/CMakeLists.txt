add_library(Cafe.TextUtils.StreamHelper INTERFACE)

target_include_directories(Cafe.TextUtils.StreamHelper INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(Cafe.TextUtils.StreamHelper
    INTERFACE Cafe.Io.Streams Cafe.TextUtils.Format)

install(TARGETS Cafe.TextUtils.StreamHelper
    EXPORT TextUtils.StreamHelper)

install(DIRECTORY include/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h")

install(EXPORT TextUtils.StreamHelper
    DESTINATION cmake/TextUtils.StreamHelper)
