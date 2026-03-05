#include "graphics_pipeline.hpp"

#include "device.hpp"
#include "shader.hpp"
#include "swapchain.hpp"

#include "vulkan/vulkan.hpp"

#include <vector>
#include <vulkan/vulkan_raii.hpp>

GraphicsPipeline::GraphicsPipeline(const VulkanDevice& device, Shader& shader, Swapchain& swapchain) 
    : mVulkanDevice{device}
    , mShader{shader} 
    , mSwapchain{swapchain}
{
    // Vertex Input
    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;

    // Input Assembly
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo {
        .topology = vk::PrimitiveTopology::eTriangleList
    };

    // Viewport + scissor
    // Set viewport and scissor as dynamic
    std::vector dynamicState = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };
    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo {
        .dynamicStateCount = static_cast<uint32_t>(dynamicState.size()),
        .pDynamicStates    = dynamicState.data()
    };
    vk::PipelineViewportStateCreateInfo viewportCreateInfo {
        .viewportCount = 1,
        .scissorCount  = 1
    };

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo {
        .depthClampEnable        = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode             = vk::PolygonMode::eFill,
        .cullMode                = vk::CullModeFlagBits::eBack,
        .frontFace               = vk::FrontFace::eClockwise,
        .depthBiasEnable         = vk::False,
        .depthBiasSlopeFactor    = 1.0f,
        .lineWidth               = 1.0f
    };

    // Multisampling
    vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable  = vk::False
    };

    // Depth & stencil

    // Color blending
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable = vk::False,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };
    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False, 
        .logicOp = vk::LogicOp::eCopy, 
        .attachmentCount = 1, 
        .pAttachments = &colorBlendAttachment
    };

    // Pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo {
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    mPipelineLayout = vk::raii::PipelineLayout(mVulkanDevice.getVkHandle(), pipelineLayoutCreateInfo);

    // Pipeline rendering
    // Move data from the vector to the array:
    vk::PipelineShaderStageCreateInfo shaderStages[mShader.getShaderStages().size()]; 
    std::move(mShader.getShaderStages().begin(), mShader.getShaderStages().end(), shaderStages);
    
    vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain {
        {
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputCreateInfo,
            .pInputAssemblyState = &inputAssemblyCreateInfo,
            .pViewportState = &viewportCreateInfo,
            .pRasterizationState = &rasterizerCreateInfo,
            .pMultisampleState = &multisampleCreateInfo,
            // .pDepthStencilState
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicStateCreateInfo,
            .layout = mPipelineLayout,
            .renderPass = nullptr
        },
        {
            .colorAttachmentCount = 1, 
            .pColorAttachmentFormats = &mSwapchain.getFormat()
        }
    };

    mGraphicsPipeline = vk::raii::Pipeline(mVulkanDevice.getVkHandle(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

const vk::raii::Pipeline& GraphicsPipeline::getVkHandle() const {
    return mGraphicsPipeline;
}
