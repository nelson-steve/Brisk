#include "ViewportPanel.hpp"

namespace Brisk {
    void ViewportPanel::OnCreate() {
    }

    void ViewportPanel::OnUpdate(){
        ImGui::Begin("Hierarchy");
        // Assume Scene contains a list of root objects

        ImGui::End();
    }

    void ViewportPanel::OnDestroy(){
        
    }
}