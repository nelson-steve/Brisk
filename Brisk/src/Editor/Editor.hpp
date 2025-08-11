#pragma once

#include "IEditorPanel.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderPass.hpp"

#include "imgui.h"
#include "ImGuiBackends/imgui_impl_glfw.h"

#include "ImGuiBackends/imgui_impl_vulkan.h"


#include "glm/glm.hpp"

#include <unordered_map>

// Fwd
struct ID3D12DescriptorHeap;

namespace Brisk 
{
	class Editor {
	public:
		Editor() = default;
		void Create(std::shared_ptr<RenderPass> renderpass, std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> tex);
		void Update();
		void Render(std::shared_ptr<CommandBuffer> cmd);
		void Release();

		const glm::vec2& GetViewportSize() const { return m_ViewportSize; }
	private:
		std::unordered_map<std::string, IEditorPanel*> m_Panels;
		ImGui_ImplVulkanH_Window s_MainWindowData;
		glm::vec2 m_ViewportSize;

		ID3D12DescriptorHeap* imguiSrvDescHeap;
	};
}
