#include <sstream>

#include <gtest/gtest.h>

#include <json-parser/parser.h>

#include <json-parser-tests/common.h>

using namespace json_parser;

TEST(JsonTests, JsonExtractMappedIfFilter) {
    auto parsed = parse_from_file(TESTS_DIR_PREFIX"samples/organisation.json");
    auto id = json::make_node<json::string>("id");
    const auto all_ids = parsed.extract_mapped_if(
        [target = id->clone()](
            const json::value &key, const json::value &) {
            return key == *target;
        });
    std::ostringstream osstr;
    all_ids.dump(osstr);

    EXPECT_EQ(osstr.str(),
"[\n"
"  2,\n"
"  1,\n"
"  0\n"
"]"
);

    osstr = std::ostringstream {};
    auto missing = json::make_node<json::string>("missing");
    const auto all_missing = parsed.extract_mapped_if(
        [target = missing->clone()](
            const json::value &key, const json::value &) {
            return key == *target;
        });
    all_missing.dump(osstr);

    EXPECT_EQ(osstr.str(), "[ ]\n");
}

TEST(JsonTests, JsonFollowPath) {
    auto parsed = parse_from_file(TESTS_DIR_PREFIX"samples/organisation.json");
    json::path path_to_the_address_of_office1;
    path_to_the_address_of_office1.emplace_back(json::make_node<json::string>("offices"));
    path_to_the_address_of_office1.emplace_back(json::make_node<json::number>(1));
    path_to_the_address_of_office1.emplace_back(json::make_node<json::string>("address"));

    auto expected_address = json::make_node<json::string>("New York City");
    const json::value &actual_address = *parsed.follow(path_to_the_address_of_office1);
    EXPECT_EQ(*expected_address, actual_address);

    json::path path_to_the_birthday_of_JohnDoe;
    path_to_the_birthday_of_JohnDoe.emplace_back(json::make_node<json::string>("members"));
    path_to_the_birthday_of_JohnDoe.emplace_back(json::make_node<json::number>(2));
    path_to_the_birthday_of_JohnDoe.emplace_back(json::make_node<json::string>("birthdate"));

    auto expected_birthday = json::make_node<json::string>("1982-03-03");
    const json::value &actual_birthday = *parsed.follow(path_to_the_birthday_of_JohnDoe);
    EXPECT_EQ(*expected_birthday, actual_birthday);

    json::path empty_path;
    EXPECT_NO_THROW((void) parsed.follow(empty_path));

    json::path invalid_path;
    invalid_path.emplace_back(json::make_node<json::string>("key-that-does-not-exist"));
    EXPECT_THROW((void) parsed.follow(invalid_path), json_exception);
}
