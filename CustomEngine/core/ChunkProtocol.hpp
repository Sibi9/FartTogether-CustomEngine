#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace CustomEngine {

    // المعرفات البرمجية لأنواع الحزم - مطابقة ومطورة عن بروتوكول FishNet
    enum class PacketType : uint8_t {
        UnreliableInput = 0x01,     // مدخلات اللاعب اللحظية (سريعة ومنخفضة التأخير)
        StateSync       = 0x02,     // مزامنة حالة اللعبة والحركة
        MultiPartChunk  = 0x03,     // أجزاء الملفات المقسمة (Multi-part Chunks)
        ChunkAck        = 0x04,     // تأكيد استلام الجزء
        StreamEnd       = 0x05      // إشارة اكتمال البث
    };

    // معايير النقل والحد الأقصى للتقسيم
    constexpr size_t DEFAULT_MTU = 1200;                // الحد الأقصى للجزء لتفادي تجزئة IP
    constexpr size_t MAX_SAFE_FILE_SIZE = 50 * 1024 * 1024; // حد الأمان 50 ميجابايت لمنع نفاد الذاكرة
    constexpr uint32_t PROTOCOL_MAGIC = 0x50554646;    // "PUFF" توقيع الحزمة للتحقق من الصحة

    // ترويسة الجزء الواحد (Chunk Header)
    #pragma pack(push, 1)
    struct ChunkHeader {
        uint32_t magic;         // توقيع البروتوكول للتأكد من سلامة الحزمة
        uint8_t  packetType;    // نوع الحزمة (MultiPartChunk)
        uint32_t fileId;        // المعرف الفريد للملف الذي ينتمي إليه هذا الجزء
        uint32_t totalChunks;   // إجمالي عدد الأجزاء الكلي للملف
        uint32_t chunkIndex;    // رقم وفهرس الجزء الحالي (يبدأ من 0)
        uint32_t payloadSize;   // حجم بايتات البيانات الفعلية في هذا الجزء
        uint32_t checksum;      // مجموع التحقق (CRC32) لمحتوى الجزء لضمان عدم التلف
    };
    #pragma pack(pop)

    // بنية تمثيل الجزء في الذاكرة
    struct ChunkPacket {
        ChunkHeader header;
        std::vector<uint8_t> payload;
    };

    // خوارزمية سريعة وخفيفة لحساب المجموع الاختباري (CRC32) مناسبة لمعالجات الأجهزة الضعيفة
    inline uint32_t CalculateCRC32(const uint8_t* data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < length; ++i) {
            crc ^= data[i];
            for (int j = 0; j < 8; ++j) {
                crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
            }
        }
        return ~crc;
    }
}
