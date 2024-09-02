#pragma once

#include "imgui.h"
#include "ImGuiBackends/imgui_impl_glfw.h"
#include "ImGuiBackends/imgui_impl_vulkan.h"

namespace Brisk {
	class Editor {
	public:
		void Create();
		void UpdateWindowSize(float width, float height);
		void Update();
	private:
		static ImGui_ImplVulkanH_Window s_MainWindowData;
	};
}
