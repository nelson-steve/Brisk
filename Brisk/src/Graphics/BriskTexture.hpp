#pragma once

#include <string>

namespace Brisk 
{
	class BriskTexture {
	public:
		enum Format {
			RGB, BGR,
		};
		enum Type {
			TEXTURE2D, TEXTURE3D, TEXTURE_ARRAY, CUBEMAP,
		};
	public:
		static BriskTexture* Create(int width, int height);
		static BriskTexture* Create();
		static BriskTexture* Create(int width, int height, Format format, Type type);
		static BriskTexture* Create(std::string path);

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual void Resize() = 0;
	protected:
		int m_Width, m_Height;
		Format m_Format;
		Type m_TextureType;
	};
}