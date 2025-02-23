#include <utility>
#include "mydriver/worker_job.hpp"

namespace MyHttpd::MyDriver {
    constexpr auto dud_task_fd = -1;
    constexpr auto default_connection_timeout = 10L;
    constexpr std::string_view dud_content = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body><p>Hello World!</p></body></html>";

    WorkerJob::WorkerJob(int wid, std::string_view server_name)
    : m_intake {}, m_outtake {}, m_server_name {server_name}, m_connection {}, m_wid {wid}, m_state {WorkerState::take_task}, m_conn_persist_flag {PersistFlag::unknown}, m_diagnosis {RequestDiagnosis::ok} {}

    void WorkerJob::operator()(TaskQueue& tasks, std::condition_variable& task_cv, std::mutex& cv_mtx) {
        Utilities::GMTGen date_gen;
        MyHttp::Request temp_req;
        MyHttp::Response temp_res;

        while (m_state != WorkerState::halt) {
            switch (m_state) {
            case WorkerState::take_task:
                stateTakeTask(tasks, task_cv, cv_mtx);
                break;
            case WorkerState::request:
                temp_req = stateRequest();
                break;
            case WorkerState::validate:
                stateValidate(temp_req);
                break;
            case WorkerState::handle_good:
                temp_res = stateHandleGood(temp_req, date_gen);
                break;
            case WorkerState::handle_bad:
                temp_res = stateHandleBad(temp_req, date_gen);
                break;
            case WorkerState::reply:
                stateReply(temp_res);
                break;
            case WorkerState::reset:
                stateReset();
                break;
            case WorkerState::error:
                break;
            case WorkerState::halt:
            default:
                break;
            }
        }
    }


    void WorkerJob::transitionAnyway(WorkerState next) noexcept {
        m_state = next;
    }

    WorkerState WorkerJob::transitionWith(WorkerState state, PersistFlag persist) const noexcept {
        if (state == WorkerState::take_task) {
            return WorkerState::request;
        } else if (state == WorkerState::request) {
            return WorkerState::validate;
        } else if (state == WorkerState::validate) {
            return WorkerState::handle_good;
        } else if (state == WorkerState::handle_good) {
            return WorkerState::reply;
        } else if (state == WorkerState::handle_bad) {
            return WorkerState::reply;
        } else if (state == WorkerState::reply) {
            return (persist == PersistFlag::yes) ? WorkerState::request : WorkerState::reset;
        } else if (state == WorkerState::reset) {
            return WorkerState::take_task;
        }

        return WorkerState::take_task;
    }

    void WorkerJob::stateTakeTask(TaskQueue& tasks, std::condition_variable& task_cv, std::mutex& cv_mtx) {
        Task temp;

        {
            std::unique_lock cv_lock {cv_mtx};

            task_cv.wait(cv_lock, [&]() {
                return tasks.getCount() > 0;
            });

            temp = tasks.getTask();
        }

        auto [temp_fd, temp_poisoned] = temp;

        if (temp_poisoned) {
            transitionAnyway(WorkerState::halt);
        } else if (temp_fd == dud_task_fd) {
            transitionAnyway(WorkerState::take_task);
        } else {
            m_connection = {temp_fd, default_connection_timeout};
            transitionAnyway(WorkerState::request);
        }
    }

    MyHttp::Request WorkerJob::stateRequest() {
        m_intake.reset();

        auto maybe_req = m_intake.nextRequest(m_connection);

        if (not maybe_req.has_value()) {
            transitionAnyway(WorkerState::error);
            return {};
        }

        transitionAnyway(WorkerState::validate);

        return maybe_req.value();
    }

    void WorkerJob::stateValidate(const MyHttp::Request& temp) {
        const auto connection_closable = (temp.headers.contains("Connection"))
            ? (std::get<std::string>(temp.headers.at("Connection")) == "close")
            : temp.schema == MyHttp::HttpSchema::http_1_0;

        m_conn_persist_flag = (connection_closable)
            ? PersistFlag::no
            : PersistFlag::yes;

        if (temp.schema == MyHttp::HttpSchema::http_unknown or temp.method == MyHttp::HttpMethod::h1_nop) {
            m_diagnosis = RequestDiagnosis::malformed_top_line;
            transitionAnyway(WorkerState::handle_bad);
            return;
        }

        if (temp.schema == MyHttp::HttpSchema::http_1_1 and not temp.headers.contains("Host")) {
            /// TODO: check Host: xyz header against the server's hostname!
            m_diagnosis = RequestDiagnosis::missing_header;
            transitionAnyway(WorkerState::handle_bad);
            return;
        }

        transitionAnyway(WorkerState::handle_good);
    }

    MyHttp::Response WorkerJob::stateHandleGood(const MyHttp::Request& temp, Utilities::GMTGen& gmt_utility) {
        if (temp.uri != "/" or temp.method != MyHttp::HttpMethod::h1_get) {
            m_diagnosis = RequestDiagnosis::has_invalid_uri;
            transitionAnyway(WorkerState::handle_bad);
            return {};
        }

        transitionAnyway(WorkerState::reply);

        std::unordered_map<std::string, MyHttp::HeaderValue> headers {
            {"Server", m_server_name.data()},
            {"Content-Type", "text/html"},
            {"Content-Length", static_cast<int>(dud_content.length())},
            {"Date", gmt_utility()}
        };

        if (m_conn_persist_flag == PersistFlag::no) {
            headers["Connection"] = "close";
        }

        return {
            MyHttp::HttpStatus::ok,
            temp.schema,
            MyHttp::stringifyToMsg(MyHttp::HttpStatus::ok),
            {dud_content},
            std::move(headers)
        };
    }

    MyHttp::Response WorkerJob::stateHandleBad(const MyHttp::Request& temp, Utilities::GMTGen& gmt_utility) {
        MyHttp::HttpStatus status_code;
        std::string_view status_msg;

        if (m_diagnosis == RequestDiagnosis::malformed_top_line) {
            status_code = MyHttp::HttpStatus::bad_request;
        } else if (m_diagnosis == RequestDiagnosis::missing_header) {
            status_code = MyHttp::HttpStatus::bad_request;
        } else if (m_diagnosis == RequestDiagnosis::has_invalid_uri) {
            status_code = MyHttp::HttpStatus::not_found;
        } else {
            status_code = MyHttp::HttpStatus::server_error;
        }

        status_msg = MyHttp::stringifyToMsg(status_code);
        const char* connect_directive = (m_conn_persist_flag == PersistFlag::yes)
            ? "keep-alive"
            : "close";

        transitionAnyway(WorkerState::reply);

        return {
            .status = status_code,
            .schema = temp.schema,
            .msg = status_msg,
            .blob = {},
            .headers = {
                {"Server", m_server_name.data()},
                {"Connection", connect_directive},
                {"Content-Type", "*/*"},
                {"Content-Length", 0},
                {"Date", gmt_utility()}
            }
        };
    }

    void WorkerJob::stateReply(const MyHttp::Response& temp) {
        if (not m_outtake.sendMessage(temp, m_connection)) {
            transitionAnyway(WorkerState::error);
            return;
        }

        m_state = transitionWith(m_state, m_conn_persist_flag);
    }

    void WorkerJob::stateReset() {
        m_connection = {};
        m_conn_persist_flag = PersistFlag::unknown;

        transitionAnyway(WorkerState::take_task);
    }

    void WorkerJob::stateError() {
        transitionAnyway(WorkerState::take_task);
    }
}