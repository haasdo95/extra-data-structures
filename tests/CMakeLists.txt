set(BINARY ${CMAKE_PROJECT_NAME}_tst)

add_executable(${BINARY} main.cpp hash_queue_test.cpp random_dict_test.cpp)
target_compile_definitions(${CMAKE_PROJECT_NAME}_tst PRIVATE ASSERT_ENABLED=1)

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} gtest ${CMAKE_PROJECT_NAME})

set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
