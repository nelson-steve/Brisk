#pragma once

#include "Engine/Renderer/Descriptor.hpp"
#include "Graphics/Texture.hpp"

#include "Engine/Renderer/Shader.hpp"

#include <Volk/volk.h>

#include <vector>

namespace Brisk
{
	class ShaderVulkan : public Shader
	{
	public:
		virtual void Init(std::shared_ptr<Pipeline> pipeline, std::string layoutName) override;

		virtual void SetPipeline(std::shared_ptr<Pipeline> pipeline) override;
		virtual void SetAlbedoTexture(std::shared_ptr<Texture> texture) override;
		virtual void SetNormalTexture(std::shared_ptr<Texture> texture) override;
		virtual void SetMetallicTexture(std::shared_ptr<Texture> texture) override;
		virtual void SetOcclusionTexture(std::shared_ptr<Texture> texture) override;
		virtual void SetEmissiveTexture(std::shared_ptr<Texture> texture) override;
		virtual void SetMVPBuffer(std::shared_ptr<Buffer> buffer) override;

		virtual void Allocate(std::shared_ptr<CommandBuffer> cmdBuffer, std::shared_ptr<Pipeline> pipeline) override;
		virtual void UpdateResources() override;
		virtual void AddTextures(std::vector<std::shared_ptr<Texture>> textures) override;
		virtual void Bind(std::shared_ptr<CommandBuffer> cmdBuffer, std::shared_ptr<Pipeline> pipeline) override;

		inline void AddDescriptorLayout(const std::shared_ptr<DescriptorLayout> descriptor) { m_DescriptorLayout = descriptor; }
		inline const std::shared_ptr<DescriptorLayout> GetDescriptorLayout() const { return m_DescriptorLayout; }

		//const VkPipelineShaderStageCreateInfo GetShaderStage() const { return m_ShaderStage; }

	private:
		VkDescriptorSet m_DescriptorSet;
		std::vector<VkWriteDescriptorSet> m_DescriptorWrites;
	};
}