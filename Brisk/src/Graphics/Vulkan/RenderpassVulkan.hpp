#pragma once

#include <Volk/volk.h>

#include <vector>

namespace Brisk {
    class RenderPassManager {
    public:
        RenderPassManager(VkDevice device);
        ~RenderPassManager();

        void addAttachment(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialLayout, VkImageLayout finalLayout);
        void addSubpass(VkPipelineBindPoint pipelineBindPoint, const std::vector<VkAttachmentReference>& colorAttachments, const VkAttachmentReference* depthStencilAttachment = nullptr);
        void createRenderPass();
        void createFramebuffer(VkExtent2D extent, const std::vector<VkImageView>& attachments);

        VkRenderPass getRenderPass() const;
        VkFramebuffer getFramebuffer() const;

    private:
        VkDevice device;
        VkRenderPass renderPass;
        VkFramebuffer framebuffer;

        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkSubpassDescription> subpasses;
        std::vector<VkSubpassDependency> dependencies;
    };
}