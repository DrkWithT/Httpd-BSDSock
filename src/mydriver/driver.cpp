#include <iostream>
#include <print>
#include <array>
#include <utility>
#include <thread>
#include "mydriver/driver.hpp"

namespace MyHttpd::MyDriver {
    static constexpr auto custom_timeout = 10L;
    static constexpr const char* server_name = "myhttpd/0.1-dev";
    static constexpr std::string_view dud_content {"<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body><p>Hello World!</p></body></html>"};

    static constexpr std::array<std::string_view, static_cast<std::size_t>(ExitCode::last) + 1> exit_msgs = {
        "OK!",
        "Bad socket setup!",
        "I/O error!",
        "Server logic error!"
    };

    std::string_view fetchExitCodeMsg(ExitCode code) noexcept {
        const auto exit_n = static_cast<int>(code);
        return exit_msgs[exit_n];
    }

    ServerDriver::ServerDriver(MySock::ServerSocket socket)
    : m_intake {}, m_outtake {}, m_entry {std::move(socket)}, m_connection {}, m_state {WorkState::connect}, m_conn_persist_flag {Persistence::unknown}, m_check {RequestCheck::ok}, m_exit_code {ExitCode::ok}, m_no_quit {true} {}

    ExitCode ServerDriver::runService() {
        if (not m_entry.isReady()) {
            return ExitCode::bad_setup;
        }

        auto prompt_stop = [this]() {
            char choice;

            std::cout << "Enter 'y' to quit.\n";

            do {
                std::cin >> choice;
            } while (choice != 'y');

            std::cout << "Stopping, please wait...\n";

            m_no_quit.clear();
        };

        std::thread prompt_thrd {prompt_stop};

        Utilities::GMTGen date_gen;
        MyHttp::Request temp_req;
        MyHttp::Response temp_res;

        while (m_no_quit.test() and m_state != WorkState::halt) {
            switch (m_state) {
            case WorkState::connect:
                stateConnect();
                break;
            case WorkState::request:
                temp_req = stateRequest();
                break;
            case WorkState::validate:
                stateValidate(temp_req);
                break;
            case WorkState::handle_good:
                temp_res = stateHandleGood(temp_req, date_gen);
                break;
            case WorkState::handle_bad:
                temp_res = stateHandleBad(temp_req, date_gen);
                break;
            case WorkState::reply:
                stateReply(temp_res);
                break;
            case WorkState::reset:
                stateReset();
                break;
            case WorkState::error:
                stateError();
                break;
            case WorkState::halt:
            default:
                break;
            }
        }

        prompt_thrd.join();

        return m_exit_code;
    }

    void ServerDriver::transitionAnyway(WorkState next) noexcept {
        m_state = next;
    }

    WorkState ServerDriver::transitionWith(WorkState state, Persistence persist) const noexcept {
        if (state == WorkState::connect) {
            return WorkState::request;
        } else if (state == WorkState::request) {
            return WorkState::validate;
        } else if (state == WorkState::validate) {
            return WorkState::handle_good;
        } else if (state == WorkState::handle_good) {
            return WorkState::reply;
        } else if (state == WorkState::handle_bad) {
            return WorkState::reply;
        } else if (state == WorkState::reply) {
            return (persist == Persistence::yes) ? WorkState::request : WorkState::reset;
        } else if (state == WorkState::reset) {
            return WorkState::request;
        } else if (state == WorkState::error) {
            return WorkState::halt;
        } else {
            return WorkState::halt;
        }
    }

    void ServerDriver::stateConnect() {
        auto incoming = m_entry.acceptConnection();

        if (not incoming.has_value()) {
            transitionAnyway(WorkState::error);
            return;
        }

        m_connection = {incoming.value(), custom_timeout};
        m_state = transitionWith(m_state, m_conn_persist_flag);
    }

    MyHttp::Request ServerDriver::stateRequest() {
        auto maybe_req = m_intake.nextRequest(m_connection);

        if (not maybe_req.has_value()) {
            transitionAnyway(WorkState::error);
            return {};
        }

        return maybe_req.value();
    }

    void ServerDriver::stateValidate(const MyHttp::Request& temp) {
        const auto connection_closable = (temp.headers.contains("Connection"))
            ? (std::get<std::string>(temp.headers.at("Connection")) == "close")
            : temp.schema != MyHttp::HttpSchema::http_1_1;

        m_conn_persist_flag = (connection_closable)
            ? Persistence::no
            : Persistence::yes;

        if (temp.schema == MyHttp::HttpSchema::http_unknown or temp.method == MyHttp::HttpMethod::h1_nop) {
            m_check = RequestCheck::malformed_top_line;
            transitionAnyway(WorkState::handle_bad);
            return;
        }

        if (temp.schema == MyHttp::HttpSchema::http_1_1 and not temp.headers.contains("Host")) {
            /// TODO: check Host: xyz header against the server's hostname!
            m_check = RequestCheck::missing_header;
            transitionAnyway(WorkState::handle_bad);
            return;
        }

        transitionAnyway(WorkState::handle_good);
    }

    MyHttp::Response ServerDriver::stateHandleGood(const MyHttp::Request& temp, Utilities::GMTGen& gmt_utility) {
        if (temp.uri != "/") {
            m_check = RequestCheck::has_invalid_uri;
            transitionAnyway(WorkState::handle_bad);
            return {};
        }

        return {
            .status = MyHttp::HttpStatus::ok,
            .schema = temp.schema,
            .msg = MyHttp::stringifyToMsg(MyHttp::HttpStatus::ok),
            .blob = dud_content,
            .headers = {
                {"Server", server_name},
                {"Content-Length", static_cast<int>(dud_content.length())},
                {"Date", gmt_utility()}
            }
        };
    }

    MyHttp::Response ServerDriver::stateHandleBad(const MyHttp::Request& temp, Utilities::GMTGen& gmt_utility) {
        MyHttp::HttpStatus status_code;
        std::string_view status_msg;
        
        if (m_check == RequestCheck::malformed_top_line) {
            status_code = MyHttp::HttpStatus::bad_request;
        } else if (m_check == RequestCheck::missing_header) {
            status_code = MyHttp::HttpStatus::bad_request;
        } else if (m_check == RequestCheck::has_invalid_uri) {
            status_code = MyHttp::HttpStatus::not_found;
        } else {
            status_code = MyHttp::HttpStatus::server_error;
        }

        status_msg = MyHttp::stringifyToMsg(status_code);

        return {
            .status = status_code,
            .schema = temp.schema,
            .msg = status_msg,
            .blob = {},
            .headers = {
                {"Server", server_name},
                {"Content-Length", 0},
                {"Date", gmt_utility()}
            }
        };
    }

    void ServerDriver::stateReply(const MyHttp::Response& temp) {
        if (not m_outtake.sendMessage(temp, m_connection)) {
            transitionAnyway(WorkState::error);
            return;
        }

        m_state = transitionWith(m_state, m_conn_persist_flag);
    }

    void ServerDriver::stateReset() {
        m_connection = {};
        m_conn_persist_flag = Persistence::unknown;

        m_state = transitionWith(m_state, m_conn_persist_flag);
    }

    void ServerDriver::stateError() {
        std::print(std::cerr, "\033[1;31m[myhttpd ERROR]: {}\033[0m", "Internal server error of a networking I/O failure.\n");

        m_state = transitionWith(m_state, Persistence::no);
    }

    void ServerDriver::stateHalt() {
        std::print(std::cout, "[myhttpd LOG]: Reached halt state, stopping.\n");
    }

}