#pragma once

#include "Texture.hpp"

#include <vector>
#include <memory>

namespace Brisk 
{
	enum class AccessType {
		None,
		ColorAttachmentWrite,
		DepthStencilWrite,
		ShaderRead,
		ShaderWrite,
		TransferSrc,
		TransferDst,
		Present
	};

	enum class PipelineStage {
		None,
		VertexShader,
		FragmentShader,
		ComputeShader,
		ColorAttachmentOutput,
		EarlyFragmentTests,
		LateFragmentTests,
		Transfer,
		BottomOfPipe,
	};

	enum class AttachmentType {
		Color,
		Depth,
		Input,
		Resolve,
		Preserve
	};

	struct RenderPassDependency {
		bool srcExternalPass = true;
		AccessType srcAccess = AccessType::None;
		AccessType dstAccess = AccessType::None;
		PipelineStage srcStage = PipelineStage::None;
		PipelineStage dstStage = PipelineStage::None;
	};


	struct RenderPassAttachment {
		uint32_t pBinding;
		AttachmentType pAttachmentType;
		std::shared_ptr<Texture> pImage;
	};

	class RenderPass {
		DEFINE_BASE_CLASS_CONSTRUCTOR(RenderPass)
	public:
		virtual void Init(const std::vector<RenderPassDependency>& dependencies, const std::vector<RenderPassAttachment>& outputs) = 0;

		virtual void Begin(std::shared_ptr<CommandBuffer> cmd) = 0;
		virtual void End(std::shared_ptr<CommandBuffer> cmd) = 0;

		int GetClearCount() const { return m_ClearCount; }
		int GetColorAttachmentCount() const { return m_ColorAttachmentCount; }

		static std::shared_ptr<RenderPass> Create();

	protected:
		bool m_HasDepth;
		int m_ClearCount = 0;
		int m_ColorAttachmentCount = 0;
	};
};
