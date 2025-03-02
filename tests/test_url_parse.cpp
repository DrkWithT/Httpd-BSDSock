#include <iostream>
#include <print>
#include <string_view>
#include <string>
#include "utilities/url/model.hpp"
#include "utilities/url/parsing.hpp"

using URLModel = MyHttpd::Utilities::Url::URL;
using URLParser = MyHttpd::Utilities::Url::Parser;

int main() {
    URLParser demo1 {"/"};
    auto result1 = demo1.parseAll();

    if (result1.path != "/") {
        std::print(std::cerr, "Check 1 failed!");
        return 1;
    }

    URLParser demo2 {"/foo/bar"};
    auto result2 = demo2.parseAll();

    if (result2.path != "/foo/bar") {
        std::print(std::cerr, "Check 2 failed!");
        return 1;
    }

    URLParser demo3 {"/foo?answer=42"};
    auto result3 = demo3.parseAll();

    if (auto result3_item1 = result3.query.at(0).value; result3.path != "/foo" or std::get<int>(result3_item1) != 42) {
        std::print(std::cerr, "Check 3 failed!");
        return 1;
    }

    URLParser demo4 {"/foo?data=3.1415"};
    auto result4 = demo4.parseAll();

    if (auto result4_item1 = result4.query.at(0).value; result4.path != "/foo" or std::get<float>(result4_item1) != 3.1415f) {
        std::print(std::cerr, "Check 4 failed!");
        return 1;
    }

    URLParser demo5 {"/foo?data=hello&answer=42"};
    auto result5 = demo5.parseAll();

    if (result5.path != "/foo") {
        std::print(std::cerr, "Check 5a failed!");
        return 1;
    }

    auto result5_item1 = result5.query.at(0);
    auto result5_item2 = result5.query.at(1);

    if (result5_item1.key != "data" or std::get<std::string>(result5_item1.value) != "hello") {
        std::print(std::cerr, "Check 5b failed!");
        return 1;
    }

    if (result5_item2.key != "answer" or std::get<int>(result5_item2.value) != 42) {
        std::print(std::cerr, "Check 5c failed!");
        return 1;
    }
}