#include "ViewportPanel.hpp"

namespace Brisk 
{
    void ViewportPanel::OnCreate() {

    }

    void ViewportPanel::OnUpdate(){
            // Create a new window
        ImGui::Begin("Viewport");

        // Set the size and position of the viewport
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();

        ImGui::End();
    }

    void ViewportPanel::OnDestroy() {
        
    }
}