#pragma once
#include "ChunkProtocol.hpp"
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace CustomEngine {

    /**
     * @brief مسؤول تقسيم الملفات الكبيرة إلى حزم مقسمة (Multi-part Slicing)
     * مماثل ومطور عن منطق PacketBundle و SendSplitMessage في المحرك الأصلي
     */
    class ChunkSlicer {
    public:
        ChunkSlicer(uint32_t fileId, size_t chunkSize = DEFAULT_MTU - sizeof(ChunkHeader));

        // تجزئة مصفوفة بايتات إلى أجزاء
        bool SliceBuffer(const uint8_t* data, size_t totalSize);
        bool SliceVector(const std::vector<uint8_t>& data);

        // عدد الأجزاء الكلي
        uint32_t GetTotalChunks() const { return m_totalChunks; }
        uint32_t GetFileId() const { return m_fileId; }
        size_t GetTotalBytes() const { return m_totalBytes; }

        // استخراج الجزء حسب الفهرس
        const ChunkPacket& GetChunk(uint32_t index) const;

        // تجهيز البايتات الجاهزة للإرسال عبر الشبكة (Header + Payload)
        std::vector<uint8_t> SerializeChunk(uint32_t index) const;

    private:
        uint32_t m_fileId;
        size_t   m_chunkPayloadLimit;
        uint32_t m_totalChunks;
        size_t   m_totalBytes;
        std::vector<ChunkPacket> m_chunks;
    };
}
