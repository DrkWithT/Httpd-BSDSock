#pragma once

#include <type_traits>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace MyHttpd::MyDriver {
    struct Task {
        int fd;
        bool poisoned;
    };

    class TaskQueue {
    public:
        TaskQueue() noexcept;

        [[nodiscard]] std::size_t getCount() noexcept;

        template <typename T> requires (std::is_same_v<std::remove_reference_t<T>, Task>)
        void addTask(T&& arg, std::condition_variable& signaling_cv) {
            {
                std::unique_lock<std::mutex> add_lock {m_mtx};

                m_items.push(arg);
            }

            signaling_cv.notify_one();
        }

        [[nodiscard]] Task getTask();

        void poisonAll(int worker_count, std::condition_variable& signaling_cv);

    private:
        std::mutex m_mtx;
        std::queue<Task> m_items;
    };
}