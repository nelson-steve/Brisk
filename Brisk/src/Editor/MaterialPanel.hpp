#pragma once

#include "IEditorPanel.hpp"
#include "Graphics/Vulkan/TextureVulkan.hpp"

namespace Brisk 
{
	class MaterialPanel : public IEditorPanel {
	public:
		virtual void OnCreate() override;
		virtual void OnUpdate() override;
		virtual void OnDestroy() override;
	private:
	};
}