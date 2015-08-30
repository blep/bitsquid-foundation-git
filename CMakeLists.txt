cmake_minimum_required(VERSION 2.8)

enable_testing()

project( Foundation CXX )

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH}) 

include(EnableLatestCppStandard)
EnableLatestCppStandard()

include(CompilationWarning)
EnableAllCompilationWarning()


include_directories( include )
add_subdirectory( src )
add_subdirectory( test )
