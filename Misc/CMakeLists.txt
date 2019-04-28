add_library(Cafe.TextUtils.Misc INTERFACE)

target_include_directories(Cafe.TextUtils.Misc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(Cafe.TextUtils.Misc INTERFACE
    Cafe.Encoding.Base Cafe.ErrorHandling)

if(TARGET Cafe.Encoding.RuntimeEncoding)
    target_link_libraries(Cafe.TextUtils.Misc INTERFACE
        Cafe.Encoding.RuntimeEncoding)
endif()

install(TARGETS Cafe.TextUtils.Misc
    EXPORT TextUtils.Misc)

install(DIRECTORY include/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h")

install(EXPORT TextUtils.Misc
    NAMESPACE Cafe.
    DESTINATION cmake/TextUtils.Misc)
