#pragma once
#include "ChunkProtocol.hpp"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <functional>

namespace CustomEngine {

    /**
     * @brief حالة إعادة تجميع ملف قيد الاستقبال
     */
    struct ReassemblySession {
        uint32_t fileId = 0;
        uint32_t totalChunks = 0;
        uint32_t receivedCount = 0;
        size_t   totalPayloadBytes = 0;
        std::vector<bool> chunkBitmask;          // قناع بيت لتتبع الأجزاء المستلمة بدقة
        std::vector<std::vector<uint8_t>> parts; // تخزين مؤقت للشرائح
        bool isComplete = false;
        uint32_t calculatedFinalCRC = 0;
    };

    /**
     * @brief مسؤول إعادة تجميع الأجزاء دون تلف (Corruption-free Reassembly)
     * مطور ومحسن عن SplitReader الداخلي في FishNet
     */
    class SplitReader {
    public:
        SplitReader(size_t maxAllowedFileSize = MAX_SAFE_FILE_SIZE);

        // معالجة جزء قادم من الشبكة
        bool ProcessChunk(const uint8_t* rawPacketData, size_t packetSize, uint32_t& outFileId);

        // التحقق من اكتمال الملف
        bool IsFileComplete(uint32_t fileId) const;

        // الحصول على الملف المكتمل بعد دمجه بالكامل
        bool TryGetCompletedFile(uint32_t fileId, std::vector<uint8_t>& outFullData);

        // نسبة التقدم المئوية الحالية لتحميل ملف معين
        float GetProgress(uint32_t fileId) const;

        // مسح وإعادة تصفير جلسة ملف
        void ResetFileSession(uint32_t fileId);

    private:
        size_t m_maxAllowedFileSize;
        mutable std::mutex m_mutex;
        std::unordered_map<uint32_t, ReassemblySession> m_sessions;
    };
}
