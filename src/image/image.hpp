#pragma once

#include "commandbuffer.hpp"
#include "image_loader.hpp"
#include "sampler.hpp"

#include <vulkan/vulkan_raii.hpp>

#include <optional>

class VulkanDevice;
class CommandPool;
class Sampler;

struct ImageConfig {
    const ImageLoader&      image;
    vk::Format              format = vk::Format::eR8G8B8A8Srgb;
    vk::ImageUsageFlags     usage  = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
    vk::ImageTiling         tiling = vk::ImageTiling::eOptimal;
    vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    vk::ImageAspectFlags    aspect = vk::ImageAspectFlagBits::eColor;
};

class Image {
public:
    Image(const VulkanDevice&, const CommandPool&, const ImageConfig&);

    const vk::raii::Image& getImage() const;
    const vk::raii::ImageView& getImageView() const;
    const Sampler& getSampler() const;

private:
    void transitionImageLayout(SingleTimeCommandBuffer& commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

    const VulkanDevice& mVulkanDevice;
    const CommandPool&  mCommandPool;

    vk::raii::Image        mImage{nullptr};
    vk::raii::DeviceMemory mImageMemory{nullptr};
    vk::raii::ImageView    mImageView{nullptr};
    std::optional<Sampler> mSampler;
};
