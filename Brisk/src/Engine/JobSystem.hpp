#pragma once

#include <cstdint>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <type_traits>
#include <utility>

namespace Brisk {

    using MainThreadCallback = std::function<void()>;

    struct Job {
        std::function<void()> func;
        MainThreadCallback mainThreadCallback = nullptr;
    };

    class JobSystem {
    public:
        explicit JobSystem(uint32_t threadsCount = std::thread::hardware_concurrency());
        ~JobSystem();

        template<typename F, typename... Args>
        auto AddJob(F&& f, MainThreadCallback callback, Args&&... args)
            -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
        {
            using result_t = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

            auto bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            auto taskPtr = std::make_shared<std::packaged_task<result_t()>>(std::move(bound));
            std::future<result_t> fut = taskPtr->get_future();

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                if (m_Shutdown) {
                    throw std::runtime_error("AddJob on stopped JobSystem");
                }

                m_JobQueue.emplace([taskPtr]() { (*taskPtr)(); }, callback);
            }

            m_CondVar.notify_one();
            return fut;
        }

        void ExecuteMainThreadCallbacks();

    private:
        void WorkerLoop();

        std::queue<MainThreadCallback> m_MainThreadCallbacks;
        std::vector<std::thread> m_Threads;
        std::queue<Job> m_JobQueue;
        std::mutex m_QueueMutex;
        std::condition_variable m_CondVar;
        bool m_Shutdown = false;
    };

}
