add_library(Cafe.TextUtils.Misc INTERFACE)

target_include_directories(Cafe.TextUtils.Misc INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(Cafe.TextUtils.Misc INTERFACE
    Cafe.Encoding.Base Cafe.ErrorHandling)

if(TARGET Cafe.Encoding.RuntimeEncoding)
    target_link_libraries(Cafe.TextUtils.Misc INTERFACE
        Cafe.Encoding.RuntimeEncoding Cafe.Environment)
endif()

install(TARGETS Cafe.TextUtils.Misc
    EXPORT TextUtils.Misc)

install(DIRECTORY src/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h")

install(EXPORT TextUtils.Misc
    DESTINATION cmake/TextUtils.Misc)
