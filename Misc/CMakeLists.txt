set(SOURCE_FILES
    src/Misc.cpp)

set(HEADERS
    include/Cafe/TextUtils/CodePointIterator.h
    include/Cafe/TextUtils/Misc.h)

add_library(Cafe.TextUtils.Misc ${SOURCE_FILES} ${HEADERS})

target_include_directories(Cafe.TextUtils.Misc PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_link_libraries(Cafe.TextUtils.Misc PUBLIC
    Cafe.Encoding.Base Cafe.ErrorHandling)

if(TARGET Cafe.Encoding.RuntimeEncoding)
    target_link_libraries(Cafe.TextUtils.Misc PUBLIC
        Cafe.Encoding.RuntimeEncoding)
endif()

install(TARGETS Cafe.TextUtils.Misc
    EXPORT TextUtils.Misc)

install(DIRECTORY include/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h")

install(EXPORT TextUtils.Misc
    DESTINATION cmake/TextUtils.Misc)
