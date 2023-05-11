# This should really not be here but in the top-level CMake instead. However
# since I am using gtest "illegally" (no 3rd party dependencies are allowed) I
# will cover myself by putting it here :).
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include(GoogleTest)
enable_testing()

function(add_unit_test name source_file)
    message("-- fmi-json-parser: Add ${name} unit test")
    set(unit_test "test_${name}")
    add_executable(${unit_test} ${TESTS_DIR}/${source_file})
    set_target_properties(${unit_test} PROPERTIES LINKER_LANGUAGE CXX)
    target_link_libraries(${unit_test} PRIVATE CONAN_PKG::gtest json-parser)
	target_include_directories(${unit_test} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
	gtest_discover_tests(${unit_test})
    add_test(NAME ${name} COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bin/${unit_test})
endfunction()

add_unit_test(it_words test_it_works.cpp)
add_unit_test(lexer test_lexer.cpp)
add_unit_test(parser test_parser.cpp)
