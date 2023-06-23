#include <gtest/gtest.h>

#include <json-parser/parser.h>

#include <json-parser-tests/common.h>

using namespace json_parser;

void reprint_cmp(const std::string &sample_name) {
    auto [print1, print2] = reprint(std::string{TESTS_DIR_PREFIX} + sample_name);
    EXPECT_EQ(print1, print2);
}

// We try this test case procedure only on the sane JSON inputs as
// if the input is not correct it won't be parsed fully.
TEST(JsonTests, Reprint) {
    reprint_cmp("samples/empty.json");
    reprint_cmp("samples/string-only.json");
    reprint_cmp("samples/array.json");
    reprint_cmp("samples/simple.json");
    reprint_cmp("samples/jokes.json");
    reprint_cmp("samples/nested.json");
    reprint_cmp("samples/array_of_objects.json");
    reprint_cmp("samples/organisation.json");
}
