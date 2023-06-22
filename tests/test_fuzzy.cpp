#include <span>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include <gtest/gtest.h>

#include <json-parser/parser.h>

#include <json-parser-tests/common.h>

using namespace json_parser;

// This is not very good as it slows the fuzzy process way too much and isn't
// interesting. Change this once there is an InputReader strategy as a template
// argument to the json::parser.
static std::string store_input(std::span<const uint8_t> input) {
    static std::size_t counter = 0;
    const std::string out_file = TESTS_DIR_PREFIX"fuzzy/" + std::to_string(counter) + ".json";
    std::ofstream out_ifs{out_file};
    std::ostream_iterator<uint8_t> out_it{out_ifs, " "};
    std::copy(input.begin(), input.end(), out_it);
    ++counter;
    return out_file;
}

static void try_reprinting(std::span<const uint8_t> input) {
    const std::string file = store_input(input);

    try {
        auto [print1, print2] = reprint(file);
        EXPECT_EQ(print1, print2);
    } catch (const parser_exception &pe) {
        // Log failure. We might want to check it later.
        const std::string err_out_file = fs::path{file}.replace_extension(".failed");
        std::ofstream err_out_ifs{err_out_file};
        err_out_ifs << pe.what() << '\n';
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    try_reprinting(std::span{data, size});
    return 0;
}
