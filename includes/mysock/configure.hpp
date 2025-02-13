#pragma once

#include <netdb.h>
#include <string_view>
#include <optional>

namespace MyHttpd::MySock {
    using SockFD = int;

    /**
     * @brief Utility to setup TCP server-socket descriptors for later use.
     */
    class SocketGenerator {
    public:
        ~SocketGenerator() noexcept;

        [[nodiscard]] explicit operator bool() const noexcept;
        std::optional<SockFD> operator()() noexcept;

        /// @note "factory" function: checks ctor arguments before attempted creation...
        friend SocketGenerator makeSelf(std::string_view port_sv) noexcept;

    private:
        SocketGenerator();
        SocketGenerator(const char* port_cstr);

        [[nodiscard]] bool hasHead() const noexcept;
        [[nodiscard]] bool hasNext() const noexcept;

        addrinfo* m_head;
        addrinfo* m_cursor;
    };
}