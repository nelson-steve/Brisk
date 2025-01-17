// INCLUDES
#include "FramebufferVulkan.hpp"
#include "RenderPassVulkan.hpp"
#include "GpuAdapterVulkan.hpp"
//-----------------------------
#include <Volk/volk.h>
//--------------------
#include <cassert>
//----------------

namespace Brisk 
{
    void FramebufferVulkan::Init(const FramebufferSpecs& specs) {
        assert(specs.pWidth > 0);
        assert(specs.pHeight > 0);
        assert(specs.pLayers > 0);
        //assert(specs.pAttachments.size() > 0);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = std::static_pointer_cast<RenderPassVulkan>(specs.pRenderPass)->GetRenderPass();
        //framebufferInfo.attachmentCount = static_cast<uint32_t>(specs.pAttachments.size());
        //framebufferInfo.pAttachments = specs.pAttachments.data();
        framebufferInfo.width = specs.pWidth;
        framebufferInfo.height = specs.pHeight;
        framebufferInfo.layers = specs.pLayers;

        if (vkCreateFramebuffer(std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer) != VK_SUCCESS) {
            //throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void FramebufferVulkan::Destroy() {

    }

    void FramebufferVulkan::Bind() {

    }
}