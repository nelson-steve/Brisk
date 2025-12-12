#include "EditorLayer.hpp"
#include "Engine/Engine.hpp"
#include "AssetsPanel.hpp"
#include "ConsolePanel.hpp"
#include "GamePanel.hpp"
#include "HeirarchyPanel.hpp"
#include "InspectorPanel.hpp"
#include "ScenePanel.hpp"
#include "MaterialPanel.hpp"
#include <Graphics/Vulkan/GpuAdapterVulkan.hpp>
#include <Graphics/Vulkan/RenderpassVulkan.hpp>
#include <Graphics/DirectX12/GpuAdapterDirectX12.hpp>

#include "ImGuiBackends/imgui_impl_dx12.h"
#include <ImGuiBackends/imgui_impl_win32.h>

#include <memory>
#include <Graphics/DirectX12/CommandBufferDirectX12.hpp>

namespace Brisk 
{
    struct PerformanceStat
    {
        std::string name;
        float value;
        std::string unit;
    };

    void ShowPerformanceStatsWindow(float deltaTime, const std::vector<PerformanceStat>& stats)
    {
        ImGui::Begin("Performance Stats");

        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Delta Time: %.3f ms", ImGui::GetIO().DeltaTime * 1000.0f);
        for (const auto& stat : stats)
        {
            ImGui::Text("%s: %.3f %s", stat.name.c_str(), stat.value, stat.unit.c_str());
        }

        ImGui::End();
    }

    void ShowRendererSettingsWindow()
    {
        ImGui::Begin("Renderer Settings");

        ImGui::Checkbox("##toggle", &Application::GetRendererSettings().RayTracing);
        ImGui::SameLine();
        ImGui::Text("RayTracing");

        ImGui::End();
    }

    void MenuBar() {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N")) { }
                if (ImGui::MenuItem("Open", "Ctrl+O")) { }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { }
                if (ImGui::MenuItem("Exit", "Alt+F4")) { }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) { }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) { }
                if (ImGui::MenuItem("Cut", "Ctrl+X")) { }
                if (ImGui::MenuItem("Copy", "Ctrl+C")) { }
                if (ImGui::MenuItem("Paste", "Ctrl+V")) { }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Options", "")) { }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::MenuItem("Settings", "")) { }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void EditorLayer::Update() {
    }

    void EditorLayer::OnAttach() {
        ScenePanel* scenePanel = new ScenePanel();
        //scenePanel->SetImage(tex);
        m_Panels.insert({ "Scene" , scenePanel });

        AssetsPanel* assetsPanel = new AssetsPanel();
        m_Panels.insert({ "Assets" , assetsPanel });

        ConsolePanel* consolePanel = new ConsolePanel();
        m_Panels.insert({ "Console" , consolePanel });

        MaterialPanel* materialPanel = new MaterialPanel();
        m_Panels.insert({ "Material" , materialPanel });

        GamePanel* gamePanel = new GamePanel();
        m_Panels.insert({ "Game" , gamePanel });

        HeirarchyPanel* heirarchyPanel = new HeirarchyPanel();
        m_Panels.insert({ "Heirarchy" , heirarchyPanel });

        InspectorPanel* inspectorPanel = new InspectorPanel();
        m_Panels.insert({ "Inspector" , inspectorPanel });

        for (const auto& panel : m_Panels) {
            panel.second->OnCreate();
        }
    }

    void EditorLayer::OnDetach() {

    }

    void EditorLayer::OnUpdate(float ts) {
    }

    void EditorLayer::OnImGuiRender() {
        MenuBar();
        for (const auto& panel : m_Panels) {
            panel.second->OnUpdate();
        }

        std::vector<PerformanceStat> stats = {
            {"GPU Usage", 65.0f, "%"},
            {"CPU Usage", 45.3f, "%"},
            {"Memory Usage", 1536.0f, "MB"},
            {"Render Time", 16.67f, "ms"}
        };

        float deltaTime = 0.1;
        ShowPerformanceStatsWindow(deltaTime, stats);
        ShowRendererSettingsWindow();
    }

    void EditorLayer::OnEvent(Event& e) {

    }

    void EditorLayer::Release() {
        for (const auto& panel : m_Panels) {
            panel.second->OnDestroy();
        }
#ifdef BRISK_ENABLE_DIRECTX12

#else
        ImGui_ImplVulkan_Shutdown();
#endif
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

