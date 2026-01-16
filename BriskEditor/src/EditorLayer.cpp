#include "EditorLayer.hpp"
#include "Engine/Engine.hpp"
#include "AssetsPanel.hpp"
#include "ConsolePanel.hpp"
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
#include <imgui_internal.h>

namespace Brisk 
{
    struct GpuTimerResult {
        std::string name;
        float timeMS;
    };

    struct FrameStats {
        float cpuTimeMS;
        float frameTimeMS;
        int drawCalls;
        int dispatches;
        float memoryMB;
        std::vector<GpuTimerResult> gpuTimers;
    };

    void ShowPerformanceStatsWindow(float deltaTime)
    {
        ImGui::Begin("Performance Stats");

        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("CPU Time: %.3f ms", ImGui::GetIO().DeltaTime * 1000.0f);
        ImGui::Text("GPU Time: %.3f ms", Application::GetRenderer()->m_GpuTime);

        static std::vector<FrameStats> frames;

        FrameStats f;
        f.cpuTimeMS = ImGui::GetIO().DeltaTime * 1000.0f;
        f.frameTimeMS = float(16 + rand() % 5);
        f.drawCalls = 1000 + rand() % 500;
        f.dispatches = 50 + rand() % 20;
        f.memoryMB = 2000 + rand() % 500;

        f.gpuTimers = {
            {"Clusters AABB", Application::GetRenderer()->m_AABBTime},
            {"Assign Lights", Application::GetRenderer()->m_AssignLightToClustersTime},
            {"Raster", Application::GetRenderer()->m_RasterTime},
        };

        frames.push_back(f);

        int32_t historySize = 120;
        if (frames.size() > historySize)
            frames.erase(frames.begin());

        if (frames.empty())
        {
            ImGui::Text("No data yet");
            ImGui::End();
            return;
        }

        const FrameStats& latest = frames.back();

        // GPU per pass
        if (!latest.gpuTimers.empty())
        {
            ImGui::Separator();
            ImGui::Text("GPU Timings (ms):");

            ImGui::Columns(2, "GPU Timer", true);
            ImGui::Text("Pass"); ImGui::NextColumn();
            ImGui::Text("Time (ms)"); ImGui::NextColumn();
            ImGui::Separator();

            for (const auto& t : latest.gpuTimers)
            {
                ImGui::Text("%s", t.name.c_str()); ImGui::NextColumn();
                ImGui::Text("%.6f", t.timeMS); ImGui::NextColumn();
            }

            ImGui::Columns(1);
        }

        if (frames.size() > 1)
        {
            ImGui::Separator();
            ImGui::Text("Frame Time (ms)");

            std::vector<float> frameTimes;
            frameTimes.reserve(historySize);
            for (int i = std::max(0, (int)frames.size() - historySize); i < frames.size(); ++i)
                frameTimes.push_back(frames[i].frameTimeMS);

            ImGui::PlotLines("Frame Time", frameTimes.data(), frameTimes.size(), 0, nullptr, 0.0f, 50.0f, ImVec2(0, 80));
        }

        ImGui::End();
    }

    void ShowRendererSettingsWindow()
    {
        ImGui::Begin("Renderer Settings");

        RendererSettings& settings = Application::GetRendererSettings();

        ImGui::Checkbox("Ray Tracing", &settings.RayTracing);
        ImGui::Checkbox("CSM", &settings.CSM);
        ImGui::Checkbox("PCF", &settings.PCF);
        ImGui::DragFloat("PCF Scale", &settings.PCFScale, 0.01f, 0.0f, 2.0f);

        ImGui::Separator();

        ImGui::DragFloat("Bloom Threshold", &settings.threshold, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Bloom Knee", &settings.knee, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Bloom Intensity", &settings.intensity, 0.01f, 0.0f, 10.0f);

        // Shadow mapping
        ImGui::DragFloat("Near Clip", &settings.NearClip, 0.01f, 0.01f, 10.0f);
        ImGui::DragFloat("Far Clip", &settings.FarClip, 0.01f, 0.01f, 1000.0f);
        ImGui::DragFloat("Light Size", &settings.LightSize, 1.0f, 0.0f, 400.0f);
        ImGui::DragFloat("Scale Clip", &settings.Scale, 0.01f, 0.0f, 40.0f);

        ImGui::End();
    }

