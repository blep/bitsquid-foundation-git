cmake_minimum_required(VERSION 3.0)

enable_testing()

project( Foundation CXX )

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH}) 

# Output all libraries and executables in the same directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Single directory for all libraries.")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
    CACHE PATH "Single directory for all executables.")
mark_as_advanced( LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH )

include_directories( include )
add_subdirectory( src )
add_subdirectory( test )
