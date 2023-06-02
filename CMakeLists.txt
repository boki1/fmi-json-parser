cmake_minimum_required(VERSION 3.22)
project(fmi-json-parser)

add_subdirectory(lib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

option(FMI_JSON_PARSER_BUILD_APP "Whether to build console application." "yes")
if("${FMI_JSON_PARSER_BUILD_APP}" STREQUAL "yes")
    add_subdirectory(app)
endif()

option(FMI_JSON_PARSER_BUILD_TESTS "Whether to build tests." "no")
#if("${FMI_JSON_PARSER_BUILD_TESTS}" STREQUAL "yes")
message("in tests")
    add_subdirectory(tests)
#endif()
