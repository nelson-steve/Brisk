#pragma once

#include "CommandBuffer.hpp"
#include "Texture.hpp"
#include "RHI.hpp"

#include <vector>
#include <memory>

namespace Brisk 
{
	//enum class AccessType : uint32_t {
	//	None = 0,
	//	ColorAttachmentWrite = 1 << 0,
	//	DepthStencilWrite = 1 << 1,
	//	ShaderRead = 1 << 2,
	//	ShaderWrite = 1 << 3,
	//	TransferSrc = 1 << 4,
	//	TransferDst = 1 << 5,
	//	Present = 1 << 6
	//};

	//enum class PipelineStage : uint32_t {
	//	None = 0,
	//	VertexShader = 1 << 0,
	//	FragmentShader = 1 << 1,
	//	ComputeShader = 1 << 2,
	//	ColorAttachmentOutput = 1 << 3,
	//	EarlyFragmentTests = 1 << 4,
	//	LateFragmentTests = 1 << 5,
	//	Transfer = 1 << 6,
	//	BottomOfPipe = 1 << 7
	//};


	enum class AttachmentType {
		Color,
		Depth,
		Input,
		Resolve,
		Preserve,
		Swapchain
	};

	struct RenderPassDependency {
		bool srcExternalPass = true;
		Core::AccessType srcAccess = Core::AccessType::None;
		Core::AccessType dstAccess = Core::AccessType::None;
		Core::PipelineStage srcStage = Core::PipelineStage::None;
		Core::PipelineStage dstStage = Core::PipelineStage::None;
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

	//template <typename Enum>
	//constexpr auto operator|(Enum lhs, Enum rhs) -> Enum {
	//	using T = std::underlying_type_t<Enum>;
	//	return static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs));
	//}

	//template <typename Enum>
	//constexpr auto operator&(Enum lhs, Enum rhs) -> Enum {
	//	using T = std::underlying_type_t<Enum>;
	//	return static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs));
	//}

	//template <typename Enum>
	//constexpr auto operator~(Enum value) -> Enum {
	//	using T = std::underlying_type_t<Enum>;
	//	return static_cast<Enum>(~static_cast<T>(value));
	//}

	//template <typename Enum>
	//inline Enum& operator|=(Enum& lhs, Enum rhs) {
	//	lhs = lhs | rhs;
	//	return lhs;
	//}

	//template <typename Enum>
	//inline Enum& operator&=(Enum& lhs, Enum rhs) {
	//	lhs = lhs & rhs;
	//	return lhs;
	//}

};
