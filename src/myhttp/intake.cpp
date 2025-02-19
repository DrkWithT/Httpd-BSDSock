#include <string>
#include <utility>
#include "myhttp/intake.hpp"
#include "myhttp/types.hpp"
#include "mysock/sockets.hpp"

/// TODO: add more robust logic to default Content-Length to 0 in its absence.

namespace MyHttpd::MyHttp {
    static constexpr auto http_colon = ':';
    static constexpr auto http_lf = '\n';

    ReadStep HttpIntake::stateTop(MySock::ClientSocket& sio_stream) noexcept {
        m_buffer.reset();

        const auto read_status = sio_stream.readLine(m_buffer, http_lf);
    
        if (read_status != MySock::SockIOStatus::ok) {
            return {
                .next = ReadState::error,
                .had_fatal_error = true
            };
        }

        m_header_stream = std::istringstream {m_buffer.getPtr()};

        std::string raw_method;
        m_header_stream >> raw_method;
        std::string raw_uri;
        m_header_stream >> raw_uri;
        std::string raw_schema;
        m_header_stream >> raw_schema;

        m_temp_method = enumify({raw_method.cbegin(), raw_method.cend()}, MethodOpt {});
        m_temp_uri = std::move(raw_uri);
        m_temp_schema = enumify({raw_schema.cbegin(), raw_schema.cend()}, SchemaOpt {});

        return {
            .next = ReadState::header,
            .had_fatal_error = false
        };
    }

    ReadStep HttpIntake::stateHeader(MySock::ClientSocket& sio_stream) noexcept {
        m_buffer.reset();

        const auto read_status = sio_stream.readLine(m_buffer, http_lf);

        if (read_status != MySock::SockIOStatus::ok) {
            return {
                .next = ReadState::error,
                .had_fatal_error = true
            };
        }

        if (m_buffer.getLength() == 0) {
            return {
                .next = ReadState::body,
                .had_fatal_error = false
            };
        }

        m_header_stream = std::istringstream {m_buffer.getPtr()};
        std::string raw_key;
        std::string raw_value;

        std::getline(m_header_stream, raw_key, http_colon);
        m_header_stream >> raw_value;

        if (ReadUtils::matchInteger(raw_value)) {
            m_temp_headers[raw_key] = std::stoi(raw_value);
        } else {
            m_temp_headers[raw_key] = std::move(raw_value);
        }

        return {
            .next = ReadState::header,
            .had_fatal_error = false
        };
    }

    ReadStep HttpIntake::stateBody() noexcept {
        const auto& content_length_val = m_temp_headers.at("Content-Length");
        const auto content_length = std::get<int>(content_length_val);

        if (content_length == 0) {
            return {
                .next = ReadState::done,
                .had_fatal_error = false
            };
        } else if (content_length < 0) {
            return {
                .next = ReadState::error,
                .had_fatal_error = true
            };
        }

        return {
            .next = ReadState::transfer_normal,
            .had_fatal_error = false
        };
    }

    ReadStep HttpIntake::stateTransferNormal(MySock::ClientSocket& sio_stream) noexcept {
        m_buffer.reset();

        const auto& content_length_val = m_temp_headers.at("Content-Length");
        const auto content_length = std::get<int>(content_length_val);

        const auto read_status = sio_stream.readBlob(m_buffer, content_length);

        if (read_status != MySock::SockIOStatus::ok) {
            return {
                .next = ReadState::error,
                .had_fatal_error = true
            };
        }

        return {
            .next = ReadState::done,
            .had_fatal_error = false
        };
    }

    HttpIntake::HttpIntake()
    : m_buffer {}, m_header_stream {}, m_temp_headers {}, m_temp_uri {}, m_temp_method {}, m_temp_schema {}, m_state {ReadState::top} {
        reset();
    }

    void HttpIntake::reset() {
        m_state = ReadState::top;
        m_temp_headers.clear();
        m_temp_headers["Content-Type"] = "*/*";
        m_temp_headers["Content-Length"] = 0;
        m_temp_uri.clear();
        m_temp_method = MyHttp::HttpMethod::h1_nop;
        m_temp_schema = MyHttp::HttpSchema::http_unknown;
    }

    std::optional<Request> HttpIntake::nextRequest(MySock::ClientSocket& socket) noexcept {
        ReadStep step;
        bool had_fatal_error = false;

        while (m_state != ReadState::done) {
            switch (m_state) {
            case ReadState::top:
                step = stateTop(socket);
                had_fatal_error = step.had_fatal_error;
                m_state = (not had_fatal_error)
                    ? step.next : ReadState::error ;
                break;
            case ReadState::header:
                step = stateHeader(socket);
                had_fatal_error = step.had_fatal_error;
                m_state = (not had_fatal_error)
                    ? step.next : ReadState::error ;
                break;
            case ReadState::body:
                step = stateBody();
                had_fatal_error = step.had_fatal_error;
                m_state = (not had_fatal_error)
                    ? step.next : ReadState::error ;
                break;
            case ReadState::transfer_normal:
                step = stateTransferNormal(socket);
                had_fatal_error = step.had_fatal_error;
                m_state = (not had_fatal_error)
                    ? step.next : ReadState::error ;
                break;
            case ReadState::done:
                break;
            case ReadState::error:
            default:
                if (had_fatal_error) {
                    return {};
                }

                had_fatal_error = false;
                m_state = ReadState::done;
            }
        }

        /// @note The buffer should only have the body content remaining as a buffer-view because the reading will end at the payload.
        return {
            Request {
                m_temp_method,
                m_temp_schema,
                m_temp_uri,
                m_buffer.makeView(0, m_buffer.getLength()),
                m_temp_headers
            }
        };
    }
}