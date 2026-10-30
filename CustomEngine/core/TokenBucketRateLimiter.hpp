#pragma once
#include <chrono>
#include <mutex>
#include <cstdint>
#include <algorithm>

namespace CustomEngine {

    /**
     * @brief منظم معدل تدفق البيانات بخوارزمية Token Bucket
     * يتيح تحديد ومراقبة سرعة التدفق لكل ملف على حدة بشكل يدوي وديناميكي
     */
    class TokenBucketRateLimiter {
    public:
        TokenBucketRateLimiter(int64_t bytesPerSecond = 0); // 0 تعني غير محدود (Unlimited)

        // تعيين سقف السرعة بالبايت في الثانية (مثلاً 512 * 1024 لنصف ميجابايت/ثانية)
        void SetSpeedLimit(int64_t bytesPerSecond);
        int64_t GetSpeedLimit() const;

        // التحقق واستهلاك كمية من البايتات مع إرجاع وقت الانتظار اللازم إن وجد
        bool TryConsume(size_t bytes);
        
        // حساب مدة التأخير المطلوبة بالمللي ثانية للامتثال لسقف السرعة
        int64_t CalculateRequiredDelayMs(size_t bytes);

        // خصم التوكنز بعد الإرسال الفعلي
        void Consume(size_t bytes);

    private:
        void RefillTokens();

        int64_t m_bytesPerSecond;       // السرعة المستهدفة
        double  m_currentTokens;        // الرصيد الحالي من البايتات المتاحة
        int64_t m_maxBurst;             // السعة اللحظية القصوى (Burst Capacity)
        std::chrono::steady_clock::time_point m_lastRefillTime;
        mutable std::mutex m_mutex;
    };
}
