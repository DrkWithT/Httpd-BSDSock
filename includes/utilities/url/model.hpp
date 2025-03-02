#pragma once

#include <string_view>
#include <variant>
#include <vector>

namespace MyHttpd::Utilities::Url {
    struct Nil {};

    using ItemValue = std::variant<Nil, int, float, std::string>;

    struct QueryItem {
        std::string_view key;
        ItemValue value;
    };

    struct URL {
        std::string_view path;
        std::vector<QueryItem> query;
    };
}