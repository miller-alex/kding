cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(kding)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

add_subdirectory( src )
add_subdirectory( resources )
add_subdirectory( doc )
add_subdirectory( po )
