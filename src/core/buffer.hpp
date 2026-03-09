#pragma once

#include "commandpool.hpp"
#include "commandbuffer.hpp"
#include "vulkan/vulkan.hpp"
#include <optional>
#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <vector>
#include <cstring>

class VulkanDevice;
class CommandPool;
class CommandBuffer;

struct BufferConfig {
    vk::BufferUsageFlags    usage;
    vk::MemoryPropertyFlags memProperties;
};

class Buffer {
public:
    Buffer(const VulkanDevice&, vk::DeviceSize size, const BufferConfig&);

    const vk::raii::Buffer& getVkHandle() const;
    const vk::raii::DeviceMemory& getMemory() const;

    template<typename T>
    static Buffer createBuffer(const VulkanDevice& device, const CommandPool& commandPool, BufferConfig& config, const std::vector<T>& vertices) {
        vk::DeviceSize bufferSize{sizeof(T) * vertices.size()};

        // Staging buffer
        BufferConfig stagingConfig{
            .usage         = vk::BufferUsageFlagBits::eTransferSrc,
            .memProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        };
        Buffer stagingBuffer{device, bufferSize, stagingConfig};
        stagingBuffer.transfer_data(vertices, bufferSize);

        // Vertex buffer
        config.usage |= vk::BufferUsageFlagBits::eTransferDst;
        Buffer vertexBuffer{device, bufferSize, config};
        vertexBuffer.copy(stagingBuffer, commandPool, bufferSize);

        return vertexBuffer;
    }

    static Buffer createUniformBuffer(const VulkanDevice& device, vk::DeviceSize size) {
        BufferConfig config{
            .usage         = vk::BufferUsageFlagBits::eUniformBuffer,
            .memProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
        };
        Buffer uniformBuffer{device, size, config};
        return uniformBuffer;
    }

    template<typename T>
    void update(const T& data) {
        static_assert(std::is_trivially_copyable_v<T>, "Buffer data must be trivially copyable"); // Check if can be safely copied with memcpy
        assert(mMemoryMapped.has_value() && "Buffer memory is not mapped");

        memcpy(*mMemoryMapped, &data, sizeof(T));
    }
    
private:
    static uint32_t findMemoryType(const vk::raii::PhysicalDevice& physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
    
    void copy(const Buffer&, const CommandPool&, vk::DeviceSize);

    template<typename T>
    void transfer_data(const std::vector<T>& vertices, vk::DeviceSize size) {
        static_assert(std::is_trivially_copyable_v<T>, "Buffer data must be trivially copyable"); // Check if can be safely copied with memcpy

        void* data{mMemory.mapMemory(0, size)};
        memcpy(data, vertices.data(), size);
        mMemory.unmapMemory();
    }

    const VulkanDevice& mVulkanDevice;

    vk::raii::Buffer       mBuffer{nullptr};
    vk::raii::DeviceMemory mMemory{nullptr};
    std::optional<void*>   mMemoryMapped; // for uniformbuffer
};
