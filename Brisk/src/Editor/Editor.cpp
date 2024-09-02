#include "Editor.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/VulkanRenderer/RendererVulkan.hpp"

namespace Brisk {
	void Editor::Create() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.WantCaptureMouse = true;
        //io.WantCaptureKeyboard = true;
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

            ImGui::ShowDemoWindow();
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            //ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            //ImGui::Checkbox("Another Window", &show_another_window);

            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
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

            ImGui::SetNextWindowFocus();

            ImGui::Render();
            //ImGui_ImplVulkan_RenderDrawData(
            //    ImGui::GetDrawData(),
            //    static_cast<RendererVulkan*>(Engine::s_Renderer)->GetCommandBuffer(),
            //    static_cast<RendererVulkan*>(Engine::s_Renderer)->GetPipeline());
        }
    }
}