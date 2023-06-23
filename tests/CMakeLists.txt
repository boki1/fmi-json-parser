enable_testing()

find_package(GTest)
include(GoogleTest)

add_library(json-parser-tests INTERFACE)
target_include_directories(json-parser-tests INTERFACE
		include/
		../lib/include/
		../mystd/include/)
target_link_libraries(json-parser-tests INTERFACE
		mystd
		json-parser)

function(add_unit_test name file_path)
    message("-- fmi-json-parser: Add ${name} unit test")
    set(target "test_${name}")
    add_executable(${target} ${file_path})
    set_target_properties(${target} PROPERTIES
        LINKER_LANGUAGE CXX)
    target_link_libraries(${target} PRIVATE
        GTest::gtest_main json-parser-tests)
    target_include_directories(${target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/include)
    gtest_discover_tests(${target})
    add_test(NAME ${target}
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests/${target})
endfunction()

add_unit_test(it_works test_it_works.cpp)
add_unit_test(tokenizer test_tokenizer.cpp)
add_unit_test(parser test_parser.cpp)
add_unit_test(reprint test_reprint.cpp)

add_unit_test(memory mystd/test_memory.cpp)
