#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "meta/helpers.hpp"
#include "mysock/buffers.hpp"

namespace MyHttpd::MySock {
    enum class SockSetupStatus {
        ok,
        bad_fd,
        bad_option
    };

    enum class SockIOStatus {
        ok,
        closed_pipe,
        invalid_size,
        exhausted_buffer
    };

    class ServerSocket {
    private:
        static constexpr auto dud_value = -1;
        int m_fd;
        bool m_ready;

        [[nodiscard]] SockSetupStatus applyOptions(long listen_timeout) noexcept;

    public:
        ServerSocket() noexcept;
        ServerSocket(int fd, long listen_timeout) noexcept;
        ~ServerSocket() noexcept;

        ServerSocket(const ServerSocket& other) = delete;
        ServerSocket& operator=(const ServerSocket& other) = delete;
        ServerSocket(ServerSocket&& x_other) noexcept;
        ServerSocket& operator=(ServerSocket&& x_other) noexcept;

        [[nodiscard]] std::optional<int> acceptConnection() noexcept;
    };

    class ClientSocket {
    private:
        static constexpr auto dud_value = -1;

        int m_fd;
        bool m_closed;

        [[maybe_unused]] SockSetupStatus applyOptions(long recv_timeout) noexcept;

    public:
        ClientSocket() noexcept;
        ClientSocket(int fd, long recv_timeout) noexcept;
        ~ClientSocket() noexcept;

        ClientSocket(const ClientSocket& other) = delete;
        ClientSocket& operator=(const ClientSocket& other) = delete;
        ClientSocket(ClientSocket&& x_other) noexcept;
        ClientSocket& operator=(ClientSocket&& x_other) noexcept;

        template <typename OctetT, std::size_t BufferN> requires (Meta::is_buffer_item_v<OctetT>)
        [[nodiscard]] SockIOStatus readLine(FixedBuffer<OctetT, BufferN>& target, OctetT delim) noexcept {
            auto residue_space = BufferN;
            auto read_n = 0UL;
            OctetT temp = OctetT {};
            bool found_delim = false;

            while (not m_closed and residue_space > 0) {
                auto temp_read_n = recv(m_fd, &temp, 1UL, 0);

                if (temp_read_n <= 0) {
                    m_closed = true;
                    return SockIOStatus::closed_pipe;
                }

                if (temp == delim) {
                    found_delim = true;
                    break;
                }

                if (delim == '\n' and temp == '\r') {
                    continue;
                }

                target.getPtr()[read_n] = temp;

                ++read_n;
                --residue_space;
            }

            if (not found_delim and not residue_space) {
                target.reset();
                return SockIOStatus::exhausted_buffer;
            }

            return SockIOStatus::ok;
        }

        template <typename OctetT, std::size_t BufferN> requires (Meta::is_buffer_item_v<OctetT>)
        [[nodiscard]] SockIOStatus readBlob(const FixedBuffer<OctetT, BufferN>& target, std::size_t blob_size) noexcept {
            if (blob_size > BufferN) {
                return SockIOStatus::invalid_size;
            }

            auto pending_n = blob_size;
            auto done_n = 0UL;
            long temp_n = 0L;

            while (not m_closed and pending_n > 0UL) {
                temp_n = recv(m_fd, target.getPtr() + done_n, pending_n, 0);

                if (temp_n <= 0) {
                    m_closed = true;
                    target.reset();

                    return SockIOStatus::closed_pipe;
                }

                done_n += temp_n;
                pending_n -= temp_n;
            }

            return (pending_n == 0UL) ? SockIOStatus::ok : SockIOStatus::closed_pipe;
        }

        template <typename OctetT, std::size_t BufferN>
        [[nodiscard]] SockIOStatus writeBlob(const FixedBuffer<OctetT, BufferN>& source) noexcept {
            auto pending_n = source.getLength();

            if (pending_n == 0UL) {
                return SockIOStatus::invalid_size;
            }

            auto done_n = 0UL;

            while (not m_closed and pending_n > 0UL) {
                auto temp_n = send(m_fd, source.getPtr() + done_n, pending_n, 0);

                if (temp_n <= 0L) {
                    m_closed = true;
                    source.reset();
                    return SockIOStatus::closed_pipe;
                }

                done_n += temp_n;
                pending_n -= temp_n;
            }

            return (pending_n == 0UL) ? SockIOStatus::ok : SockIOStatus::closed_pipe;
        }

        template <typename OctetT, std::size_t BufferN>
        [[nodiscard]] SockIOStatus writeBlob(const FixedBuffer<OctetT, BufferN>& source, std::size_t N) noexcept {
            auto pending_n = N;

            if (N == 0UL or N > source.getLength()) {
                return SockIOStatus::invalid_size;
            }

            auto done_n = 0UL;

            while (not m_closed and pending_n > 0UL) {
                auto temp_n = send(m_fd, source.getPtr() + done_n, pending_n, 0);

                if (temp_n <= 0L) {
                    m_closed = true;
                    source.reset();
                    return SockIOStatus::closed_pipe;
                }

                done_n += temp_n;
                pending_n -= temp_n;
            }

            return (pending_n == 0UL) ? SockIOStatus::ok : SockIOStatus::closed_pipe;
        }
    };
}