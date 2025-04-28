#pragma once

#include "Engine/Renderer/RHI.hpp"
#include "Engine/Renderer/CommandBuffer.hpp"

#include <tiny_gltf.h>

#include <string>
#include <memory>

namespace Brisk 
{
	class Texture {
	public:
		enum class ImageLayout {
			Undefined,
			General,
			ShaderReadOnlyOptimal,
			TransferSrc,
			TransferDst,
			ColorAttachmentOptimal,
			DepthStencilAttachmentOptimal,
		};

		enum class AccessType {
			None = 0,
			ShaderRead,
			ShaderWrite,
			TransferRead,
			TransferWrite,
			ColorAttachmentRead,
			ColorAttachmentWrite,
		};

		enum class PipelineStage {
			TopOfPipe,
			BottomOfPipe,
			TransferStage,
			ComputeShader,
			FragmentShader,
		};

		enum ImageAspectFlags {
			Color,
			Depth,
			Stencil,
		};

		struct ImageBarrierParams {
			std::shared_ptr<Texture> texture;
			ImageLayout oldLayout;
			ImageLayout newLayout;
			AccessType srcAccess;
			AccessType dstAccess;
			PipelineStage srcStage;
			PipelineStage dstStage;
			ImageAspectFlags aspectFlags;
			uint32_t baseMipLevel = 0;
			uint32_t levelCount = 1;
			uint32_t baseLayer = 0;
			uint32_t layerCount = 1;
		};

		///--------------------------------------------------------------------------
		enum Filter {
			FILTER_NEAREST = 0,
			FILTER_LINEAR = 1,
			FILTER_CUBIC_EXT = 1000015000,
		};

		enum SamplerAddressMode {
			SAMPLER_ADDRESS_MODE_REPEAT = 0,
			SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1,
			SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2,
			SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER = 3,
			SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 4,
		};

		struct TextureSampler {
			Filter mag_filter;
			Filter min_filter;
			SamplerAddressMode address_modeU;
			SamplerAddressMode address_modeV;
			SamplerAddressMode address_modeW;
		};
		enum TextureType {
			TEXTURE2D, TEXTURE3D, TEXTURE_ARRAY, CUBEMAP
		};
		
		enum Sampling {
			SAMPLE_COUNT_1_BIT,
			SAMPLE_COUNT_2_BIT,
			SAMPLE_COUNT_4_BIT,
			SAMPLE_COUNT_8_BIT,
			SAMPLE_COUNT_16_BIT,
			SAMPLE_COUNT_32_BIT,
			SAMPLE_COUNT_64_BIT
		};
		struct TextureSpecification {
			TextureType p_Type;
			TextureSampler p_Sampler{};
			Core::Format p_Format;
			bool p_IsDepth;
			uint32_t p_Width = 1, p_Height = 1, p_Depth = 1;
			uint32_t p_MipLevels = 1;
			uint32_t p_ArrayLayers = 1;
		};
	public:
		virtual void Init(const TextureSpecification& specs) = 0;
		virtual void Init(const std::string& path) = 0;
		virtual void Init(tinygltf::Image image, TextureSampler sampler) = 0;

		virtual void TransitionImageLayout(std::shared_ptr<CommandBuffer> cmd, std::vector<ImageBarrierParams> params) = 0;
		virtual void CopyImage(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> src, std::shared_ptr<Texture> dest, uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual bool IsDepth() { return m_IsDepth; }
		virtual void Resize() = 0;


		static SamplerAddressMode GetVkWrapMode(int32_t wrapMode)
		{
			switch (wrapMode) {
			case -1:
			case 10497:
				return SamplerAddressMode::SAMPLER_ADDRESS_MODE_REPEAT;
			case 33071:
				return SamplerAddressMode::SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case 33648:
				return SamplerAddressMode::SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			}

			//std::cerr << "Unknown wrap mode for getVkWrapMode: " << wrapMode << std::endl;
			return SamplerAddressMode::SAMPLER_ADDRESS_MODE_REPEAT;
		}

		static Filter GetVkFilterMode(int32_t filterMode)
		{
			switch (filterMode) {
			case -1:
			case 9728:
				return Filter::FILTER_LINEAR;
			case 9729:
				return Filter::FILTER_LINEAR;
			case 9984:
				return Filter::FILTER_NEAREST;
			case 9985:
				return Filter::FILTER_NEAREST;
			case 9986:
				return Filter::FILTER_LINEAR;
			case 9987:
				return Filter::FILTER_LINEAR;
			}

			//std::cerr << "Unknown filter mode for GetFilterMode: " << filterMode << std::endl;
			return Filter::FILTER_NEAREST;
		}

		static std::shared_ptr<Texture> Create();
	protected:
		// Metadata
		uint32_t m_Width, m_Height;
		bool m_IsDepth;
		Core::Format m_Format;
		TextureType m_TextureType;
	};
}