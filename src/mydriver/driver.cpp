#include <print>
#include <iostream>
#include <utility>
#include <thread>
#include <vector>
#include "mydriver/entry_job.hpp"
#include "mydriver/worker_job.hpp"
#include "mydriver/driver.hpp"

namespace MyHttpd::MyDriver {
    constexpr std::string_view server_name = "myhttpd/0.1-dev";
    constexpr auto min_worker_n = 1;

    ServerDriver::ServerDriver(ServerConfig config)
    : m_tasks {}, m_cv_mtx {}, m_task_cv {}, m_worker_n {(config.workers >= min_worker_n) ? config.workers : min_worker_n } {}

    bool ServerDriver::runService(MySock::ServerSocket socket) {
        if (not socket.isReady()) {
            return false;
        }

        MyDriver::EntryJob entry {std::move(socket), m_worker_n};
        std::vector<std::thread> worker_thrds;

        std::thread entry_thrd {[&entry, this]() {
            std::print("[{} LOG]: started producer...\n", server_name);

            entry(m_tasks, m_task_cv);

            std::print("[{} LOG]: stopped producer.\n", server_name);
        }};

        for (auto worker_i = 0; worker_i < m_worker_n; worker_i++) {
            worker_thrds.emplace_back([worker_i, this]() {
                std::print("[{} LOG]: starting worker {}...\n", server_name, worker_i);

                MyDriver::WorkerJob worker {worker_i, server_name};
                worker(m_tasks, m_task_cv, m_cv_mtx);

                std::print("[{} LOG]: worker {} done.\n", server_name, worker_i);
            });
        }

        std::thread user_thrd {[&]() {
            char choice;

            do {
                std::print("Enter 'y' to stop the server:\n");

                std::cin >> choice;
            } while (choice != 'y');

            std::print("Stopping, please wait.\n");
            entry.shutdown();
        }};

        entry_thrd.join();

        for (auto& thrd : worker_thrds) {
            thrd.join();
        }

        user_thrd.join();

        return true;
    }
}