#ifndef FMI_JSON_PARSER_TESTS_COMMON_INCLUDED
#define FMI_JSON_PARSER_TESTS_COMMON_INCLUDED

#include <string>
#include <fstream>

#ifndef TESTS_DIR_PREFIX
#define TESTS_DIR_PREFIX "tests/"
#endif

std::string slurp(const std::string &filename) {
    // Seems not to be most efficient implementation but seems elegant to me.
    // Source: https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c#116220

    std::ifstream in{filename};
    return std::string{std::istreambuf_iterator<char>{in}, {}};
}

#endif // FMI_JSON_PARSER_TESTS_COMMON_INCLUDED
