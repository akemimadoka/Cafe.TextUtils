add_library(Cafe.TextUtils.Format INTERFACE)

target_include_directories(Cafe.TextUtils.Format INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(Cafe.TextUtils.Format INTERFACE
    Cafe.Encoding.Base Cafe.ErrorHandling Cafe.TextUtils.Misc)

install(TARGETS Cafe.TextUtils.Format
    EXPORT TextUtils.Format)

install(DIRECTORY include/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h")

install(EXPORT TextUtils.Format
    NAMESPACE Cafe.
    DESTINATION cmake/TextUtils.Format)
