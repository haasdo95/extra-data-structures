set(BINARY ${CMAKE_PROJECT_NAME}_tst)

add_executable(${BINARY} hash_queue_test.cpp random_dict_test.cpp)

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} gtest ${CMAKE_PROJECT_NAME})

set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
