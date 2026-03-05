#pragma once

// #define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Instance {
public:
    Instance();

    const vk::raii::Instance& getVkHandle() const;
private:
    vk::raii::Context  mContext;
    vk::raii::Instance mInstance{nullptr};
};
