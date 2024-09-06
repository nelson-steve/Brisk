#include "HeirarchyPanel.hpp"

namespace Brisk {
    void HeirarchyPanel::OnCreate() {
    }

    void HeirarchyPanel::OnUpdate(){
        ImGui::Begin("Hierarchy");
        // Assume Scene contains a list of root objects

        ImGui::End();
    }

    void HeirarchyPanel::OnDestroy(){
        
    }
}