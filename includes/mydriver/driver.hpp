#pragma once

#include <condition_variable>
#include "mysock/sockets.hpp"
#include "mydriver/task_queue.hpp"

namespace MyHttpd::MyDriver {
    struct ServerConfig {
        int workers;
    };

    class ServerDriver {
    public:
        ServerDriver(ServerConfig config);

        [[nodiscard]] bool runService(MySock::ServerSocket socket);

    private:
        MyDriver::TaskQueue m_tasks;
        std::mutex m_cv_mtx;
        std::condition_variable m_task_cv;
        int m_worker_n;
    };
}