cmake_minimum_required(VERSION 3.10)
project(jarray C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -Werror")

include(GNUInstallDirs)

set(LIB_SOURCES
    src/jarray.c
    src/jarray_presets/jarray_string.c
    src/jarray_presets/jarray_int.c
    src/jarray_presets/jarray_float.c
    src/jarray_presets/jarray_char.c
    src/jarray_presets/jarray_short.c
    src/jarray_presets/jarray_long.c
    src/jarray_presets/jarray_double.c
    src/jarray_presets/jarray_uint.c
    src/jarray_presets/jarray_ulong.c
    src/jarray_presets/jarray_ushort.c
)

add_library(jarray STATIC ${LIB_SOURCES})
set_target_properties(jarray PROPERTIES OUTPUT_NAME "jarray")

add_library(jarray_shared SHARED ${LIB_SOURCES})
set_target_properties(jarray_shared PROPERTIES OUTPUT_NAME "jarray")

target_include_directories(jarray PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(jarray_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS jarray jarray_shared
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .a
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .so
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install inc/jarray.h dans /usr/local/include
install(FILES inc/jarray.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_custom_target(lib
    COMMAND ${CMAKE_COMMAND} --build . --target install
    DEPENDS jarray jarray_shared
)

add_executable(jarray_test src/main.c)
target_link_libraries(jarray_test jarray)
target_include_directories(jarray_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(test
    COMMAND ${CMAKE_COMMAND} --build . --target jarray_test
    DEPENDS jarray_test
)
