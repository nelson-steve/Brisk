#pragma once

#include "IEditorPanel.hpp"
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
	class EditorLayer : public Layer {
	public:
		EditorLayer() = default;
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void OnUpdate(float ts) override;
		virtual void OnImGuiRender() override;
		void OnEvent(Event& e) override;

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
