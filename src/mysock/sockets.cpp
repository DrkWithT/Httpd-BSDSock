#include <unistd.h>
#include <utility>
#include "mysock/sockets.hpp"

namespace MyHttpd::MySock {
    SockSetupStatus ServerSocket::applyOptions(long listen_timeout) noexcept {
        if (m_fd == dud_value) {
            return SockSetupStatus::bad_fd;
        }

        timeval timeout_secs {
            .tv_sec = listen_timeout,
            .tv_usec = 0
        };

        if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout_secs, sizeof(timeval)) == dud_value) {
            return SockSetupStatus::bad_option;
        }

        return SockSetupStatus::ok;
    }

    ServerSocket::ServerSocket() noexcept
    : m_fd {dud_value}, m_ready {false} {}

    ServerSocket::ServerSocket(int fd, long listen_timeout) noexcept
    : m_fd {fd}, m_ready {applyOptions(listen_timeout) == SockSetupStatus::ok} {}

    ServerSocket::~ServerSocket() noexcept {
        if (m_fd != dud_value) {
            close(m_fd);
            m_fd = dud_value;
        }
    }

    ServerSocket::ServerSocket(ServerSocket&& x_other) noexcept
    : m_fd {dud_value}, m_ready {false} {
        if (&x_other == this) {
            return;
        }

        if (m_fd != dud_value) {
            close(m_fd);
        }

        m_fd = std::exchange(x_other.m_fd, dud_value);
        m_ready = std::exchange(x_other.m_ready, false);
    }

    ServerSocket& ServerSocket::operator=(ServerSocket&& x_other) noexcept {
        if (&x_other == this) {
            return *this;
        }

        if (m_fd != dud_value) {
            close(m_fd);
        }

        m_fd = std::exchange(x_other.m_fd, dud_value);
        m_ready = std::exchange(x_other.m_ready, false);

        return *this;
    }

    bool ServerSocket::isReady() const noexcept {
        return m_fd != dud_value;
    }

    std::optional<int> ServerSocket::acceptConnection() noexcept {
        if (not m_ready) {
            return {};
        }

        auto temp_fd = accept(m_fd, nullptr, nullptr);

        if (temp_fd == dud_value) {
            return {};
        }

        return {temp_fd};
    }


    SockSetupStatus ClientSocket::applyOptions(long recv_timeout) noexcept {
        if (m_fd == dud_value) {
            return SockSetupStatus::bad_fd;
        }
    
        timeval timeout_secs {
            .tv_sec = recv_timeout,
            .tv_usec = 0
        };
    
        if (setsockopt(m_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout_secs, sizeof(timeval)) == dud_value) {
            return SockSetupStatus::bad_option;
        }
    
        return SockSetupStatus::ok;
    }

    ClientSocket::ClientSocket() noexcept
    : m_fd {dud_value}, m_closed {true} {}

    ClientSocket::ClientSocket(int fd, long recv_timeout) noexcept
    : m_fd {fd}, m_closed {false} {
        applyOptions(recv_timeout);
    }

    ClientSocket::~ClientSocket() noexcept {
        if (m_fd != dud_value) {
            close(m_fd);
            m_fd = dud_value;
        }
    }

    ClientSocket::ClientSocket(ClientSocket&& x_other) noexcept
    : m_fd {dud_value}, m_closed {true} {
        m_fd = std::exchange(x_other.m_fd, dud_value);
        m_closed = std::exchange(x_other.m_closed, true);
    }

    ClientSocket& ClientSocket::operator=(ClientSocket&& x_other) noexcept {
        if (&x_other == this) {
            return *this;
        }

        if (m_fd != dud_value) {
            close(m_fd);
        }

        m_fd = std::exchange(x_other.m_fd, dud_value);
        m_closed = std::exchange(x_other.m_closed, true);

        return *this;
    }

    bool ClientSocket::isReady() const noexcept {
        return m_fd != dud_value;
    }
}