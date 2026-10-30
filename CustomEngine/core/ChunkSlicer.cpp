#include "ChunkSlicer.hpp"
#include <cstring>
#include <stdexcept>
#include <algorithm>

namespace CustomEngine {

    ChunkSlicer::ChunkSlicer(uint32_t fileId, size_t chunkSize)
        : m_fileId(fileId),
          m_chunkPayloadLimit(chunkSize),
          m_totalChunks(0),
          m_totalBytes(0)
    {
        if (m_chunkPayloadLimit == 0) {
            m_chunkPayloadLimit = DEFAULT_MTU - sizeof(ChunkHeader);
        }
    }

    bool ChunkSlicer::SliceBuffer(const uint8_t* data, size_t totalSize) {
        if (!data || totalSize == 0) return false;

        m_totalBytes = totalSize;
        m_totalChunks = static_cast<uint32_t>((totalSize + m_chunkPayloadLimit - 1) / m_chunkPayloadLimit);
        m_chunks.clear();
        m_chunks.reserve(m_totalChunks);

        for (uint32_t i = 0; i < m_totalChunks; ++i) {
            size_t offset = i * m_chunkPayloadLimit;
            size_t currentChunkSize = std::min(m_chunkPayloadLimit, totalSize - offset);

            ChunkPacket packet;
            packet.header.magic = PROTOCOL_MAGIC;
            packet.header.packetType = static_cast<uint8_t>(PacketType::MultiPartChunk);
            packet.header.fileId = m_fileId;
            packet.header.totalChunks = m_totalChunks;
            packet.header.chunkIndex = i;
            packet.header.payloadSize = static_cast<uint32_t>(currentChunkSize);
            packet.header.checksum = CalculateCRC32(data + offset, currentChunkSize);

            packet.payload.assign(data + offset, data + offset + currentChunkSize);
            m_chunks.push_back(std::move(packet));
        }

        return true;
    }

    bool ChunkSlicer::SliceVector(const std::vector<uint8_t>& data) {
        return SliceBuffer(data.data(), data.size());
    }

    const ChunkPacket& ChunkSlicer::GetChunk(uint32_t index) const {
        if (index >= m_chunks.size()) {
            throw std::out_of_range("Chunk index out of bounds");
        }
        return m_chunks[index];
    }

    std::vector<uint8_t> ChunkSlicer::SerializeChunk(uint32_t index) const {
        const auto& chunk = GetChunk(index);
        std::vector<uint8_t> buffer(sizeof(ChunkHeader) + chunk.payload.size());
        
        // نسخ الترويسة أولاً
        std::memcpy(buffer.data(), &chunk.header, sizeof(ChunkHeader));
        // نسخ محتوى الجزء
        std::memcpy(buffer.data() + sizeof(ChunkHeader), chunk.payload.data(), chunk.payload.size());
        
        return buffer;
    }
}
