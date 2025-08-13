cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(Array C)

# Standards et warnings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

# Fichiers sources
set(SOURCES
    main.c
    my_array.c
)

# Créer l'exécutable
add_executable(my_array ${SOURCES})

# Inclure le dossier courant pour les .h
target_include_directories(my_array PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
