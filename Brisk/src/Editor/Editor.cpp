#include "Editor.hpp"
#include "Engine/Engine.hpp"
//#include "Graphics/Vulkan/TextureVulkan.hpp"
#include "Graphics/Vulkan/VulkanRenderer/RendererVulkan.hpp"

#include "AssetsPanel.hpp"
#include "ConsolePanel.hpp"
#include "GamePanel.hpp"
#include "HeirarchyPanel.hpp"
#include "InspectorPanel.hpp"
#include "ScenePanel.hpp"


namespace Brisk {
    // Sample asset dat

    struct PerformanceStat
    {
        std::string name;
        float value;
        std::string unit;
    };

    void SetLightGreenishTheme() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (light but not bright)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.85f, 0.9f, 0.88f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f };       // Sea-greenish
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.5f, 0.75f, 0.65f, 1.0f }; // Brighter sea-green on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.35f, 0.6f, 0.5f, 1.0f };   // Active state

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f };        // Sea-greenish buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.5f, 0.75f, 0.65f, 1.0f }; // Lighter hover
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.35f, 0.6f, 0.5f, 1.0f };   // Darker active

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.8f, 0.85f, 0.83f, 1.0f };       // Light frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.9f, 0.95f, 0.93f, 1.0f }; // Slightly brighter when hovered
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.7f, 0.75f, 0.73f, 1.0f };  // Active frame background

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f };            // Sea-greenish tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.5f, 0.75f, 0.65f, 1.0f };     // Brighter hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.45f, 0.7f, 0.6f, 1.0f };       // Slightly brighter when active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.35f, 0.6f, 0.5f, 1.0f };    // Dimmed when unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f }; // Slightly brighter when active and unfocused

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.35f, 0.6f, 0.5f, 1.0f };         // Sea-greenish title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f };  // Slightly brighter when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.3f, 0.55f, 0.45f, 1.0f }; // Dimmed for collapsed title

    }

    void SetDarkGreenishTheme() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (darker)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.15f, 0.13f, 1.0f }; // Dark, slight greenish tint

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f };       // Dark sea-greenish
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.2f, 0.55f, 0.45f, 1.0f }; // Brighter sea-green on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.1f, 0.35f, 0.3f, 1.0f };   // Slightly darker when active

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f };        // Dark sea-greenish buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.2f, 0.55f, 0.45f, 1.0f }; // Lighter hover
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.1f, 0.35f, 0.3f, 1.0f };   // Darker active

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.12f, 0.2f, 0.18f, 1.0f };       // Dark frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.15f, 0.35f, 0.3f, 1.0f }; // Sea-green hover
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.1f, 0.3f, 0.25f, 1.0f };  // Active state

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f };            // Sea-green tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.2f, 0.55f, 0.45f, 1.0f };     // Brighter on hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.18f, 0.5f, 0.4f, 1.0f };       // Slightly brighter when active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.12f, 0.35f, 0.3f, 1.0f };   // Dimmed when unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f }; // Bright when unfocused but active

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.1f, 0.3f, 0.25f, 1.0f };         // Dark sea-greenish title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f };  // Slightly brighter when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.08f, 0.25f, 0.2f, 1.0f }; // Dimmed for collapsed title

    }

    void LavenderTheme() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (dark base)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f }; // Darker, neutral tone

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f };        // Muted lavender
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.2f, 0.15f, 0.25f, 1.0f }; // Subtle pinkish lavender on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f }; // Slightly darker lavender when active

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f };        // Muted lavender buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.2f, 0.15f, 0.25f, 1.0f }; // Pinkish hover effect
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f }; // Slightly pinkish lavender for active

        // Frame Background
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.12f, 0.1f, 0.15f, 1.0f };       // Dark lavender frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.2f, 0.15f, 0.25f, 1.0f }; // Subtle pinkish lavender on hover
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f }; // Active state with muted lavender

        //colors[ImGuiCol_MenuBarBg] = ImVec4{ 1.0f, 0.14f, 0.0f, 1.0f }; // Active state with muted lavender
        colors[ImGuiCol_PopupBg] = ImVec4{ 1.0f, 0.14f, 0.0f, 1.0f }; // Active state with muted lavender

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f };           // Muted lavender tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.2f, 0.15f, 0.25f, 1.0f };    // Subtle pinkish lavender on hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f };    // Slightly darker for active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.1f, 0.1f, 0.12f, 1.0f };   // Darker when unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f }; // Brighter when active and unfocused

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };         // Darker title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f }; // Muted lavender when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f }; // Very dark for collapsed title
    }

	void Editor::Create() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.WantCaptureMouse = true;
        io.WantCaptureKeyboard = true;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;      // Enable Gamepad Controls
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        LavenderTheme();
        
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Engine::s_MainWindow->GetWindowHandle(), true);

        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = GpuContextVulkan::s_Instance;
        info.PhysicalDevice = GpuContextVulkan::s_GPUDevice->GetPhysicalDevice();
        info.Device = GpuContextVulkan::s_GPUDevice->GetDevice();
        info.QueueFamily = 0;
        info.Queue = GpuContextVulkan::s_GPUDevice->GetGraphicsQueue().Handle;
        info.DescriptorPool = static_cast<RendererVulkan*>(Engine::s_Renderer)->GetUIDescriptorPool();
        info.RenderPass = static_cast<RendererVulkan*>(Engine::s_Renderer)->GetUIRenderpass();
        info.ImageCount = 2;
        info.MinImageCount = 2;
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_Init(&info);

        ScenePanel* scenePanel = new ScenePanel();
        m_Panels.insert({ "Scene" , scenePanel });

        AssetsPanel* assetsPanel = new AssetsPanel();
        m_Panels.insert({ "Assets" , assetsPanel });

        ConsolePanel* consolePanel = new ConsolePanel();
        m_Panels.insert({ "Console" , consolePanel });

        GamePanel* gamePanel = new GamePanel();
        m_Panels.insert({ "Game" , gamePanel });

        HeirarchyPanel* heirarchyPanel = new HeirarchyPanel();
        m_Panels.insert({ "Heirarchy" , heirarchyPanel });

        //m_Texture = new TextureVulkan();
        //m_Texture->Create("../Data/Images/texture.jpg");

        for (const auto& panel : m_Panels) {
            panel.second->OnCreate();
        }

        static_cast<RendererVulkan*>(Engine::s_Renderer)->CreateOffscreenResources();

        //textureSet = ImGui_ImplVulkan_AddTexture(m_Texture->GetSampler(), m_Texture->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

    // Function to show the performance stats window
    void ShowPerformanceStatsWindow(float deltaTime, const std::vector<PerformanceStat>& stats)
    {
        // Begin the performance stats window
        ImGui::Begin("Performance Stats");

        // Show FPS
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

        // Show Delta Time
        ImGui::Text("Delta Time: %.3f ms", deltaTime * 1000.0f);

        // Display custom performance stats
        for (const auto& stat : stats)
        {
            ImGui::Text("%s: %.3f %s", stat.name.c_str(), stat.value, stat.unit.c_str());
        }

        // End the performance stats window
        ImGui::End();
    }

    void MenuBar() {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N")) { /* Handle New action */ }
                if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Handle Open action */ }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Handle Save action */ }
                if (ImGui::MenuItem("Exit", "Alt+F4")) { /* Handle Exit action */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* Handle Undo action */ }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) { /* Handle Redo action (disabled in this case) */ }
                if (ImGui::MenuItem("Cut", "Ctrl+X")) { /* Handle Cut action */ }
                if (ImGui::MenuItem("Copy", "Ctrl+C")) { /* Handle Copy action */ }
                if (ImGui::MenuItem("Paste", "Ctrl+V")) { /* Handle Paste action */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                if (ImGui::MenuItem("Options", "")) { /* Handle Options action */ }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Options"))
            {
                if (ImGui::MenuItem("Settings", "")) { /* Handle Settings action */ }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void Editor::Update() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        MenuBar();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        for (const auto& panel : m_Panels) {
            panel.second->OnUpdate();
        }

        //ImGui::Begin("Test Image");
        //ImGui::Image((ImTextureID)textureSet, ImVec2{ (float)m_Texture->GetWidth(), (float)m_Texture->GetHeight() });
        //ImGui::End();

        ImGui::Begin("Console");
        ImGui::End();

        // Sample performance stats (add more as needed)
        std::vector<PerformanceStat> stats = {
            {"GPU Usage", 65.0f, "%"},
            {"CPU Usage", 45.3f, "%"},
            {"Memory Usage", 1536.0f, "MB"},
            {"Render Time", 16.67f, "ms"}
        };

        // Display the stats window
        float deltaTime = 0.1;
        ShowPerformanceStatsWindow(deltaTime, stats);

        ImGui::Render();
    }

    VkDescriptorSet Editor::AddTexToUI(BriskTexture* texture) {
        return ImGui_ImplVulkan_AddTexture(
            static_cast<TextureVulkan*>(texture)->GetSampler(), static_cast<TextureVulkan*>(texture)->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void Editor::Release() {
        for (const auto& panel : m_Panels) {
            panel.second->OnDestroy();
        }
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

