#include "TokenBucketRateLimiter.hpp"
#include <thread>

namespace CustomEngine {

    TokenBucketRateLimiter::TokenBucketRateLimiter(int64_t bytesPerSecond)
        : m_bytesPerSecond(bytesPerSecond),
          m_currentTokens(static_cast<double>(bytesPerSecond)),
          m_maxBurst(std::max<int64_t>(bytesPerSecond, 65536)), // سعة انفجارية افتراضية 64 كيلوبايت
          m_lastRefillTime(std::chrono::steady_clock::now())
    {
    }

    void TokenBucketRateLimiter::SetSpeedLimit(int64_t bytesPerSecond) {
        std::lock_guard<std::mutex> lock(m_mutex);
        RefillTokens();
        m_bytesPerSecond = bytesPerSecond;
        if (m_bytesPerSecond <= 0) {
            m_currentTokens = 1e9; // رصيد ضخم في حال إلغاء التحديد
            m_maxBurst = 1e9;
        } else {
            m_maxBurst = std::max<int64_t>(bytesPerSecond, 65536);
            m_currentTokens = std::min(m_currentTokens, static_cast<double>(m_maxBurst));
        }
    }

    int64_t TokenBucketRateLimiter::GetSpeedLimit() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_bytesPerSecond;
    }

    void TokenBucketRateLimiter::RefillTokens() {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - m_lastRefillTime;
        m_lastRefillTime = now;

        if (m_bytesPerSecond > 0) {
            m_currentTokens += elapsed.count() * m_bytesPerSecond;
            if (m_currentTokens > m_maxBurst) {
                m_currentTokens = static_cast<double>(m_maxBurst);
            }
        }
    }

    bool TokenBucketRateLimiter::TryConsume(size_t bytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_bytesPerSecond <= 0) return true; // غير محدود

        RefillTokens();
        if (m_currentTokens >= static_cast<double>(bytes)) {
            m_currentTokens -= static_cast<double>(bytes);
            return true;
        }
        return false;
    }

    int64_t TokenBucketRateLimiter::CalculateRequiredDelayMs(size_t bytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_bytesPerSecond <= 0) return 0;

        RefillTokens();
        if (m_currentTokens >= static_cast<double>(bytes)) {
            return 0;
        }

        double needed = static_cast<double>(bytes) - m_currentTokens;
        double secondsToWait = needed / m_bytesPerSecond;
        return static_cast<int64_t>(secondsToWait * 1000.0) + 1;
    }

    void TokenBucketRateLimiter::Consume(size_t bytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_bytesPerSecond > 0) {
            RefillTokens();
            m_currentTokens -= static_cast<double>(bytes);
        }
    }
}
