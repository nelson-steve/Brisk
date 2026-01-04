#pragma once

#include "CommandBuffer.hpp"
#include "Texture.hpp"
#include "RHI.hpp"

#include <vector>
#include <memory>

namespace Brisk 
{
	class Framebuffer;

	enum class AttachmentType {
		Color,
		Depth,
		Input,
		Resolve,
		Preserve,
		Swapchain
	};

	struct RenderPassDependency {
		int32_t srcSubpass = -1; // -1 -> external subpass
		int32_t dstSubpass = -1; // -1 -> external subpass
		Core::AccessType srcAccess = Core::AccessType::None;
		Core::AccessType dstAccess = Core::AccessType::None;
		Core::PipelineStage srcStage = Core::PipelineStage::None;
		Core::PipelineStage dstStage = Core::PipelineStage::None;
	};

	enum class LoadOp {
		Clear,
		Load,
		DontCare,
		None
	};

	enum class StoreOp {
		Store,
		DontCare,
		None
	};

	struct RenderPassAttachment {
		uint32_t p_Binding;
		AttachmentType p_AttachmentType;
		LoadOp p_LoadOp;
		StoreOp p_StoreOp;
		Core::Format p_Format;
		Core::ImageLayout p_InitialLayout;
		Core::ImageLayout p_FinalLayout;
	};

	class RenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
	public:
		virtual void Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& outputs) = 0;
		virtual void Release() = 0;

		virtual void Begin(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Framebuffer> framebuffer) = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

		//int GetClearCount() const { return m_ClearCount; }
		int GetColorAttachmentCount() const { return m_ColorAttachmentCount; }
		//bool HasDepth() const { return m_HasDepth; }

		static std::shared_ptr<RenderPass> Create();

	protected:
		bool m_HasDepth;
		int m_ColorAttachmentCount = 0;
	};
};
