#pragma once

#include "../Engine/Renderer/RHI.hpp"

#include <tiny_gltf.h>

#include <string>
#include <memory>

namespace Brisk 
{
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

	class Texture {
	public:
		enum Type {
			TEXTURE2D, TEXTURE3D, TEXTURE_ARRAY, CUBEMAP,
		};
	public:
		virtual void Init() = 0;
		virtual void Init(int width, int height, Core::Format format = Core::Format::FORMAT_R8G8B8A8_SRGB, Type type = Type::TEXTURE2D) = 0;
		virtual void Init(tinygltf::Image image, TextureSampler sampler) = 0;
		virtual void Init(const std::string& path) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
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
		int m_Width, m_Height;
		Core::Format m_Format;
		Type m_TextureType;
	};
}