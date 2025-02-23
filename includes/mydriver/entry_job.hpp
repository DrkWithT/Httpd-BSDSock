#pragma once

#include "mysock/sockets.hpp"
#include "mydriver/task_queue.hpp"

namespace MyHttpd::MyDriver {
    class EntryJob {
    public:
        EntryJob() = delete;
        EntryJob(MySock::ServerSocket entry_socket, int worker_n) noexcept;

        void operator()(TaskQueue& tasks, std::condition_variable& task_cv) noexcept;

        void shutdown() noexcept;

    private:
        MySock::ServerSocket m_entry;
        int m_worker_n;
        std::atomic_flag m_continue_flag;
    };
}