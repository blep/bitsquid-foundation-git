cmake_minimum_required(VERSION 2.8)

enable_testing()

project( Foundation CXX )

# Output all libraries and executables in the same directory
#set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
#    CACHE PATH "Single directory for all libraries.")
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
#    CACHE PATH "Single directory for all executables.")
#mark_as_advanced( LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH )

include_directories( include )
add_subdirectory( src )
add_subdirectory( test )
