add_executable(foundation_test
    unit_test.cpp
) 

target_link_libraries(foundation_test foundation_lib)

add_test(foundation_test foundation_test)
