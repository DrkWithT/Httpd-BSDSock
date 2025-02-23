#include <utility>
#include "mydriver/entry_job.hpp"

namespace MyHttpd::MyDriver {
    EntryJob::EntryJob(MySock::ServerSocket entry_socket, int worker_n) noexcept
    : m_entry {std::move(entry_socket)}, m_worker_n {worker_n}, m_continue_flag {true} {}

    void EntryJob::operator()(TaskQueue& tasks, std::condition_variable& task_cv) noexcept {
        while (m_continue_flag.test()) {
            auto incoming_opt = m_entry.acceptConnection();

            if (not incoming_opt.has_value()) {
                continue;
            }

            Task connection_task {
                .fd = incoming_opt.value(),
                .poisoned = false
            };

            tasks.addTask(connection_task, task_cv);
        }

        tasks.poisonAll(m_worker_n, task_cv);
    }

    void EntryJob::shutdown() noexcept {
        m_continue_flag.clear();
        m_entry = {};
    }
}