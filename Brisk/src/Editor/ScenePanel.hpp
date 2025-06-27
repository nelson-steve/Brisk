#pragma once

#include "IEditorPanel.hpp"
#include "Graphics/Vulkan/TextureVulkan.hpp"

namespace Brisk 
{
	class ScenePanel : public IEditorPanel {
	public:
		virtual void OnCreate() override;
		virtual void OnUpdate() override;
		virtual void OnDestroy() override;

		void SetImage(std::shared_ptr<Texture> tex);
	private:
		std::shared_ptr<Texture> m_SceneTexture;
		VkDescriptorSet m_SceneDescriptorSet;

		ImTextureID m_RenderTargetID;
	};
}