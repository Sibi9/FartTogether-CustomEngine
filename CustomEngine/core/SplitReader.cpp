#include "SplitReader.hpp"
#include <cstring>
#include <iostream>

namespace CustomEngine {

    SplitReader::SplitReader(size_t maxAllowedFileSize)
        : m_maxAllowedFileSize(maxAllowedFileSize)
    {
    }

    bool SplitReader::ProcessChunk(const uint8_t* rawPacketData, size_t packetSize, uint32_t& outFileId) {
        if (!rawPacketData || packetSize < sizeof(ChunkHeader)) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_mutex);

        // 1. قراءة الترويسة
        ChunkHeader header;
        std::memcpy(&header, rawPacketData, sizeof(ChunkHeader));
        outFileId = header.fileId;

        // 2. التحقق من صحة التوقيع ونوع الحزمة
        if (header.magic != PROTOCOL_MAGIC || header.packetType != static_cast<uint8_t>(PacketType::MultiPartChunk)) {
            return false;
        }

        // 3. التحقق من حجم محتوى الجزء ومطابقته للترويسة
        size_t expectedTotalSize = sizeof(ChunkHeader) + header.payloadSize;
        if (packetSize < expectedTotalSize) {
            return false; // حزمة مبتورة
        }

        const uint8_t* payloadPtr = rawPacketData + sizeof(ChunkHeader);

        // 4. فحص المجموع الاختباري (CRC32) لضمان عدم حدوث تلف أثناء النقل
        uint32_t computedCRC = CalculateCRC32(payloadPtr, header.payloadSize);
        if (computedCRC != header.checksum) {
            std::cerr << "[SplitReader] Checksum mismatch! Corrupted chunk dropped for File: " << header.fileId << std::endl;
            return false;
        }

        // 5. فحص أمان السعة الإجمالية القصوى
        if (header.totalChunks * header.payloadSize > m_maxAllowedFileSize) {
            std::cerr << "[SplitReader] Security rejection: File exceeds maximum allowed quota!" << std::endl;
            return false;
        }

        // 6. تهيئة أو جلب جلسة إعادة التجميع
        auto& session = m_sessions[header.fileId];
        if (session.totalChunks == 0) {
            session.fileId = header.fileId;
            session.totalChunks = header.totalChunks;
            session.chunkBitmask.assign(header.totalChunks, false);
            session.parts.resize(header.totalChunks);
            session.receivedCount = 0;
            session.totalPayloadBytes = 0;
            session.isComplete = false;
        }

        // التحقق من صحة الفهرس وعدم تكرار استلام نفس الجزء
        if (header.chunkIndex >= session.totalChunks) {
            return false;
        }

        if (!session.chunkBitmask[header.chunkIndex]) {
            session.chunkBitmask[header.chunkIndex] = true;
            session.parts[header.chunkIndex].assign(payloadPtr, payloadPtr + header.payloadSize);
            session.receivedCount++;
            session.totalPayloadBytes += header.payloadSize;

            // التحقق الذري من اكتمال كافة الأجزاء
            if (session.receivedCount == session.totalChunks) {
                session.isComplete = true;
            }
        }

        return true;
    }

    bool SplitReader::IsFileComplete(uint32_t fileId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_sessions.find(fileId);
        if (it != m_sessions.end()) {
            return it->second.isComplete;
        }
        return false;
    }

    bool SplitReader::TryGetCompletedFile(uint32_t fileId, std::vector<uint8_t>& outFullData) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_sessions.find(fileId);
        if (it == m_sessions.end() || !it->second.isComplete) {
            return false;
        }

        const auto& session = it->second;
        outFullData.clear();
        outFullData.reserve(session.totalPayloadBytes);

        // دمج الأجزاء المتسلسلة دون أي تلف في مصفوفة واحدة مستمرة
        for (uint32_t i = 0; i < session.totalChunks; ++i) {
            const auto& part = session.parts[i];
            outFullData.insert(outFullData.end(), part.begin(), part.end());
        }

        return true;
    }

    float SplitReader::GetProgress(uint32_t fileId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_sessions.find(fileId);
        if (it != m_sessions.end() && it->second.totalChunks > 0) {
            return static_cast<float>(it->second.receivedCount) / static_cast<float>(it->second.totalChunks);
        }
        return 0.0f;
    }

    void SplitReader::ResetFileSession(uint32_t fileId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_sessions.erase(fileId);
    }
}
