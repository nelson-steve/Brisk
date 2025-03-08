#pragma once

// INCLUDES
#include "Descriptor.hpp"
#include "Pipeline.hpp"
//--------------------
#include <vector>
#include <cstdint>
//----------------

#define DEFINE_BASE_CLASS_CONSTRUCTOR(className) \
private: \
    className() = default; \
protected: \
    virtual ~className() = default;

namespace Brisk
{
	class Shader
	{
	public:
		enum ShaderType
		{
			VERTEX,
			FRAGMENT,
		};

		virtual void Init(std::shared_ptr<Pipeline> pipeline) = 0;

		virtual void SetPipeline(std::shared_ptr<Pipeline> pipeline) = 0;
		virtual void SetAlbedoTexture(std::shared_ptr<Texture> texture) = 0;
		virtual void SetNormalTexture(std::shared_ptr<Texture> texture) = 0;
		virtual void SetMetallicTexture(std::shared_ptr<Texture> texture) = 0;
		virtual void SetOcclusionTexture(std::shared_ptr<Texture> texture) = 0;
		virtual void SetEmissiveTexture(std::shared_ptr<Texture> texture) = 0;
		virtual void SetMVPBuffer(std::shared_ptr<Buffer> buffer) = 0;

		virtual void BindTexture(std::shared_ptr<Texture> texture, uint32_t descriptor, uint32_t binding) = 0;

		virtual void Allocate(std::shared_ptr<CommandBuffer> cmdBuffer, std::shared_ptr<Pipeline> pipeline) = 0;
		virtual void UpdateResources() = 0;
		virtual void AddTextures(std::vector<std::shared_ptr<Texture>> textures) = 0;
		virtual void Bind(std::shared_ptr<CommandBuffer> cmdBuffer, std::shared_ptr<Pipeline> pipeline) = 0;

		inline void AddDescriptorLayout(const std::shared_ptr<DescriptorLayout> descriptor) { m_DescriptorLayout = descriptor; }
		inline const std::shared_ptr<DescriptorLayout> GetDescriptorLayout() const { return m_DescriptorLayout; }

		static std::shared_ptr<Shader> Create();

	public:
		std::shared_ptr<DescriptorLayout> m_DescriptorLayout;
	};
}