cmake_minimum_required(VERSION 2.8)

enable_testing()

project( Foundation CXX )

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH}) 
option(COVERALLS "Generate coveralls data" OFF)

if (COVERALLS) # Coverall service integration (https://github.com/JoakimSoderberg/coveralls-cmake)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/coveralls-cmake/cmake)
    include(Coveralls)
	coveralls_turn_on_coverage() # update compile flags for coverage
    
    file(GLOB_RECURSE COVER_SOURCES include/*.h include/*.hpp src/*.cpp src/*.c src/*.h src/*.hpp test/*.cpp test/*.c test/*.h test/*.hpp)

# Setup the coveralls target and tell it to gather
# coverage data for all the lib sources.
	coveralls_setup(
        "${LIB_SRC}" 
        ${COVERALLS_UPLOAD}                 
        "${CMAKE_CURRENT_SOURCE_DIR}/coveralls-cmake/cmake")
endif (COVERALLS)

include(EnableLatestCppStandard)
EnableLatestCppStandard()

include(CompilationWarning)
EnableAllCompilationWarning()
CompilationWarningAsError()

include_directories( include )
add_subdirectory( src )
add_subdirectory( test )
