add_library(Cafe.TextUtils.Format INTERFACE)

target_include_directories(Cafe.TextUtils.Format INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(Cafe.TextUtils.Format INTERFACE
    CONAN_PKG::Cafe.Encoding
    CONAN_PKG::Cafe.ErrorHandling
    Cafe.TextUtils.Misc
)

target_compile_features(Cafe.TextUtils.Format INTERFACE cxx_std_20)

AddCafeSharedFlags(Cafe.TextUtils.Format)

install(TARGETS Cafe.TextUtils.Format
    EXPORT TextUtils.Format
)

install(DIRECTORY src/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT TextUtils.Format
    DESTINATION cmake/TextUtils.Format
)
