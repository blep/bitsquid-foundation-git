cmake_minimum_required(VERSION 2.8)

enable_testing()

project( Foundation CXX )

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH}) 
option(COVERALLS "Generate coveralls data" OFF)

# Coverall service integration (https://github.com/JoakimSoderberg/coveralls-cmake)
if (COVERALLS AND (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/coveralls-cmake/cmake)
    include(Coveralls)
	coveralls_turn_on_coverage() # update compile flags for coverage
    
    file(GLOB_RECURSE COVER_SOURCES include/*.h include/*.hpp src/*.cpp src/*.c src/*.h src/*.hpp test/*.cpp test/*.c test/*.h test/*.hpp)

    # Setup the coveralls target and tell it to gather
    # coverage data for all the lib sources.
	coveralls_setup(
        "${COVER_SOURCES}" 
        ${COVERALLS_UPLOAD}                 
        "${CMAKE_CURRENT_SOURCE_DIR}/coveralls-cmake/cmake")
else()
    add_custom_target(coveralls)
endif ()

include(EnableLatestCppStandard)
EnableLatestCppStandard()

include(CompilationWarning)
EnableAllCompilationWarning()
CompilationWarningAsError()

include_directories( include )
add_subdirectory( src )
add_subdirectory( test )
