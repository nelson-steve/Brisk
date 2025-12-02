#include "JobSystem.hpp"
#include <stdexcept>
#include "Mutexes.hpp"

namespace Brisk {

    JobSystem::JobSystem(uint32_t threadsCount) {
        if (threadsCount == 0) threadsCount = 1;
        m_Threads.reserve(threadsCount);
        for (uint32_t i = 0; i < threadsCount; ++i) {
            m_Threads.emplace_back(&JobSystem::WorkerLoop, this);
        }
    }

    JobSystem::~JobSystem() {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Shutdown = true;
        }
        m_CondVar.notify_all();

        for (auto& t : m_Threads) {
            if (t.joinable())
                t.join();
        }
    }

    void JobSystem::WorkerLoop() {
        while (true) {
            std::function<void()> job;
            std::function<void()> callback;
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_CondVar.wait(lock, [this]() { return m_Shutdown || !m_JobQueue.empty(); });

                if (m_Shutdown && m_JobQueue.empty())
                    return;

                job = std::move(m_JobQueue.front()).func;
                callback = std::move(m_JobQueue.front()).mainThreadCallback;
                m_JobQueue.pop();
            }

            try {
                job();
                {
                    std::lock_guard<std::mutex> lock();
                    m_MainThreadCallbacks.push(callback);
                }
            }
            catch (...) {
            }
        }
    }

    void JobSystem::ExecuteMainThreadCallbacks() {
        std::queue<MainThreadCallback> local;

        {
            std::lock_guard<std::mutex> lock(g_MainThreadCallbackMutex);
            std::swap(local, m_MainThreadCallbacks);
        }

        while (!local.empty())
        {
            local.front()();  // execute callback
            local.pop();
        }
    }

}
