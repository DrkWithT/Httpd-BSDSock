#pragma once

#include <atomic>
#include "mysock/sockets.hpp"
#include "myhttp/types.hpp"
#include "myhttp/intake.hpp"
#include "myhttp/outtake.hpp"
#include "utilities/mycaching.hpp"

namespace MyHttpd::MyDriver {
    enum class WorkState : unsigned char {
        connect,  // accept client connection
        request,  // try reading HTTP request
        validate, // check HTTP request for semantic / sizing errors
        handle_good,   // prepare HTTP OK reply
        handle_bad,    // prepare HTTP <err> reply
        reply,    // try sending reply
        reset,    // close client connection
        error,    // handle error and go to quit
        halt      // quit
    };

    enum class Persistence : unsigned char {
        yes,
        no,
        unknown
    };

    enum class RequestCheck {
        ok,                  // no validation errors
        malformed_top_line,  // status 400
        missing_header,      // status 400 
        has_invalid_uri,     // URI has no handler
        has_other_error      // any other processing error
    };

    enum class ExitCode : unsigned char {
        ok,
        bad_setup,
        bad_sio,
        bad_general,
        last = bad_general
    };

    [[nodiscard]] std::string_view fetchExitCodeMsg(ExitCode code) noexcept;

    class ServerDriver {
    public:
        ServerDriver(MySock::ServerSocket socket);

        [[nodiscard]] ExitCode runService();

    private:
        void transitionAnyway(WorkState next) noexcept;
        [[nodiscard]] WorkState transitionWith(WorkState state, Persistence persist) const noexcept;

        void stateConnect();
        [[nodiscard]] MyHttp::Request stateRequest();
        void stateValidate(const MyHttp::Request& temp);
        [[nodiscard]] MyHttp::Response stateHandleGood(const MyHttp::Request& temp, Utilities::GMTGen& gmt_utility);
        [[nodiscard]] MyHttp::Response stateHandleBad(const MyHttp::Request& temp, Utilities::GMTGen& gmt_utility);
        void stateReply(const MyHttp::Response& temp);
        void stateReset();
        void stateError();
        void stateHalt();

        MyHttp::HttpIntake m_intake;
        MyHttp::HttpOuttake m_outtake;
        MySock::ServerSocket m_entry;
        MySock::ClientSocket m_connection;
        WorkState m_state;
        Persistence m_conn_persist_flag;
        RequestCheck m_check;
        ExitCode m_exit_code;
        std::atomic_flag m_no_quit;
    };
}