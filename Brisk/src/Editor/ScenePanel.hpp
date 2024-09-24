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
	private:
		BriskTexture* m_SceneTexture;
		VkDescriptorSet m_SceneDescriptorSet;
	};
}