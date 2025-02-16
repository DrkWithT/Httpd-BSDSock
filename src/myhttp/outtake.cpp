#include <format>
#include "myhttp/types.hpp"
#include "myhttp/outtake.hpp"

namespace MyHttpd::MyHttp {
    static constexpr auto header_int_v = 0;
    static constexpr auto buffer_sendable_n = 512UL;

    bool HttpOuttake::serializeTop(HttpSchema schema, HttpStatus status) noexcept {
        /// format info from Request object e.g HTTP/x.x 200 OK
        auto res_schema_name = stringifyEnum(schema);
        auto res_status_code = stringifyEnum(status);
        auto res_status_msg = stringifyToMsg(status);

        auto top_line = std::format("{} {} {}\r\n", res_schema_name, res_status_code, res_status_msg);
        const auto expected_length = top_line.size() + m_buffer.getLength();

        if (expected_length > m_buffer.getLimit()) {
            return false;
        }

        std::copy(top_line.cbegin(), top_line.cend(), m_buffer.getPtr() + m_buffer.getLength());
        m_buffer.markLength(expected_length);

        return true;
    }

    [[nodiscard]] bool HttpOuttake::serializeHeaderInfo(const std::string& key, const HeaderValue& value) noexcept {
        std::string temp_line;

        if (const auto value_index = value.index(); value_index == header_int_v) {
            const auto header_int = std::get<int>(value);
            temp_line = std::format("{}: {}\r\n", key, header_int);
        } else {
            const auto header_str = std::get<std::string>(value);
            temp_line = std::format("{} : {}\r\n", key, header_str);
        }

        const auto expected_length = temp_line.size() + m_buffer.getLength();

        if (expected_length > m_buffer.getLimit()) {
            return false;
        }

        std::copy(temp_line.cbegin(), temp_line.cend(), m_buffer.getPtr());
        m_buffer.markLength(expected_length);

        return true;
    }

    [[nodiscard]] bool HttpOuttake::serializeBlob(const DynamicBlob<Meta::ASCIIOctet>& blob) noexcept {
        m_buffer.reset();

        const auto blob_size = blob.getLength();

        if (blob_size > m_buffer.getLimit()) {
            return false;
        }

        std::copy(blob.getReadingPtr(), blob.getReadingPtr() + blob_size, m_buffer.getPtr());
        m_buffer.markLength(blob_size);

        return true;
    }

    HttpOuttake::HttpOuttake()
    : m_buffer {} {}

    bool HttpOuttake::sendMessage(const Response& reply, MySock::ClientSocket& sio_out) {
        m_buffer.reset();

        if (const auto top_load_ok = serializeTop(reply.schema, reply.status); not top_load_ok) {
            return false;
        }

        auto written_yet = false;

        // serialize these: all the headers, finally the body
        for (const auto& [key, value] : reply.headers) {
            if (not serializeHeaderInfo(key, value)) {
                m_buffer.reset();
                return false;
            }

            if (m_buffer.getLength() >= buffer_sendable_n) {
                if (sio_out.writeBlob(m_buffer, m_buffer.getLength()) != MySock::SockIOStatus::ok) {
                    m_buffer.reset();
                    return false;
                } else {
                    m_buffer.reset();
                    written_yet = true;
                }
            } else {
                written_yet = false;
            }
        }

        if (not written_yet) {
            if (sio_out.writeBlob(m_buffer, m_buffer.getLength()) != MySock::SockIOStatus::ok) {
                m_buffer.reset();
                return false;
            }
        }

        return true;
    }
}