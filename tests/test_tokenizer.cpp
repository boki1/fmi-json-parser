#include <gtest/gtest.h>

#include <sstream>
#include <tuple>
#include <iterator>

#include <mystd/memory.h>

#include <json-parser/tokenizer.h>

using namespace json_parser;

#ifndef TESTS_DIR_PREFIX
#define TESTS_DIR_PREFIX ""
#endif

static constexpr std::string g_tests_dir_prefix = TESTS_DIR_PREFIX;

struct tokenized_details {
    std::string as_text;
    std::size_t num_tokens;
};

static tokenized_details tokenize_and_get_details(const std::string &json_input_filename) {
    tokenizer tokenizer{g_tests_dir_prefix + json_input_filename};
    std::size_t num_tokens = 0;
    std::ostringstream sstr;
    auto it = tokenizer.begin();
    auto end_it = tokenizer.end();
    for ( ; it != end_it; ++it) {
        mystd::unique_ptr<token> tok = *it;
        tok->serialize(sstr);
        ++num_tokens;
    }

    return tokenized_details {
        .as_text = sstr.str(),
        .num_tokens = num_tokens
    };
}

static std::string slurp_expected_output(const std::string &expected_output_filename) {
    // Seems not to be most efficient implementation but seems elegant to me.
    // Source: https://stackoverflow.com/questions/116038/how-do-i-read-an-entire-file-into-a-stdstring-in-c#116220
    std::ifstream in{g_tests_dir_prefix + expected_output_filename};
    return std::string{std::istreambuf_iterator<char>{in}, {}};
}

TEST(JsonTests, TokenizeSimple)
{
    // FIXME: Change to relative path!
    auto [actual, num_tokens] = tokenize_and_get_details("tests/samples/simple.json");
    auto expected = slurp_expected_output("tests/expected/simple.string");

    EXPECT_EQ(num_tokens, 13);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTests, TokenizeJokes)
{
    // FIXME: Change to relative path!
    auto [actual, num_tokens] = tokenize_and_get_details("tests/samples/jokes.json");
    auto expected = slurp_expected_output("tests/expected/jokes.string");

    EXPECT_EQ(num_tokens, 357);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTests, TokenizeNested)
{
    // FIXME: Change to relative path!
    auto [actual, num_tokens] = tokenize_and_get_details("tests/samples/nested.json");
    auto expected = slurp_expected_output("tests/expected/nested.string");

    EXPECT_EQ(num_tokens, 85);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTests, TokenizeEmpty)
{
    // FIXME: Change to relative path!
    auto [as_text, num_tokens] = tokenize_and_get_details("tests/samples/empty.json");

    EXPECT_EQ(num_tokens, 0);
    EXPECT_EQ(as_text, "");
}

TEST(JsonTests, TokenizeBadUnclosedString)
{
    // FIXME: Change to relative path!
    EXPECT_THROW(tokenize_and_get_details("tests/samples/bad_unclosed_string.json"), token_exception);
}
