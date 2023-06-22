#ifndef FMI_JSON_PARSER_TESTS_COMMON_INCLUDED
#define FMI_JSON_PARSER_TESTS_COMMON_INCLUDED

#include <string>
#include <fstream>
#include <utility>
#include <filesystem>
namespace fs = std::filesystem;

#include <json-parser/parser.h>

#ifndef TESTS_DIR_PREFIX
#define TESTS_DIR_PREFIX "tests/"
#endif

std::string slurp(const std::string &filename) {
    // Seems not to be most efficient implementation but seems elegant to me.
    // Source: https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c#116220

    std::ifstream in{filename};
    return std::string{std::istreambuf_iterator<char>{in}, {}};
}

std::string file_prefix(const std::string &sample_name) {
    return fs::path{sample_name}.filename().replace_extension("");
}

// This is used for the test_reprint test cases.
// It follows a simple procedure which seems effective.
//
// 1. Parse a sample "A" and print it to a file "B".
// 2. Parse file "B" and print it to file "C".
// 3. Make sure that "B" and "C" are the same.
//
// Also this is the one used in the nlohman JSON test cases, so
// I consider it to be a not-bad idea :D.
std::pair<std::string, std::string> reprint(const std::string &sample_name) {
    using namespace json_parser;

    const std::string out_dir = TESTS_DIR_PREFIX"reprints/" + file_prefix(sample_name) + "/";
    fs::create_directories(out_dir);

    const json parsed1 = parser{sample_name}();
    const std::string out1 = out_dir + "out1.json";
    std::ofstream out1_ifs{out1, std::ios::trunc | std::ios::out};
    parsed1.dump(out1_ifs);

    const json parsed2 = parser{out1}();
    const std::string out2 = out_dir + "out2.json";
    std::ofstream out2_ifs{out2, std::ios::trunc | std::ios::out};
    parsed1.dump(out2_ifs);

    return std::make_pair(slurp(out1), slurp(out2));
}

#endif // FMI_JSON_PARSER_TESTS_COMMON_INCLUDED
