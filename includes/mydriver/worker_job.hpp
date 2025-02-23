#pragma once

#include "mydriver/task_queue.hpp"
#include "mysock/sockets.hpp"
#include "myhttp/types.hpp"
#include "myhttp/intake.hpp"
#include "myhttp/outtake.hpp"
#include "utilities/mycaching.hpp"

namespace MyHttpd::MyDriver {
    enum class WorkerState : unsigned char {
        take_task,
        request,
        validate,
        handle_good,
        handle_bad,
        reply,
        reset,
        error,
        halt
    };

    enum class PersistFlag : unsigned char {
        yes,
        no,
        unknown
    };

    enum class RequestDiagnosis : unsigned char {
        ok,                  // no validation errors
        malformed_top_line,  // status 400
        missing_header,      // status 400 
        has_invalid_uri,     // URI has no handler
        has_other_error      // any other processing error
    };

    class WorkerJob {
    public:
        WorkerJob() = delete;

        /// @note Only pass a terminated C-string literal through `server_name`!
        WorkerJob(int wid, std::string_view server_name);

        void operator()(TaskQueue& tasks, std::condition_variable& task_cv, std::mutex& cv_mtx);

    private:
        void transitionAnyway(WorkerState next) noexcept;
        [[nodiscard]] WorkerState transitionWith(WorkerState state, PersistFlag persist) const noexcept;

        void stateTakeTask(TaskQueue& tasks, std::condition_variable& task_cv, std::mutex& cv_mtx);
        [[nodiscard]] MyHttp::Request stateRequest();
        void stateValidate(const MyHttp::Request& temp);
        [[nodiscard]] MyHttp::Response stateHandleGood(const MyHttp::Request& temp, Utilities::GMTGen& gmt_utility);
        [[nodiscard]] MyHttp::Response stateHandleBad(const MyHttp::Request& temp, Utilities::GMTGen& gmt_utility);
        void stateReply(const MyHttp::Response& temp);
        void stateReset();
        void stateError();

        MyHttp::HttpIntake m_intake;
        MyHttp::HttpOuttake m_outtake;
        std::string_view m_server_name;
        MySock::ClientSocket m_connection;
        int m_wid;
        WorkerState m_state;
        PersistFlag m_conn_persist_flag;
        RequestDiagnosis m_diagnosis;
    };
}