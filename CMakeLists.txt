cmake_minimum_required(VERSION 3.20)
project(extra_data_structure)

set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} INTERFACE)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(src)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(GRAPH_LITE_BUILD_TESTS "whether or not tests should be built" ON)
    if (GRAPH_LITE_BUILD_TESTS)
        message("building tests")
        include(CTest)
        enable_testing()
        add_subdirectory(tests)
        add_subdirectory(lib/googletest)
    endif()
endif()
