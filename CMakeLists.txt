cmake_minimum_required(VERSION 3.22)
project(fmi-json-parser)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lib)

option(FMI_JSON_PARSER_BUILD_WITH_MYSTD "Whether to build the project with mystd" "ON")
message("-- Building with mystd? ${FMI_JSON_PARSER_BUILD_WITH_MYSTD}")
add_subdirectory(mystd)

option(FMI_JSON_PARSER_BUILD_APP "Whether to build console application." "ON")
message("-- Building app? ${FMI_JSON_PARSER_BUILD_APP}")
if("${FMI_JSON_PARSER_BUILD_APP}" STREQUAL "ON")
    add_subdirectory(app)
endif()

option(FMI_JSON_PARSER_BUILD_TESTS "Whether to build tests." "ON")
message("-- Building tests? ${FMI_JSON_PARSER_BUILD_TESTS}")
if("${FMI_JSON_PARSER_BUILD_TESTS}" STREQUAL "ON")
	include(cmake/conan.cmake)
    add_subdirectory(tests)
endif()
