#include <sys/socket.h>
#include <unistd.h>
#include "mysock/configure.hpp"

namespace MyHttpd::MySock {
    static constexpr auto dud_value = -1;
    static constexpr auto success_value = 0;

    SocketGenerator::~SocketGenerator() noexcept {
        if (hasHead()) {
            freeaddrinfo(m_head);
            m_head = nullptr;
        }
    }

    SocketGenerator::operator bool() const noexcept {
        return hasNext();
    }

    std::optional<SockFD> SocketGenerator::operator()() noexcept {
        if (not hasNext()) {
            return {};
        }

        auto temp_fd = socket(m_cursor->ai_family, m_cursor->ai_socktype, m_cursor->ai_protocol);

        if (temp_fd == dud_value) {
            return {};
        }

        if (bind(temp_fd, m_cursor->ai_addr, m_cursor->ai_addrlen) == dud_value) {
            close(temp_fd);
            return {};
        }

        return {temp_fd};
    }

    /// NOTE: For SocketGenerator, and a C-string literal must be passed to `hints.port_sv`!
    [[nodiscard]] SocketGenerator makeSelf(std::string_view port_sv) noexcept {
        if (port_sv.empty()) {
            return {};
        }

        return {port_sv.data()};
    }

    SocketGenerator::SocketGenerator()
    : m_head {nullptr}, m_cursor {nullptr} {}

    SocketGenerator::SocketGenerator(const char* port_cstr)
    : m_head {nullptr}, m_cursor {nullptr} {
        addrinfo hints;
        std::memset(&hints, 0, sizeof(addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if (auto status = getaddrinfo(nullptr, port_cstr, &hints, &m_head); status != success_value) {
            return;
        }

        m_cursor = m_head;
    }

    bool SocketGenerator::hasHead() const noexcept {
        return m_head;
    }

    bool SocketGenerator::hasNext() const noexcept {
        return m_cursor != nullptr;
    }
}