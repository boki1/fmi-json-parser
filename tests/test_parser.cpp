#include <gtest/gtest.h>

#include <json-parser/parser.h>

#ifndef TESTS_DIR_PREFIX
#define TESTS_DIR_PREFIX ""
#endif

using namespace json_parser;

// TODO: Fix lookup to return the concrete type:
// { "fruit" : "Apply" } when parsed should be usable as
// const json::string val = parsed["fruit"];

TEST(JsonTests, ParseStringOnly) {
    json parsed = parser{TESTS_DIR_PREFIX"tests/samples/string-only.json"}();
    EXPECT_NO_THROW(parsed.root_unsafe());
    const auto &root = parsed.root_unsafe();
    const auto root_as_str = dynamic_cast<const json::string &>(root);
    EXPECT_EQ(root_as_str, std::string{"blah-blah-blah"});
}

TEST(JsonTests, ParseSimple) {
    json parsed = parser{TESTS_DIR_PREFIX"tests/samples/simple.json"}();

    const json::string &fruit_val = dynamic_cast<const json::string &>(parsed["fruit"]);
    EXPECT_EQ(fruit_val, "Apple");
    const json::string &size_val = dynamic_cast<const json::string &>(parsed["size"]);
    EXPECT_EQ(size_val, "Large");
    const json::string &color_val = dynamic_cast<const json::string &>(parsed["color"]);
    EXPECT_EQ(color_val, "Red");
}

TEST(JsonTests, ParseArray) {
    const json parsed = parser{TESTS_DIR_PREFIX"tests/samples/array.json"}();

    std::string expected = "";
    for (int i = 0; i < 10; ++i) {
        expected += std::to_string(i);
        const json::string &actual = dynamic_cast<const json::string &>(parsed[i]);
        EXPECT_EQ(actual, expected);
    }
}
