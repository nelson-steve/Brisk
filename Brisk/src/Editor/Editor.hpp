#pragma once

#include "HeirarchyPanel.hpp"
#include "ViewportPanel.hpp"

#include "imgui.h"
#include "ImGuiBackends/imgui_impl_glfw.h"
#include "ImGuiBackends/imgui_impl_vulkan.h"

#include "vector"

namespace Brisk {
	class Editor {
	public:
		Editor() = default;
		void Create();
		void Update(VkDescriptorSet set);
		void Release();
	private:
		ImGui_ImplVulkanH_Window s_MainWindowData;
		std::vector<IEditorPanel*> s_Panels;
	};
}
