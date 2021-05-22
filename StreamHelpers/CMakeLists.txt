add_library(Cafe.TextUtils.StreamHelpers INTERFACE)

target_include_directories(Cafe.TextUtils.StreamHelpers INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(Cafe.TextUtils.StreamHelpers
    INTERFACE CONAN_PKG::Cafe.Io Cafe.TextUtils.Format
)

target_compile_features(Cafe.TextUtils.StreamHelpers INTERFACE cxx_std_20)

AddCafeSharedFlags(Cafe.TextUtils.StreamHelpers)

install(TARGETS Cafe.TextUtils.StreamHelpers
    EXPORT TextUtils.StreamHelpers
)

install(DIRECTORY src/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT TextUtils.StreamHelpers
    DESTINATION cmake/TextUtils.StreamHelpers
)
