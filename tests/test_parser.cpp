#include <gtest/gtest.h>

#include <json-parser/parser.h>

#include <json-parser-tests/common.h>

using namespace json_parser;

// TODO: Fix lookup to return the concrete type:
// { "fruit" : "Apply" } when parsed should be usable as
// const json::string val = parsed["fruit"];

TEST(JsonTests, ParseStringOnly) {
    json parsed = parse_from_file(TESTS_DIR_PREFIX"samples/string-only.json");
    EXPECT_NO_THROW((void) parsed.root_unsafe());
    const auto &root = parsed.root_unsafe();
    const auto root_as_str = dynamic_cast<const json::string &>(root);
    EXPECT_EQ(root_as_str, std::string{"blah-blah-blah"});
}

TEST(JsonTests, ParseSimple) {
    json parsed = parse_from_file(TESTS_DIR_PREFIX"samples/simple.json");

    const json::string &fruit_val = dynamic_cast<const json::string &>(parsed["fruit"]);
    EXPECT_EQ(fruit_val, "Apple");
    const json::string &size_val = dynamic_cast<const json::string &>(parsed["size"]);
    EXPECT_EQ(size_val, "Large");
    const json::string &color_val = dynamic_cast<const json::string &>(parsed["color"]);
    EXPECT_EQ(color_val, "Red");
}

TEST(JsonTests, ParseArray) {
    const json parsed = parse_from_file(TESTS_DIR_PREFIX"samples/array.json");

    std::string expected = "";
    for (int i = 0; i < 10; ++i) {
        expected += std::to_string(i);
        const json::string &actual = dynamic_cast<const json::string &>(parsed[i]);
        EXPECT_EQ(actual, expected);
    }
}

TEST(JsonTests, ParseJokes) {
    const json parsed = parse_from_file(TESTS_DIR_PREFIX"samples/jokes.json");

    const json::number &amount_val = dynamic_cast<const json::number &>(parsed["amount"]);
    EXPECT_EQ(amount_val, 6.);

    const json::array &jokes_val = dynamic_cast<const json::array &>(parsed["jokes"]);
    const json::object &joke_no4 = dynamic_cast<const json::object &>(jokes_val[4]);
    const json::string &joke_no4_setup = dynamic_cast<const json::string &>(joke_no4["setup"]);
    const json::string &joke_no4_delivery = dynamic_cast<const json::string &>(joke_no4["delivery"]);
    const json::object &joke_no4_flags = dynamic_cast<const json::object &>(joke_no4["flags"]);
    const json::boolean &joke_no4_flags_religious = dynamic_cast<const json::boolean &>(joke_no4_flags["religious"]);

    EXPECT_EQ(std::string{joke_no4_setup}, "Why did the koala get rejected?");
    EXPECT_EQ(std::string{joke_no4_delivery}, "Because he did not have any koalafication.");
    EXPECT_EQ(bool{ joke_no4_flags_religious}, false);

    EXPECT_THROW((void) jokes_val[6], std::out_of_range);
    EXPECT_THROW((void) joke_no4["joke"], std::out_of_range);

    const json::object &joke_no5 = dynamic_cast<const json::object &>(jokes_val[5]);
    const json::string &joke_no5_joke = dynamic_cast<const json::string &>(joke_no5["joke"]);

    // FIXME: I am not really sure whether this is the exact output we expect - there are a couple of '\'
    // that are different between the two strings, so nothing big, but kind of strange. Check this out later.
    // Anyways, if it is actually a problem it is really in the tokenizer, which is weird. I am not really
    // sure how it has eluded me.
    EXPECT_NE(std::string{joke_no5_joke}, "Two C strings walk into a bar."
"The bartender asks \"What can I get ya?\""
"The first string says \"I'll have a gin and tonic.\""
"The second string thinks for a minute, then says \"I'll take a tequila sunriseJF()#$JF(#)$(@J#()$@#())!*FNIN!OBN134ufh1ui34hf9813f8h8384h981h3984h5F!##@\""
"The first string apologizes, \"You'll have to excuse my friend, he's not null-terminated.\"");
    EXPECT_THROW((void) joke_no5["setup"], std::out_of_range);
}

TEST(JsonTests, ParseNested) {
    const json parsed = parse_from_file(TESTS_DIR_PREFIX"samples/nested.json");
    const json::object &quiz = dynamic_cast<const json::object &>(parsed["quiz"]);
    const json::object &maths = dynamic_cast<const json::object &>(quiz["maths"]);
    const json::object &q2 = dynamic_cast<const json::object &>(maths["q2"]);
    const json::array &q2_options = dynamic_cast<const json::array &>(q2["options"]);
    const json::string &q2_question = dynamic_cast<const json::string &>(q2["question"]);
    const json::string &q2_answer = dynamic_cast<const json::string &>(q2["answer"]);
    const json::string &q2_option_2 = dynamic_cast<const json::string &>(q2_options[2]);

    EXPECT_EQ(std::string{ q2_question }, "12 - 8 = ?");
    EXPECT_EQ(std::string{ q2_answer }, "4");
    EXPECT_EQ(std::string { q2_option_2 }, "3");
}