    void ShowDebugCSMMapsWindow()
    {
        ImGui::Begin("CSM Debug");

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 cellSize = { avail.x * 0.5f, avail.y * 0.5f }; // 2x2 grid

        auto ShowShadowMap = [&](ImTextureID texId, const char* label)
            {
                float aspect = (float)2048 / (float)2048;

                ImVec2 imageSize = cellSize;
                if (imageSize.x / imageSize.y > aspect) {
                    imageSize.x = imageSize.y * aspect;
                }
                else {
                    imageSize.y = imageSize.x / aspect;
                }

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::Image(texId, imageSize);
                ImGui::PopStyleVar();
            };

        ImGui::BeginChild("ShadowMapDebug");
        {
            // Row 0
            ShowShadowMap((ImTextureID)Application::GetRenderer()->m_ImGuiIdShadowMap0, "Cascade0");
            ImGui::SameLine();
            ShowShadowMap((ImTextureID)Application::GetRenderer()->m_ImGuiIdShadowMap1, "Cascade1");

            // Row 1
            ShowShadowMap((ImTextureID)Application::GetRenderer()->m_ImGuiIdShadowMap2, "Cascade2");
            ImGui::SameLine();
            ShowShadowMap((ImTextureID)Application::GetRenderer()->m_ImGuiIdShadowMap3, "Cascade3");
        }
        ImGui::EndChild();

        ImGui::End();
    }

    void ShowRandomLightsMenu()
    {
        ImGui::Begin("Random Light Generator");

        static uint32_t lightCount = 1000;
        static float range = 50.0f;
        static float radiusMin = 0.5f;
        static float radiusMax = 3.0f;
        static float colorMin = 0.2f;
        static float colorMax = 1.0f;
        static float intensityMin = 1.0f;
        static float intensityMax = 10.0f;

        ImGui::Text("Light Count");
        ImGui::InputScalar("##LightCount", ImGuiDataType_U32, &lightCount);

        ImGui::Separator();

        ImGui::SliderFloat("Spawn Range", &range, 1.0f, 500.0f);

        ImGui::Separator();

        ImGui::Text("Radius");
        ImGui::SliderFloat("Radius Min", &radiusMin, 0.01f, 10.0f);
        ImGui::SliderFloat("Radius Max", &radiusMax, 0.01f, 10.0f);

        ImGui::Separator();

        ImGui::Text("Color Intensity");
        ImGui::SliderFloat("Color Min", &colorMin, 0.0f, 1.0f);
        ImGui::SliderFloat("Color Max", &colorMax, 0.0f, 1.0f);

        ImGui::Separator();

        ImGui::Text("Light Intensity");
        ImGui::SliderFloat("Intensity Min", &intensityMin, 0.0f, 50.0f);
        ImGui::SliderFloat("Intensity Max", &intensityMax, 0.0f, 50.0f);

        ImGui::Separator();

        // Clamp safety
        radiusMin = std::min(radiusMin, radiusMax);
        colorMin = std::min(colorMin, colorMax);
        intensityMin = std::min(intensityMin, intensityMax);

        if (ImGui::Button("Generate Lights", ImVec2(200, 0)))
        {
            Application::GenerateRandomLights(
                lightCount,
                range,
                radiusMin,
                radiusMax,
                colorMin,
                colorMax,
                intensityMin,
                intensityMax);
        }

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
        m_Panels.insert({ "Scene" , scenePanel });

        AssetsPanel* assetsPanel = new AssetsPanel();
        m_Panels.insert({ "Assets" , assetsPanel });

        ConsolePanel* consolePanel = new ConsolePanel();
        m_Panels.insert({ "Console" , consolePanel });

        MaterialPanel* materialPanel = new MaterialPanel();
        m_Panels.insert({ "Material" , materialPanel });

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

        float deltaTime = 0.1;
        ShowPerformanceStatsWindow(deltaTime);
        ShowRendererSettingsWindow();
        ShowDebugCSMMapsWindow();
        ShowRandomLightsMenu();
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

