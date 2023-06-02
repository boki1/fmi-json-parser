cmake_minimum_required(VERSION 3.22)
project(fmi-json-parser)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

option(FMI_JSON_PARSER_BUILD_WITH_MYSTD "Whether to build the project with mystd" "ON")
add_subdirectory(mystd)

option(FMI_JSON_PARSER_BUILD_APP "Whether to build console application." "ON")
if("${FMI_JSON_PARSER_BUILD_APP}" STREQUAL "ON")
    add_subdirectory(app)
endif()

option(FMI_JSON_PARSER_BUILD_TESTS "Whether to build tests." "ON")
if("${FMI_JSON_PARSER_BUILD_TESTS}" STREQUAL "ON")
	include(cmake/conan.cmake)
    add_subdirectory(tests)
endif()
