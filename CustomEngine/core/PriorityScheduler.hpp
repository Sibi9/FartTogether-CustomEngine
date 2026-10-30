#pragma once
#include "TokenBucketRateLimiter.hpp"
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <unordered_map>

namespace CustomEngine {

    enum class TaskPriority {
        High = 0,       // مدخلات وتزامن حرج (Real-time Ticks / Input)
        Normal = 1,     // رسائل عادية
        Background = 2  // بث أجزاء الملفات (File Chunks & Asset Streaming)
    };

    struct ScheduledTask {
        uint64_t taskId;
        TaskPriority priority;
        uint32_t fileId; // معرف الملف المرتبط بالـ Rate Limiter إن وجد
        size_t byteWeight;
        std::function<void()> execute;

        bool operator<(const ScheduledTask& other) const {
            // الأولوية الأعلى تسبق في الطابور
            return static_cast<int>(priority) > static_cast<int>(other.priority);
        }
    };

    /**
     * @brief مجدول مهام متعدد الأولويات ومتوافق مع توقيت الشبكة
     * مستوحى من PriorityQueue و TickTimer المستخرجين من اللعبة الأصلية
     */
    class PriorityScheduler {
    public:
        PriorityScheduler();
        ~PriorityScheduler();

        // إضافة مهمة جديدة مع تحديد أولويتها
        void EnqueueTask(TaskPriority priority, uint32_t fileId, size_t byteWeight, std::function<void()> task);

        // تخصيص أو جلب محدد سرعة لملف معين
        std::shared_ptr<TokenBucketRateLimiter> GetOrCreateRateLimiter(uint32_t fileId, int64_t initialLimit = 0);

        // تعيين سرعة ملف محدد بالبايت في الثانية
        void SetFileSpeedLimit(uint32_t fileId, int64_t bytesPerSec);

        // معالجة دفعة من المهام المجدولة في دورة التحديث (Tick Step)
        void ProcessTick(size_t maxTasksToProcess = 32);

        void Stop();

    private:
        std::priority_queue<ScheduledTask> m_taskQueue;
        std::unordered_map<uint32_t, std::shared_ptr<TokenBucketRateLimiter>> m_limiters;
        mutable std::mutex m_mutex;
        uint64_t m_nextTaskId = 1;
        bool m_running = true;
    };
}
