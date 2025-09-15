#pragma once

#include "CommandBuffer.hpp"
#include "Texture.hpp"
#include "RHI.hpp"

#include <vector>
#include <memory>

namespace Brisk 
{
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
		DontCare
	};

	enum class StoreOp {
		Store,
		DontCare
	};

	struct RenderPassAttachment {
		uint32_t pBinding;
		AttachmentType pAttachmentType;
		std::shared_ptr<Texture> pImage;
		LoadOp pLoadOp;
		StoreOp pStoreOp;
		Core::ImageLayout pInitialLayout;
		Core::ImageLayout pFinalLayout;
	};

	class RenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
	public:
		virtual void Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& outputs) = 0;
		virtual void Release() = 0;

		virtual void Begin(std::shared_ptr<CommandBuffer> cmd, uint32_t imageIndex = 0) = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

		int GetClearCount() const { return m_ClearCount; }
		int GetColorAttachmentCount() const { return m_ColorAttachmentCount; }
		bool HasDepth() const { return m_HasDepth; }

		static std::shared_ptr<RenderPass> Create();

	protected:
		bool m_HasDepth;
		int m_ClearCount = 0;
		int m_ColorAttachmentCount = 0;
	};
};
