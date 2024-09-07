//#include "ViewportVulkan.hpp"
//
//#include "Engine/Engine.hpp"
//#include "../SwapchainVulkan.hpp"
//#include "../VulkanUtilities.hpp"
//
//namespace Brisk 
//{
//	void ViewportVulkan::Init() {
//		SwapchainVulkan* swapchain = static_cast<SwapchainVulkan*>(Engine::s_Swapchain);
//		m_Images.resize(swapchain->GetImageCount());
//		m_ImageMemory.resize(swapchain->GetImageCount());
//
//		for (uint32_t i = 0; i < m_Images.size(); i++)
//		{
//			// Create the linear tiled destination image to copy to and to read the memory from
//			VkImageCreateInfo imageCreateCI{};
//			imageCreateCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//			imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
//			// Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
//			imageCreateCI.format = swapchain->GetFormat().format;
//			imageCreateCI.extent.width = swapchain->GetExtentWidth();
//			imageCreateCI.extent.height = swapchain->GetExtentHeight();
//			imageCreateCI.extent.depth = 1;
//			imageCreateCI.arrayLayers = 1;
//			imageCreateCI.mipLevels = 1;
//			imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//			imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
//			imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
//			imageCreateCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//			// Create the image
//			// VkImage dstImage;
//			vkCreateImage(GpuContextVulkan::s_GPUDevice->GetDevice(), &imageCreateCI, nullptr, &m_Images[i]);
//			// Create memory to back up the image
//			VkMemoryRequirements memRequirements;
//			VkMemoryAllocateInfo memAllocInfo{};
//			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//			// VkDeviceMemory dstImageMemory;
//			vkGetImageMemoryRequirements(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Images[i], &memRequirements);
//			memAllocInfo.allocationSize = memRequirements.size;
//			// Memory must be host visible to copy from
//			memAllocInfo.memoryTypeIndex = VulkanUtilities::FindMemoryType(GpuContextVulkan::s_GPUDevice->GetPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//			vkAllocateMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), &memAllocInfo, nullptr, &m_ImageMemory[i]);
//			vkBindImageMemory(GpuContextVulkan::s_GPUDevice->GetDevice(), m_Images[i], m_ImageMemory[i], 0);
//
//			CommandBufferVulkan* singleTimeCommand = new CommandBufferVulkan();
//			singleTimeCommand->Allocate(m_CommandPool);
//			singleTimeCommand->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
//
//			VulkanUtilities::InsertImageMemoryBarrier(
//				singleTimeCommand->Get(),
//				m_Images[i],
//				VK_ACCESS_TRANSFER_READ_BIT,
//				VK_ACCESS_MEMORY_READ_BIT,
//				VK_IMAGE_LAYOUT_UNDEFINED,
//				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//				VK_PIPELINE_STAGE_TRANSFER_BIT,
//				VK_PIPELINE_STAGE_TRANSFER_BIT,
//				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
//
//			singleTimeCommand->End();
//		}
//
//		for (size_t i = 0; i < m_Images.size(); i++) {
//			VkImageViewCreateInfo image_views_create_info{};
//			image_views_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//			image_views_create_info.image = m_Images[i];
//			image_views_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//			image_views_create_info.format = VK_FORMAT_B8G8R8A8_SRGB;
//			image_views_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//			image_views_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//			image_views_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//			image_views_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//			image_views_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//			image_views_create_info.subresourceRange.baseMipLevel = 0;
//			image_views_create_info.subresourceRange.levelCount = 1;
//			image_views_create_info.subresourceRange.baseArrayLayer = 0;
//			image_views_create_info.subresourceRange.layerCount = 1;
//
//			if (vkCreateImageView(GpuContextVulkan::s_GPUDevice->GetDevice(), &image_views_create_info, nullptr, &m_ImageViews[i]) != VK_SUCCESS) {
//				throw std::runtime_error("Failed to create Swapchain Image Views!");
//			}
//		}
//		{
//			VkAttachmentDescription colorAttachment{};
//			colorAttachment.format = swapchain->GetFormat().format;
//			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//			VkAttachmentDescription depthAttatchment{};
//			depthAttatchment.format = swapchain->GetDepthFormat();
//			depthAttatchment.samples = VK_SAMPLE_COUNT_1_BIT;
//			depthAttatchment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//			depthAttatchment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//			depthAttatchment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//			depthAttatchment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//			depthAttatchment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//			depthAttatchment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//			VkAttachmentReference colorAttachmentRef{};
//			colorAttachmentRef.attachment = 0;
//			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//			VkAttachmentReference depthAttachmentRef{};
//			depthAttachmentRef.attachment = 1;
//			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//			VkSubpassDescription subpass{};
//			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//			subpass.colorAttachmentCount = 1;
//			subpass.pColorAttachments = &colorAttachmentRef;
//			subpass.pDepthStencilAttachment = &depthAttachmentRef;
//
//			VkSubpassDependency dependency{};
//			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//			dependency.dstSubpass = 0;
//			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//			dependency.srcAccessMask = 0;
//			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//
//			std::vector<VkAttachmentDescription> colorAttachments = { colorAttachment, depthAttatchment };
//			m_RenderPass = new RenderPassVulkan();
//			m_RenderPass->Create(colorAttachments, { subpass }, { dependency });
//			for (int i = 0; i < swapchain->GetImageCount(); i++) {
//				std::vector<VkImageView> attachments = {
//					swapchain->GetSwapchainImageViews()[i],
//					swapchain->GetDepthImageView(),
//				};
//				m_RenderPass->CreateNAddFramebuffer(attachments, swapchain->GetExtentWidth(), swapchain->GetExtentHeight());
//			}
//		}
//
//		{
//			m_Pipeline = new GraphicsPipelineVulkan();
//
//			VkShaderModule vertexModule = VulkanUtilities::CreateShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleVS.spv");
//			VkShaderModule fragmentModule = VulkanUtilities::CreateShaderModule(GpuContextVulkan::s_GPUDevice->GetDevice(), "Shaders/Vulkan/Compiled/TriangleFS.spv");
//			m_Pipeline->CreateShaderStage(vertexModule, VK_SHADER_STAGE_VERTEX_BIT);
//			m_Pipeline->CreateShaderStage(fragmentModule, VK_SHADER_STAGE_FRAGMENT_BIT);
//
//			std::vector<GraphicsPipelineVulkan::Binding> bindings;
//			bindings.push_back({
//				0,
//				VK_VERTEX_INPUT_RATE_VERTEX,
//				sizeof(Point),
//				});
//			std::vector<GraphicsPipelineVulkan::AttributeDescription> attributes;
//			attributes.push_back({
//				0,
//				0,
//				VK_FORMAT_R32G32B32_SFLOAT,
//				offsetof(Point, Point::Position),
//				});
//			attributes.push_back({
//				0,
//				1,
//				VK_FORMAT_R32G32B32_SFLOAT,
//				offsetof(Point, Point::Color),
//				});
//			std::vector<VkVertexInputBindingDescription> bindingDescriptions;
//			bindingDescriptions.resize(bindings.size());
//			for (int i = 0; i < bindingDescriptions.size(); i++) {
//				bindingDescriptions[i].binding = bindings[i].BindingIndex;
//				bindingDescriptions[i].inputRate = bindings[i].InputRate;
//				bindingDescriptions[i].stride = bindings[i].Stride;
//			}
//			std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
//			attributeDescriptions.resize(attributes.size());
//			for (int i = 0; i < attributeDescriptions.size(); i++) {
//				attributeDescriptions[i].binding = attributes[i].BindingIndex;
//				attributeDescriptions[i].location = attributes[i].Location;
//				attributeDescriptions[i].format = attributes[i].Format;
//				attributeDescriptions[i].offset = attributes[i].Offset;
//			}
//			m_Pipeline->CreateVertexInputState(bindingDescriptions, attributeDescriptions);
//			m_Pipeline->CreateInputAssembly(false, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
//			m_Pipeline->CreateViewportState(1, 1);
//			m_Pipeline->CreateRasterizer(false, false, VK_POLYGON_MODE_FILL, 1.0f, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, false);
//			m_Pipeline->CreateMultiSampling(false, VK_SAMPLE_COUNT_1_BIT);
//			m_Pipeline->CreateDepthStencil(true, true, VK_COMPARE_OP_LESS, false, false);
//			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//			colorBlendAttachment.blendEnable = VK_FALSE;
//			std::vector< VkPipelineColorBlendAttachmentState> colorBlendAttachments = { colorBlendAttachment };
//			m_Pipeline->CrateColorBlending(colorBlendAttachments, false, VK_LOGIC_OP_COPY);
//			std::vector<VkDynamicState> dynamicStates = {
//				VK_DYNAMIC_STATE_VIEWPORT,
//				VK_DYNAMIC_STATE_SCISSOR
//			};
//			m_Pipeline->CreateDynamicState(dynamicStates);
//			m_Pipeline->CreatePipelineLayout(m_DescriptorSetLayouts, 0);
//			m_Pipeline->CreatePipeline(m_RenderPass->GetRenderPass());
//		}
//	}
//
//	void ViewportVulkan::Update() {
//
//	}
//
//	void ViewportVulkan::Destroy() {
//
//	}
//}