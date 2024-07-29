#pragma once

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
    class RenderpassVulkan {
    public:
        RenderpassVulkan();
        ~RenderpassVulkan();

        void Create(std::vector<VkFramebuffer> framebuffers = {});
        void Release();

        void AddAttachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialLayout, VkImageLayout finalLayout);
        void AddSubpass(VkPipelineBindPoint pipelineBindPoint, const std::vector<VkAttachmentReference>& colorAttachments, const VkAttachmentReference* depthStencilAttachment = nullptr);
        void CreateRenderPass();
        //void CreateFramebuffer(VkExtent2D extent, const std::vector<VkImageView>& attachments);
        void CreateFramebuffer(VkExtent2D extent, const std::vector<VkImageView>& attachments);

        VkRenderPass GetRenderPass() const;
        VkFramebuffer GetFramebuffer() const;

    private:
        VkRenderPass m_RenderPass;
        std::vector<VkFramebuffer> m_Framebuffers;

        std::vector<VkAttachmentDescription> m_Attachments;
        std::vector<VkSubpassDescription> m_Subpasses;
        std::vector<VkSubpassDependency> m_Dependencies;
    };
}