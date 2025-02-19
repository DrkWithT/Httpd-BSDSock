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

#include <iostream>
#include "mysock/configure.hpp"
#include "mydriver/driver.hpp"

static constexpr auto minimum_argc = 3;

int main(int argc, char* argv[]) {
    using namespace MyHttpd;

    if (argc < minimum_argc) {
        std::print(std::cerr, "Error: invalid argc of {}\n\tusage: ./myhttpd <port> <timeout>\n", argc);
        return 1;
    }

    auto socket_gen = MySock::SocketGenerator::makeSelf(argv[1]);

    auto make_socket = [&socket_gen] [[nodiscard]] (long io_timeout) {
        while (socket_gen) {
            auto fd_opt = socket_gen();

            if (fd_opt.has_value()) {
                return MySock::ServerSocket {fd_opt.value(), io_timeout};
            }
        }

        return MySock::ServerSocket {};
    };

    MyDriver::ServerDriver app {make_socket(std::atol(argv[2]))};

    if (auto serve_status = app.runService(); serve_status != MyDriver::ExitCode::ok) {
        std::print(std::cerr, "Big Error: {}\n", fetchExitCodeMsg(serve_status));
    }
}
