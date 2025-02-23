#include "mydriver/task_queue.hpp"

namespace MyHttpd::MyDriver {
    constexpr auto empty_count = 0UL;
    constexpr auto task_dud_fd = -1;

    TaskQueue::TaskQueue() noexcept
    : m_mtx {}, m_items {} {}

    std::size_t TaskQueue::getCount() noexcept {
        std::unique_lock<std::mutex> read_count_lock {m_mtx};
        return m_items.size(); 
    }

    Task TaskQueue::getTask() {
        std::unique_lock<std::mutex> pop_lock {m_mtx};

        if (m_items.size() == empty_count) {
            return Task {
                .fd = task_dud_fd,
                .poisoned = false
            };
        }

        auto temp = m_items.front();
        m_items.pop();

        return temp;
    }

    void TaskQueue::poisonAll(int worker_count, std::condition_variable& signaling_cv) {
        {
            std::lock_guard<std::mutex> m_poison_lock {m_mtx};

            for (auto poison_it = 0; poison_it < worker_count; poison_it++) {
                m_items.push(Task {
                    .fd = task_dud_fd,
                    .poisoned = true
                });
            }
        }

        signaling_cv.notify_all();
    }
}