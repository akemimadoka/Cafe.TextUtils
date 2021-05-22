add_library(Cafe.TextUtils.Misc INTERFACE)

target_include_directories(Cafe.TextUtils.Misc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(Cafe.TextUtils.Misc INTERFACE
    CONAN_PKG::Cafe.Encoding
    CONAN_PKG::Cafe.ErrorHandling
    CONAN_PKG::Cafe.Environment
)

target_compile_features(Cafe.TextUtils.Misc INTERFACE cxx_std_20)

AddCafeSharedFlags(Cafe.TextUtils.Misc)

install(TARGETS Cafe.TextUtils.Misc
    EXPORT TextUtils.Misc
)

install(DIRECTORY src/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT TextUtils.Misc
    DESTINATION cmake/TextUtils.Misc
)
