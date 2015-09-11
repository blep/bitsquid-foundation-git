set(INC_DIR ../include/foundation)
set( PUBLIC_HEADERS 
    ${INC_DIR}/array.h
    ${INC_DIR}/assert.h
    ${INC_DIR}/cast.h
    ${INC_DIR}/collection_types.h
    ${INC_DIR}/hash.h
    ${INC_DIR}/math_types.h
    ${INC_DIR}/memory.h
    ${INC_DIR}/memory_types.h
    ${INC_DIR}/murmur_hash.h
    ${INC_DIR}/queue.h
    ${INC_DIR}/snprintf_msvc.h
    ${INC_DIR}/sorted_range.h
    ${INC_DIR}/string_stream.h
    ${INC_DIR}/temp_allocator.h
    ${INC_DIR}/types.h
)

source_group( "api" FILES ${PUBLIC_HEADERS} ) 
SET( SOURCES 
    assert.cpp
    memory.cpp
    murmur_hash.cpp
    string_stream.cpp
    windows_lean.h
   )

ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

add_library( foundation_lib ${SOURCES} ${PUBLIC_HEADERS} )

set_target_properties(foundation_lib PROPERTIES OUTPUT_NAME foundation_lib )