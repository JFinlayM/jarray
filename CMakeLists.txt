cmake_minimum_required(VERSION 3.10)
project(jarray C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra")

include(GNUInstallDirs)

# ----- Sources de la bibliothèque -----
set(LIB_SOURCES
    src/jarray.c
    src/jarray_string.c
    src/jarray_int.c
    src/jarray_float.c
    src/jarray_char.c
)

# Bibliothèque statique
add_library(jarray STATIC ${LIB_SOURCES})
set_target_properties(jarray PROPERTIES OUTPUT_NAME "jarray")

# Bibliothèque partagée (optionnelle)
add_library(jarray_shared SHARED ${LIB_SOURCES})
set_target_properties(jarray_shared PROPERTIES OUTPUT_NAME "jarray")

# Où trouver les headers lors de la compilation interne
target_include_directories(jarray PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(jarray_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# ----- Installation -----
install(TARGETS jarray jarray_shared
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .a
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # .so
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Installe inc/jarray.h dans /usr/local/include
install(FILES inc/jarray.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ----- Cibles personnalisées -----
# "make lib" -> construit et installe la lib
add_custom_target(lib
    COMMAND ${CMAKE_COMMAND} --build . --target install
    DEPENDS jarray jarray_shared
)

# Exécutable de test
add_executable(jarray_test src/main.c)
target_link_libraries(jarray_test jarray)
target_include_directories(jarray_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# "make test" -> compile seulement le test
add_custom_target(test
    COMMAND ${CMAKE_COMMAND} --build . --target jarray_test
    DEPENDS jarray_test
)
