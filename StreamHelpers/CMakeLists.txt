add_library(Cafe.TextUtils.StreamHelpers INTERFACE)

target_include_directories(Cafe.TextUtils.StreamHelpers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(Cafe.TextUtils.StreamHelpers
    INTERFACE Cafe.Io.Streams Cafe.TextUtils.Format)

install(TARGETS Cafe.TextUtils.StreamHelpers
    EXPORT TextUtils.StreamHelpers)

install(DIRECTORY include/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h")

install(EXPORT TextUtils.StreamHelpers
    DESTINATION cmake/TextUtils.StreamHelpers)
