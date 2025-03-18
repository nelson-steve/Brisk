#pragma once

#include "IEditorPanel.hpp"
#include "Engine/Renderer/Texture.hpp"

#include "imgui.h"
#include "ImGuiBackends/imgui_impl_glfw.h"
#include "ImGuiBackends/imgui_impl_vulkan.h"
#include "glm/glm.hpp"

#include <unordered_map>

namespace Brisk 
{
	class Editor {
	public:
		Editor() = default;
		void Create();
		void Update();
		void Release();

		const glm::vec2& GetViewportSize() const { return m_ViewportSize; }

		//static VkDescriptorSet AddTexToUI(BriskTexture* texture);
	private:
		std::unordered_map<std::string, IEditorPanel*> m_Panels;
		ImGui_ImplVulkanH_Window s_MainWindowData;
		glm::vec2 m_ViewportSize;
	};
}
