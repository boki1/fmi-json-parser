#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include <mystd/optional.h>

std::ostringstream sstr;

///
/// These test cases are taken directly from cppreference.
///
///
// optional can be used as the return type of a factory that may fail
mystd::optional<std::string> create(bool b) {
    if (b)
        return "Godzilla";
    return {};
}

// std::nullopt can be used to create any (empty) mystd::optional
auto create2(bool b) {
    return b ? mystd::optional<std::string>{"Godzilla"} : std::nullopt;
}

// std::reference_wrapper may be used to return a reference
auto create_ref(bool b) {
    static std::string value = "Godzilla";
    return b ? mystd::optional<std::reference_wrapper<std::string>>{value}
             : std::nullopt;
}

TEST(MystdTests, Optional1) {


    sstr << "create(false) returned " << create(false).value_or("empty") << '\n';

  // optional-returning factory functions are usable as conditions of while and
  // if
  if (auto str = create2(true)) {
    sstr << "create2(true) returned " << *str << '\n';
  }

  if (auto str = create_ref(true)) {
    // using get() to access the reference_wrapper's value
    sstr << "create_ref(true) returned " << str->get() << '\n';
    str->get() = "Mothra";
    sstr << "modifying it changed it to " << str->get() << '\n';
  }

  EXPECT_EQ(sstr.str(), "create(false) returned empty\n"
                        "create2(true) returned Godzilla\n"
                        "create_ref(true) returned Godzilla\n"
                        "modifying it changed it to Mothra\n");
  sstr = std::ostringstream{};
}

TEST(MystdTests, Optional2) {
  using namespace std::string_literals;
  mystd::optional<int> opt1 = 1;
  sstr << "opt1: " << *opt1 << '\n';

  *opt1 = 2;
  sstr << "opt1: " << *opt1 << '\n';

  mystd::optional<std::string> opt2 = "abc"s;
  sstr << "opt2: " << *opt2 << " size: " << opt2->size() << '\n';

  // You can "take" the contained value by calling operator* on an rvalue to
  // optional

  auto taken = *std::move(opt2);
  sstr << "taken: " << taken << " opt2: " << *opt2 << "size: " << opt2->size()
       << '\n';

  EXPECT_EQ(sstr.str(), "opt1: 1\n"
                        "opt1: 2\n"
                        "opt2: abc size: 3\n"
                        "taken: abc opt2: size: 0\n");
    sstr = std::ostringstream{};
}
