cmake_policy(SET CMP0054 NEW) # Only variable expand once in if statement.

macro(EnableAllCompilationWarning) 
    if(MSVC)
      # Force to always compile with W4
      if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
      endif()
    elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
      # Update if necessary
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
    endif()
endmacro()


# See http://www.cmake.org/cmake/help/v2.8.10/cmake.html#variable:CMAKE_LANG_COMPILER_ID
# for list of all compilers
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
	macro(CompilationWarningAsError)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror ") 
	endmacro()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
	macro(CompilationWarningAsError)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror ") 
	endmacro()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "SunPro")
  # using Oracle Solaris Studio
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	macro(CompilationWarningAsError)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX ") 
	endmacro()
endif()

if ( NOT COMMAND CompilationWarningAsError )
	# warning as error is not supported by the compiler => ignore
	macro(CompilationWarningAsError)
        message(STATUS "For compiler ${CMAKE_CXX_COMPILER}, support for compilation warning as error is missing. Please update CompilationWarning.cmake.")
	endmacro()
endif ( NOT COMMAND CompilationWarningAsError )
