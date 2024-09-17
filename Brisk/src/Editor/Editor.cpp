#include "Editor.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/VulkanRenderer/RendererVulkan.hpp"

namespace Brisk {
    TextureVulkan* m_Texture;
    VkDescriptorSet textureSet;
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
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();
        
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

    void Editor::Update(VkDescriptorSet set) {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        ImGui::Begin("Scene");
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        ImGui::Image((ImTextureID)set, ImVec2{ (float)m_Texture->GetWidth(), (float)m_Texture->GetHeight() });
        ImGui::End();

        ImGui::Begin("Test Image");
        ImGui::Image((ImTextureID)textureSet, ImVec2{ (float)m_Texture->GetWidth(), (float)m_Texture->GetHeight() });
        ImGui::End();

        ImGui::Begin("Heirarchy");
        ImGui::End();

        ImGui::Begin("Game");
        ImGui::End();

        ImGui::Begin("Console");
        ImGui::End();

        ImGui::Begin("Inspector");
        ImGui::End();

        ImGui::Begin("Assets");
        ImGui::End();

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

