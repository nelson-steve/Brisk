#include "Editor.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/VulkanRenderer/RendererVulkan.hpp"

namespace Brisk {
    // Sample asset data
    struct Asset
    {
        std::string name;
        bool isFolder;
        std::vector<Asset> children; // Only used if isFolder is true
    };

    struct PerformanceStat
    {
        std::string name;
        float value;
        std::string unit;
    };

    TextureVulkan* m_Texture;
    VkDescriptorSet textureSet;

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

        // Setup Dear ImGui style
        //ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();
        //SetLightGreenishTheme();
        //SetDarkGreenishTheme();
        LavenderTheme();
        
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Engine::s_MainWindow->GetWindowHandle(), true);

        ImGui_ImplVulkan_InitInfo info{};
        static_cast<RendererVulkan*>(Engine::s_Renderer)->SetupImGuiData(info);
        ImGui_ImplVulkan_Init(&info);

        //s_Panels.push_back(new HeirarchyPanel());
        //s_Panels.push_back(new ViewportPanel());
        //for (int i = 0; i < s_Panels.size(); i++)
        //{
        //    s_Panels[i]->OnCreate();
        //}

        m_Texture = new TextureVulkan();
        m_Texture->Create("../Data/Images/texture.jpg");

        textureSet = ImGui_ImplVulkan_AddTexture(m_Texture->GetSampler(), m_Texture->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

    void ShowAssetFolder(const Asset& asset)
    {
        if (asset.isFolder)
        {
            if (ImGui::TreeNode(asset.name.c_str())) // Folder node
            {
                for (const auto& child : asset.children)
                {
                    ShowAssetFolder(child); // Recursively show child assets/folders
                }
                ImGui::TreePop(); // Close the folder node
            }
        }
        else
        {
            // Show individual asset
            ImGui::Text(asset.name.c_str());
        }
    }

    void ShowAssetsWindow(const std::vector<Asset>& assets)
    {
        // Begin the assets window
        ImGui::Begin("Assets");

        // Iterate over the root-level assets (could be folders or files)
        for (const auto& asset : assets)
        {
            ShowAssetFolder(asset);
        }

        // End the assets window
        ImGui::End();
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

    void Editor::Update(VkDescriptorSet set) {
        // Sample asset hierarchy (could be loaded from file)
        std::vector<Asset> assets = {
            {"Folder 1", true, {{"Asset 1", false}, {"Asset 2", false}}},
            {"Folder 2", true, {{"Asset 3", false}, {"Folder 2.1", true, {{"Asset 4", false}}}}},
            {"Asset 5", false}
        };

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        //{
        //    // Set the window to be docked at the top
        //    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        //        ImGuiWindowFlags_NoCollapse /* | ImGuiWindowFlags_NoDocking*/ | ImGuiWindowFlags_AlwaysAutoResize;

        //    // Adjust the position to the top
        //    ImGui::SetNextWindowPos(ImVec2(0, 0)); // Top-left corner
        //    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 30)); // Full width of the screen and a height of 30

        //    // Begin the top bar window
        //    ImGui::Begin("Top Bar", nullptr, windowFlags);

        //    // Create buttons for the top bar
        //    if (ImGui::Button("File"))
        //    {
        //        // Logic for File button click
        //    }

        //    ImGui::SameLine();
        //    if (ImGui::Button("Assets"))
        //    {
        //        // Logic for Assets button click
        //    }

        //    ImGui::SameLine();
        //    if (ImGui::Button("Edit"))
        //    {
        //        // Logic for Edit button click
        //    }

        //    // End the top bar window
        //    ImGui::End();
        //}

        ImGui::Begin("Scene");
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = glm::vec2(viewportSize.x, viewportSize.y);
        ImGui::Image((ImTextureID)set, ImVec2{ viewportSize.x, viewportSize.y });
        ImGui::End();

        // Display the Assets window
        ShowAssetsWindow(assets);

        ImGui::Begin("Test Image");
        ImGui::Image((ImTextureID)textureSet, ImVec2{ (float)m_Texture->GetWidth(), (float)m_Texture->GetHeight() });
        ImGui::End();

        {
            ImGui::Begin("Hierarchy");

            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();

            // Example items in the hierarchy
            if (ImGui::Selectable("Item 1")) {
                // Handle selection
            }
            if (ImGui::Selectable("Item 2")) {
                // Handle selection
            }

            // Check if right-click is inside the window and not on any item
            bool isHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
            bool isRightClick = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

            if (isHovered && isRightClick) {
                // Open context menu only if mouse is inside the window
                ImGui::OpenPopup("ContextMenu");
            }

            // Render the context menu
            if (ImGui::BeginPopup("ContextMenu")) {
                if (ImGui::MenuItem("Create Empty")) {
                    // Handle creating an empty item
                }
                if (ImGui::MenuItem("Create Element")) {
                    // Handle creating a new element
                }
                ImGui::EndPopup();
            }

            ImGui::End();
        }

        //ImGui::Begin("Heirarchy");
        //{
        //    for (int i = 0; i < Engine::m_Scene->objects.size(); i++) {
        //        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
        //        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        //        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)Engine::m_Scene->objects[i].id, flags, Engine::m_Scene->objects[i].name.c_str());
        //        if (ImGui::IsItemClicked())
        //        {
        //        }

        //        bool entityDeleted = false;
        //        if (ImGui::BeginPopupContextItem())
        //        {
        //            if (ImGui::MenuItem("Delete Entity"))
        //                entityDeleted = true;

        //            ImGui::EndPopup();
        //        }
        //        if (opened)
        //            ImGui::TreePop();
        //    }
        //}
        //ImGui::End();

        ImGui::Begin("Game");
        ImGui::End();

        ImGui::Begin("Console");
        ImGui::End();

        ImGui::Begin("Inspector");
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

        //ImGui::ShowDemoWindow();
        //ImGui::Begin("Debug");

        //if (ImGui::Button("Button"))
        //    counter++;
        //ImGui::SameLine();
        //ImGui::Text("counter = %d", counter);

        //ImGuiIO& io = ImGui::GetIO();
        //ImGui::Text("Mouse Pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
        //ImGui::Text("Mouse Left Click: %d", io.MouseDown[0]);
        //ImGui::Text("Mouse Right Click: %d", io.MouseDown[1]);

        //ImGui::GetIO();
        //ImGui::Text("Mouse Down: %d", io.MouseDown[0]);
        //ImGui::Text("Mouse Clicked: %d", ImGui::IsMouseClicked(0));
        //ImGui::Text("Mouse Dragging: %d", ImGui::IsMouseDragging(0));
        //ImGui::Text("Mouse Double Clicked: %d", ImGui::IsMouseDoubleClicked(0));


        ////ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        //ImGui::End();

        ImGui::Render();
    }

    void Editor::Release() {
        for (int i = 0; i < s_Panels.size(); i++)
        {
            s_Panels[i]->OnDestroy();
            delete s_Panels[i];
        }
        //ImGui::EndFrame();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

