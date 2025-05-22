#include "Editor.hpp"
#include "Engine/Engine.hpp"
#include "AssetsPanel.hpp"
#include "ConsolePanel.hpp"
#include "GamePanel.hpp"
#include "HeirarchyPanel.hpp"
#include "InspectorPanel.hpp"
#include "ScenePanel.hpp"
#include <Graphics/Vulkan/GpuAdapterVulkan.hpp>


namespace Brisk 
{
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
        colors[ImGuiCol_PopupBg] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f }; // Active state with muted lavender

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

        colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f }; // Very dark for collapsed title
    }

    void PinkTheme() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (bright, pinkish tone)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.9f, 0.78f, 0.85f, 1.0f }; // Bright pinkish background

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };        // Bright pink
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.9f, 0.6f, 0.75f, 1.0f };  // Slightly brighter pink on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.8f, 0.5f, 0.65f, 1.0f };   // Slightly darker pink when active

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };        // Bright pink buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.9f, 0.6f, 0.75f, 1.0f };  // Brighter pink hover effect
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.8f, 0.5f, 0.65f, 1.0f };   // Slightly darker pink for active

        // Frame Background
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.8f, 0.55f, 0.65f, 1.0f };       // Pink frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.9f, 0.6f, 0.75f, 1.0f }; // Brighter pink on hover
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f }; // Active state with bright pink

        // Popup
        colors[ImGuiCol_PopupBg] = ImVec4{ 0.9f, 0.65f, 0.75f, 1.0f };       // Bright pink popup background

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };           // Bright pink tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.9f, 0.6f, 0.75f, 1.0f };     // Brighter pink on hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.8f, 0.5f, 0.65f, 1.0f };      // Slightly darker for active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.75f, 0.5f, 0.6f, 1.0f };   // Slightly muted pink for unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f }; // Bright when active and unfocused

        // Text
        colors[ImGuiCol_Text] = ImVec4{ 0.75f, 0.45f, 0.65f, 1.0f };         // Bright pink title background
        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.75f, 0.45f, 0.65f, 1.0f };         // Bright pink title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };    // Brighter pink when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.7f, 0.4f, 0.6f, 1.0f };   // Muted pink for collapsed title

    }

    void PinkTheme2() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (less bright, solid tone)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.25f, 0.2f, 0.28f, 1.0f }; // Darker, more solid background

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f };        // Darker, solid pink
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.7f, 0.4f, 0.55f, 1.0f };  // Slightly lighter on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };   // Even darker when active

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f };        // Solid pink buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.7f, 0.4f, 0.55f, 1.0f };  // Brighter on hover
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };   // Darker pink when active

        // Frame Background
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.55f, 0.3f, 0.4f, 1.0f };        // Solid pink frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f }; // Brighter on hover
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };  // Darker when active

        // Popup
        colors[ImGuiCol_PopupBg] = ImVec4{ 0.3f, 0.25f, 0.35f, 1.0f };       // Solid background for popups

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };           // Darker pink tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.7f, 0.4f, 0.55f, 1.0f };    // Lighter on hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f };    // Brighter when active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.4f, 0.25f, 0.35f, 1.0f }; // Muted pink for unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.55f, 0.3f, 0.45f, 1.0f }; // Brighter when active and unfocused

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.45f, 0.25f, 0.35f, 1.0f };         // Solid title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.55f, 0.3f, 0.45f, 1.0f };    // Brighter when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.35f, 0.2f, 0.3f, 1.0f };  // Darker for collapsed title

        // Text
        colors[ImGuiCol_Text] = ImVec4{ 0.9f, 0.9f, 0.9f, 1.0f };               // Brighter text for contrast

        // Scrollbars
        colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.25f, 0.2f, 0.28f, 1.0f };      // Solid scrollbar background
        colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.55f, 0.3f, 0.4f, 1.0f };     // Pink grab
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f }; // Lighter on hover
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };  // Darker when active

        // Checkboxes
        colors[ImGuiCol_CheckMark] = ImVec4{ 0.9f, 0.9f, 0.9f, 1.0f };           // White for contrast on checkboxes

        // Slider
        colors[ImGuiCol_SliderGrab] = ImVec4{ 0.55f, 0.3f, 0.4f, 1.0f };         // Solid pink grab
        colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f };  // Brighter on active

        // Separators
        colors[ImGuiCol_Separator] = ImVec4{ 0.35f, 0.2f, 0.3f, 1.0f };          // Solid, darker separator
        colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.4f, 0.25f, 0.35f, 1.0f };  // Slightly brighter on hover
        colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.45f, 0.3f, 0.4f, 1.0f };    // Brighter when active

        // Resize grip
        colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.55f, 0.3f, 0.4f, 1.0f };         // Pink grip
        colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f }; // Brighter on hover
        colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.7f, 0.4f, 0.55f, 1.0f };   // Brighter when active

        // Modals
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4{ 0.2f, 0.15f, 0.25f, 0.75f }; // Dimmed background for modals

        // DragDrop
        colors[ImGuiCol_DragDropTarget] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };    // Brighter pink for drag-and-drop target

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

        io.FontDefault = io.Fonts->AddFontFromFileTTF("../Data/Fonts/Nunito/Nunito-Medium.ttf", 18.0f);

        LavenderTheme();
        //PinkTheme();
        //PinkTheme2();
        
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Engine::s_Application->GetWindow()->GetWindowHandle(), true);

        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetInstance();
        info.PhysicalDevice = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetPhysicalDevice();
        info.Device = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDevice();
        info.QueueFamily = 0;
        info.Queue = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetGraphicsQueue();
        info.DescriptorPool = std::static_pointer_cast<GpuAdapterVulkan>(Engine::s_Application->GetGpuAdapter())->GetDescriptorPool();
        //info.RenderPass = ;
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

        InspectorPanel* inspectorPanel = new InspectorPanel();
        m_Panels.insert({ "Inspector" , inspectorPanel });

        //m_Texture = new TextureVulkan();
        //m_Texture->Create("../Data/Images/texture.jpg");

        for (const auto& panel : m_Panels) {
            panel.second->OnCreate();
        }

        //static_cast<RendererVulkan*>(Engine::s_Renderer)->CreateOffscreenResources();

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

    //VkDescriptorSet Editor::AddTexToUI(BriskTexture* texture) {
    //    return ImGui_ImplVulkan_AddTexture(
    //        static_cast<TextureVulkan*>(texture)->GetSampler(), static_cast<TextureVulkan*>(texture)->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //}

    void Editor::Release() {
        for (const auto& panel : m_Panels) {
            panel.second->OnDestroy();
        }
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

