#pragma once

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
		VkSamplerAddressMode address_modeU;
		VkSamplerAddressMode address_modeV;
		VkSamplerAddressMode address_modeW;
	};

	class Texture {
	public:
		enum Format {
			RGB, BGR,
		};
		enum Type {
			TEXTURE2D, TEXTURE3D, TEXTURE_ARRAY, CUBEMAP,
		};
	public:
		virtual void Init() = 0;
		virtual void Init(int width, int height, Format format = Format::RGB, Type type = Type::TEXTURE2D) = 0;
		virtual void Init(tinygltf::Image image, TextureSampler sampler) = 0;
		virtual void Init(const std::string& path) = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void Resize() = 0;
	protected:
		int m_Width, m_Height;
		Format m_Format;
		Type m_TextureType;
	};
}