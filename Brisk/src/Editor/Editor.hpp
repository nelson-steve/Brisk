#pragma once

#include "imgui.h"
#include "ImGuiBackends/imgui_impl_glfw.h"
#include "ImGuiBackends/imgui_impl_vulkan.h"

namespace Brisk {
	class Editor {
	public:
		void Create();
		void Update();
		void Release();
	private:
		static ImGui_ImplVulkanH_Window s_MainWindowData;
	};
}
