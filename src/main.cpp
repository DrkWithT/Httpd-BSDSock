/**
 * @file main.cpp
 * @author DrkWithT
 * @brief Implements driver logic of my HTTP/1.1 server.
 * @version 0.0.1
 * @date 2025-02-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <format>
#include <iostream>

static constexpr auto minimum_argc = 2;

int main(int argc, [[maybe_unused]] char* argv[]) {
    if (argc < minimum_argc) {
        std::cerr << std::format("Error: invalid argc of {}\n\tusage: ./myhttpd <port> ?<static-dir>\n", argc);
        return 1;
    }
}
