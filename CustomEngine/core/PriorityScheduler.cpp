#include "PriorityScheduler.hpp"
#include <thread>

namespace CustomEngine {

    PriorityScheduler::PriorityScheduler() = default;

    PriorityScheduler::~PriorityScheduler() {
        Stop();
    }

    void PriorityScheduler::Stop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = false;
    }

    std::shared_ptr<TokenBucketRateLimiter> PriorityScheduler::GetOrCreateRateLimiter(uint32_t fileId, int64_t initialLimit) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_limiters.find(fileId);
        if (it != m_limiters.end()) {
            return it->second;
        }
        auto limiter = std::make_shared<TokenBucketRateLimiter>(initialLimit);
        m_limiters[fileId] = limiter;
        return limiter;
    }

    void PriorityScheduler::SetFileSpeedLimit(uint32_t fileId, int64_t bytesPerSec) {
        auto limiter = GetOrCreateRateLimiter(fileId, bytesPerSec);
        limiter->SetSpeedLimit(bytesPerSec);
    }

    void PriorityScheduler::EnqueueTask(TaskPriority priority, uint32_t fileId, size_t byteWeight, std::function<void()> task) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running) return;

        ScheduledTask scheduled;
        scheduled.taskId = m_nextTaskId++;
        scheduled.priority = priority;
        scheduled.fileId = fileId;
        scheduled.byteWeight = byteWeight;
        scheduled.execute = std::move(task);

        m_taskQueue.push(std::move(scheduled));
    }

    void PriorityScheduler::ProcessTick(size_t maxTasksToProcess) {
        std::vector<ScheduledTask> tasksToRun;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            size_t processed = 0;
            std::vector<ScheduledTask> deferredTasks;

            while (!m_taskQueue.empty() && processed < maxTasksToProcess) {
                ScheduledTask current = m_taskQueue.top();
                m_taskQueue.pop();

                // إذا كانت مهمة بث جزء ملف خاضعة لسقف سرعة (Rate Limited)
                if (current.fileId > 0 && current.byteWeight > 0) {
                    auto limiterIt = m_limiters.find(current.fileId);
                    if (limiterIt != m_limiters.end()) {
                        if (!limiterIt->second->TryConsume(current.byteWeight)) {
                            // لم يتوفر رصيد توكنز كافٍ حالياً، نؤجل المهمة للدورة القادمة
                            deferredTasks.push_back(std::move(current));
                            continue;
                        }
                    }
                }

                tasksToRun.push_back(std::move(current));
                processed++;
            }

            // إعادة المهام المؤجلة للطابور
            for (auto& def : deferredTasks) {
                m_taskQueue.push(std::move(def));
            }
        }

        // تنفيذ المهام خارج القفل لتفادي تجميد الخيوط (Deadlock prevention)
        for (auto& t : tasksToRun) {
            if (t.execute) {
                t.execute();
            }
        }
    }
}
