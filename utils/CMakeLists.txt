project(chromaprint_utils LANGUAGES C CXX)
cmake_minimum_required(VERSION 3.3)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(CMAKE_CXX_STANDARD 11)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

include(CTest)

set(chromaprint_utils_SOURCES
    base64.h
    base64.c
    packed_uint3_array.h
    packed_uint3_array.c
    packed_uint5_array.h
    packed_uint5_array.c
)

set(chromaprint_utils_tests_SOURCES
    base64_test.cc
    packed_uint3_array_test.cc
    packed_uint5_array_test.cc
)

add_library(chromaprint_utils ${chromaprint_utils_SOURCES})

if(BUILD_TESTING)
    find_package(Threads REQUIRED)
    find_package(GTest REQUIRED)
    find_package(GMock REQUIRED)

    add_library(gtest STATIC EXCLUDE_FROM_ALL ${GTEST_SOURCES})
    target_include_directories(gtest INTERFACE ${GTEST_INCLUDE_DIRS})
    target_include_directories(gtest PRIVATE ${GTEST_INCLUDE_DIRS} ${GTEST_SOURCE_DIR})
    target_link_libraries(gtest Threads::Threads)

    add_library(gtest_main STATIC EXCLUDE_FROM_ALL ${GTEST_MAIN_SOURCES})
    target_include_directories(gtest_main INTERFACE ${GTEST_INCLUDE_DIRS})
    target_include_directories(gtest_main PRIVATE ${GTEST_INCLUDE_DIRS} ${GTEST_SOURCE_DIR})
    target_link_libraries(gtest_main gtest)

    add_library(gmock STATIC EXCLUDE_FROM_ALL ${GMOCK_SOURCES})
    target_include_directories(gmock INTERFACE ${GMOCK_INCLUDE_DIRS})
    target_include_directories(gmock PRIVATE ${GMOCK_INCLUDE_DIRS} ${GMOCK_SOURCE_DIR} ${GTEST_SOURCE_DIR})
    target_link_libraries(gmock gtest)

    add_executable(chromaprint_utils_tests ${chromaprint_utils_tests_SOURCES})
    target_link_libraries(chromaprint_utils_tests chromaprint_utils gtest gmock gtest_main)

    add_test(NAME chromaprint_utils_tests
             COMMAND ${CMAKE_CURRENT_BINARY_DIR}/chromaprint_utils_tests)

    add_custom_target(check
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/chromaprint_utils_tests $ENV{GTEST_FLAGS}
        DEPENDS chromaprint_utils_tests)
endif()
