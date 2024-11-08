#pragma once

#include <string>
#include <memory>

namespace Brisk 
{
	class Texture {
	public:
		enum Format {
			RGB, BGR,
		};
		enum Type {
			TEXTURE2D, TEXTURE3D, TEXTURE_ARRAY, CUBEMAP,
		};
	public:
		static std::shared_ptr<Texture> Create(int width, int height);
		static std::shared_ptr<Texture> Create();
		static std::shared_ptr<Texture> Create(int width, int height, Format format, Type type);
		static std::shared_ptr<Texture> Create(std::string path);

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void Resize() = 0;
	protected:
		int m_Width, m_Height;
		Format m_Format;
		Type m_TextureType;
	};
}