#pragma once
#include "ChunkProtocol.hpp"
#include "ChunkSlicer.hpp"
#include "SplitReader.hpp"
#include "PriorityScheduler.hpp"
#include "TokenBucketRateLimiter.hpp"
#include <memory>
#include <string>
#include <iostream>

namespace CustomEngine {

    /**
     * @brief المحرك المخصص المتكامل (Custom Cross-Platform Core)
     * يجمع بين منطق جدولة الشبكة، والتقطيع الآمن، ومحدد السرعة، وإدارة المدخلات
     */
    class EngineCore {
    public:
        EngineCore();
        ~EngineCore();

        void Initialize();

        // بدء ملف جديد للتقسيم والبث
        bool QueueFileForStreaming(uint32_t fileId, const std::string& fileName, const std::vector<uint8_t>& data, int64_t speedLimitBytesPerSec = 0);

        // تعديل سقف السرعة لملف معين بشكل يدوي ومباشر (من الواجهة)
        void SetFileSpeedLimit(uint32_t fileId, int64_t bytesPerSec);

        // محاكاة استلام جزء من الشبكة
        bool ReceiveNetworkChunk(const uint8_t* data, size_t size, uint32_t& outFileId);

        // فحص نسبة التقدم
        float GetFileProgress(uint32_t fileId) const;
        bool IsFileComplete(uint32_t fileId) const;

        // الحصول على الملف بعد إعادة تجميعه
        bool GetCompletedFile(uint32_t fileId, std::vector<uint8_t>& outData);

        // دورة التحديث الرئيسية (Tick Step) المتزامنة
        void UpdateTick();

        // إرسال المدخلات الفورية عالية الأولوية (مطابقة لـ PirateInput)
        void SendPlayerInput(float moveX, float moveY, bool normalGas, bool mousePuff, uint32_t tick);

        PriorityScheduler& GetScheduler() { return m_scheduler; }
        SplitReader& GetSplitReader() { return m_splitReader; }

    private:
        PriorityScheduler m_scheduler;
        SplitReader m_splitReader;
        std::unordered_map<uint32_t, std::shared_ptr<ChunkSlicer>> m_activeSlicers;
        uint32_t m_currentTick = 0;
    };
}
