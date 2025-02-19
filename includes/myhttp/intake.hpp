#pragma once

#include <optional>
#include <string>
#include <sstream>
#include <unordered_map>
#include "mysock/buffers.hpp"
#include "mysock/sockets.hpp"
#include "myhttp/types.hpp"

namespace MyHttpd::MyHttp {
    namespace ReadUtils {
        constexpr auto matchDigit(char c) noexcept {
            return c >= '0' and c <= '9';
        }

        constexpr auto matchInteger(const std::string& s) noexcept {
            for (const auto c : s) {
                if (not matchDigit(c)) {
                    return false;
                }
            }

            return true;
        }
    };

    enum class ReadState {
        top,
        header,
        body,
        transfer_normal,
        // transfer_chunked,
        done,
        error
    };

    struct ReadStep {
        ReadState next;
        bool had_fatal_error;
    };

    class HttpIntake {
    private:
        static constexpr auto buffer_size = 1024UL;

        MySock::FixedBuffer<Meta::ASCIIOctet, buffer_size> m_buffer;
        std::istringstream m_header_stream;

        std::unordered_map<std::string, HeaderValue> m_temp_headers;
        std::string m_temp_uri;
        HttpMethod m_temp_method;
        HttpSchema m_temp_schema;

        ReadState m_state;

        
        [[nodiscard]] ReadStep stateTop(MySock::ClientSocket& sio_stream) noexcept;
        [[nodiscard]] ReadStep stateHeader(MySock::ClientSocket& sio_stream) noexcept;
        [[nodiscard]] ReadStep stateBody() noexcept;
        [[nodiscard]] ReadStep stateTransferNormal(MySock::ClientSocket& sio_stream) noexcept;
        // [[nodiscard]] ReaderMetaData stateTransferChunked(MySock::ClientSocket& sio_stream) noexcept;
        
        public:
        HttpIntake();
        
        [[nodiscard]] std::optional<Request> nextRequest(MySock::ClientSocket& socket) noexcept;
        void reset();
    };
}