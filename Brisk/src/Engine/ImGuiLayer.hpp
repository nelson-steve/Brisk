#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/RenderPass.hpp"
#include "Layers/Layer.hpp"

#include "imgui.h"
#include "ImGuiBackends/imgui_impl_glfw.h"
#include "ImGuiBackends/imgui_impl_vulkan.h"
#include "glm/glm.hpp"

#include <unordered_map>

// Fwd
struct ID3D12DescriptorHeap;

namespace Brisk
{
	class ImGuiLayer : public Layer {
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void Render(std::shared_ptr<CommandBuffer> cmd);

		const glm::vec2& GetViewportSize() const { return m_ViewportSize; }
	private:
		ImGui_ImplVulkanH_Window s_MainWindowData;
		glm::vec2 m_ViewportSize;

		ID3D12DescriptorHeap* imguiSrvDescHeap;
	};
}

