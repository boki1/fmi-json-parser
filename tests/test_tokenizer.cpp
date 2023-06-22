#include <gtest/gtest.h>

#include <sstream>
#include <tuple>
#include <iterator>

#include <mystd/memory.h>

#include <json-parser/tokenizer.h>

#include <json-parser-tests/common.h>

using namespace json_parser;

struct tokenized_details {
    std::string as_text;
    std::size_t num_tokens;
};

static tokenized_details tokenize_and_get_details(const std::string &json_input_filename) {
    ifs_tokenizer tokenizer{ifs_input_reader{json_input_filename}};
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

TEST(JsonTests, TokenizeSimple)
{
    auto [actual, num_tokens] = tokenize_and_get_details(TESTS_DIR_PREFIX"samples/simple.json");
    auto expected = slurp(TESTS_DIR_PREFIX"expected/simple.string");

    EXPECT_EQ(num_tokens, 13);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTests, TokenizeJokes)
{
    auto [actual, num_tokens] = tokenize_and_get_details(TESTS_DIR_PREFIX"samples/jokes.json");
    auto expected = slurp(TESTS_DIR_PREFIX"expected/jokes.string");

    EXPECT_EQ(num_tokens, 357);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTests, TokenizeNested)
{
    auto [actual, num_tokens] = tokenize_and_get_details(TESTS_DIR_PREFIX"samples/nested.json");
    auto expected = slurp(TESTS_DIR_PREFIX"expected/nested.string");

    EXPECT_EQ(num_tokens, 85);
    EXPECT_EQ(actual, expected);
}

TEST(JsonTests, TokenizeEmpty)
{
    auto [as_text, num_tokens] = tokenize_and_get_details(TESTS_DIR_PREFIX"samples/empty.json");

    EXPECT_EQ(num_tokens, 0);
    EXPECT_EQ(as_text, "");
}

TEST(JsonTests, TokenizeBadUnclosedString)
{
    EXPECT_THROW(tokenize_and_get_details(TESTS_DIR_PREFIX"samples/bad_unclosed_string.json"), token_exception);
}
