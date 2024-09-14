#include "Editor.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/VulkanRenderer/RendererVulkan.hpp"

namespace Brisk {
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
	}

    void Editor::Update() {
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            {
                // Create a new window
                ImGui::Begin("Viewport");

                // Set the size and position of the viewport
                //ImVec2 viewportSize = ImGui::GetContentRegionAvail();
                //ImGui::Image((ImTextureID)viewportDescriptorSet, ImVec2{ viewportSize.x, viewportSize.y });

                ImGui::End();
            }

            //for (int i = 0; i < s_Panels.size(); i++)
            {
                //s_Panels[i]->OnUpdate();
            }

            ImGui::ShowDemoWindow();
            ImGui::Begin("Debug");

            if (ImGui::Button("Button"))
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGuiIO& io = ImGui::GetIO();
            ImGui::Text("Mouse Pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
            ImGui::Text("Mouse Left Click: %d", io.MouseDown[0]);
            ImGui::Text("Mouse Right Click: %d", io.MouseDown[1]);

            ImGui::GetIO();
            ImGui::Text("Mouse Down: %d", io.MouseDown[0]);
            ImGui::Text("Mouse Clicked: %d", ImGui::IsMouseClicked(0));
            ImGui::Text("Mouse Dragging: %d", ImGui::IsMouseDragging(0));
            ImGui::Text("Mouse Double Clicked: %d", ImGui::IsMouseDoubleClicked(0));


            //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();

            ImGui::Render();
            //ImGui_ImplVulkan_RenderDrawData(
            //    ImGui::GetDrawData(),
            //    static_cast<RendererVulkan*>(Engine::s_Renderer)->GetCommandBuffer(),
            //    static_cast<RendererVulkan*>(Engine::s_Renderer)->GetPipeline());
        }
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