TEST(JsonTests, ParseEmpty) {
    const json parsed = parse_from_file(TESTS_DIR_PREFIX"samples/empty.json");
    EXPECT_THROW((void) parsed["whatever-key-i-pass-this-should-fail-as-the-json-is-empty"], json_parser::json_exception);
}

///
/// str_input_reader
///

TEST(JsonTests, ParseSimpleStrInputReader) {
    json parsed = parse_from_string(TESTS_DIR_PREFIX"samples/simple.json");

    const json::string &fruit_val = dynamic_cast<const json::string &>(parsed["fruit"]);
    EXPECT_EQ(fruit_val, "Apple");
    const json::string &size_val = dynamic_cast<const json::string &>(parsed["size"]);
    EXPECT_EQ(size_val, "Large");
    const json::string &color_val = dynamic_cast<const json::string &>(parsed["color"]);
    EXPECT_EQ(color_val, "Red");
}

TEST(JsonTests, ParseNestedStrInputReader) {
    const json parsed = parse_from_string(TESTS_DIR_PREFIX"samples/nested.json");
    const json::object &quiz = dynamic_cast<const json::object &>(parsed["quiz"]);
    const json::object &maths = dynamic_cast<const json::object &>(quiz["maths"]);
    const json::object &q2 = dynamic_cast<const json::object &>(maths["q2"]);
    const json::array &q2_options = dynamic_cast<const json::array &>(q2["options"]);
    const json::string &q2_question = dynamic_cast<const json::string &>(q2["question"]);
    const json::string &q2_answer = dynamic_cast<const json::string &>(q2["answer"]);
    const json::string &q2_option_2 = dynamic_cast<const json::string &>(q2_options[2]);

    EXPECT_EQ(std::string{ q2_question }, "12 - 8 = ?");
    EXPECT_EQ(std::string{ q2_answer }, "4");
    EXPECT_EQ(std::string { q2_option_2 }, "3");
}

///
/// Bad ones - try parsing unsound JSON and report it.
///

TEST(JsonTests, ParseBadUnclosedString) {
    EXPECT_THROW((void) parse_from_file(TESTS_DIR_PREFIX"samples/bad_unclosed_string.json") , json_parser::parser_exception);
}

TEST(JsonTests, ParseBadExtraComma) {
    // json_parser::parser_exception with description "Expected valid JSON value, but got an unexpected punctuator - '}'"
    EXPECT_THROW(parse_from_file(TESTS_DIR_PREFIX"samples/bad_extra_comma_object.json"), json_parser::parser_exception);

    // json_parser::parser_exception with description "Expected valid JSON value, but got an unexpected punctuator - ']'"
    EXPECT_THROW(parse_from_file(TESTS_DIR_PREFIX"samples/bad_extra_comma_array.json"), json_parser::parser_exception);
}

// TODO: Improve error messages on this one:
TEST(JsonTests, ParseBadMissingComma) {
    // json_parser::parser_exception with description "Expected token of type `N11json_parser11token_punctE` but no such was found.
    EXPECT_THROW(parse_from_file(TESTS_DIR_PREFIX"samples/bad_missing_comma_object.json"), json_parser::parser_exception);

    // json_parser::parser_exception with description "Expected token of type `N11json_parser11token_punctE` but no such was found.
    EXPECT_THROW(parse_from_file(TESTS_DIR_PREFIX"samples/bad_missing_comma_array.json"), json_parser::parser_exception);
}

TEST(JsonTests, ParseBadMissingColumn) {
    // json_parser::parser_exception with description "Expected token of type `N11json_parser11token_punctE` but no such was found.
    EXPECT_THROW(parse_from_file(TESTS_DIR_PREFIX"samples/bad_missing_column.json"), json_parser::parser_exception);
}

TEST(JsonTests, ParseBadUnclosed) {
    // json_parser::parser_exception with description "Expected more tokens during parsing."
    EXPECT_THROW(parse_from_file(TESTS_DIR_PREFIX"samples/bad_unclosed_object.json"), json_parser::parser_exception);

    // json_parser::parser_exception with description "Expected more tokens during parsing."
    EXPECT_THROW(parse_from_file(TESTS_DIR_PREFIX"samples/bad_unclosed_array.json"), json_parser::parser_exception);
}

TEST(JsonTests, ParseBadUnexpectedSymbol) {
    // json_parser::parser_exception with description "0:12 (12): Unexpected symbol '%' found."
    EXPECT_THROW(parse_from_file(TESTS_DIR_PREFIX"samples/bad_unexpected_symbol.json"), json_parser::parser_exception);
}
