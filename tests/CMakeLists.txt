enable_testing()

find_package(GTest)
include(GoogleTest)

function(add_unit_test name)
    message("-- fmi-json-parser: Add ${name} unit test")
    set(file "test_${name}")
    add_executable(${file} ${file}.cpp)
    set_target_properties(${file} PROPERTIES
			LINKER_LANGUAGE CXX)
    target_link_libraries(${file} PRIVATE
			GTest::gtest_main json-parser)
	target_include_directories(${file} PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
	gtest_discover_tests(${file})
    add_test(NAME ${file}
			COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bin/${file})
endfunction()

add_unit_test(it_works test_it_works.cpp)
add_unit_test(lexer test_lexer.cpp)
add_unit_test(parser test_parser.cpp)
