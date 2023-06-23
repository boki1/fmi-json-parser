#include <iostream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include <mystd/unordered_map.h>

std::ostringstream sstr;

TEST(MystdTests, UnorderedMap) {
  // Create an unordered_map of three strings (that map to strings)
  mystd::unordered_map<std::string, std::string> u;
  u.emplace("RED", "#FF0000");
  u.emplace("GREEN", "#00FF00");
  u.emplace("BLUE", "#0000FF");

  // Helper lambda function to print key-value pairs
  auto print_key_value = [](const auto &key, const auto &value) {
    sstr << "Key:[" << key << "] Value:[" << value << "]\n";
  };

  sstr << "Iterate and print key-value pairs using C++17 structured "
          "binding:\n";
  for (const auto &[key, value] : u)
    print_key_value(key, value);

  // Add two new entries to the unordered_map
  u["BLACK"] = "#000000";
  u["WHITE"] = "#FFFFFF";

  sstr << "\nOutput values by key:\n"
          "The HEX of color RED is:["
       << u["RED"]
       << "]\n"
          "The HEX of color BLACK is:["
       << u["BLACK"] << "]\n\n";

  sstr << "Use operator[] with non-existent key to insert a new key-value "
          "pair:\n";
  print_key_value("new_key", u["new_key"]);

  sstr << "\nIterate and print key-value pairs, using `auto`;\n"
          "new_key is now one of the keys in the map:\n";
  for (const auto &n : u)
    print_key_value(n.first, n.second);

  EXPECT_EQ(
      sstr.str(),
      "Iterate and print key-value pairs using C++17 structured binding:\n"
      "Key:[RED] Value:[#FF0000]\n"
      "Key:[GREEN] Value:[#00FF00]\n"
      "Key:[BLUE] Value:[#0000FF]\n\n"
      "Output values by key:\n"
      "The HEX of color RED is:[#FF0000]\n"
      "The HEX of color BLACK is:[#000000]\n\n"
      "Use operator[] with non-existent key to insert a new key-value pair:\n"
      "Key:[new_key] Value:[]\n\n"
      "Iterate and print key-value pairs, using `auto`;\n"
      "new_key is now one of the keys in the map:\n"
      "Key:[RED] Value:[#FF0000]\n"
      "Key:[GREEN] Value:[#00FF00]\n"
      "Key:[BLUE] Value:[#0000FF]\n"
      "Key:[BLACK] Value:[#000000]\n"
      "Key:[WHITE] Value:[#FFFFFF]\n"
      "Key:[new_key] Value:[]\n"
	  );
}
