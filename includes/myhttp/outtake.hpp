#pragma once

#include "meta/helpers.hpp"
#include "mysock/buffers.hpp"
#include "mysock/sockets.hpp"
#include "myhttp/types.hpp"

namespace MyHttpd::MyHttp {
    class HttpOuttake {
    private:
        static constexpr auto buffer_size = 1024UL;
        MySock::FixedBuffer<Meta::ASCIIOctet, buffer_size> m_buffer;

        [[nodiscard]] bool serializeTop(HttpSchema schema, HttpStatus status) noexcept;
        [[nodiscard]] bool serializeEmptyBreak() noexcept;
        [[nodiscard]] bool serializeHeaderInfo(const std::string& key, const HeaderValue& value) noexcept;
        [[nodiscard]] bool serializeBlob(const DynamicBlob<Meta::ASCIIOctet>& blob) noexcept;

    public:
        HttpOuttake();

        [[nodiscard]] bool sendMessage(const Response& reply, MySock::ClientSocket& sio_out);
    };
}