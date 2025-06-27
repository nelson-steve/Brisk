#include "ConsolePanel.hpp"

#include "Core/Log.hpp"

namespace Brisk
{
    void ConsolePanel::OnCreate() {
    }

    void ConsolePanel::OnUpdate() {
        ImGui::Begin("Console");

        if (ImGui::Button("Clear"))
            Log::g_ImGuiSink->Clear();

        ImGui::Separator();
        ImGui::BeginChild("ConsoleScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        const auto& entries = Log::g_ImGuiSink->GetEntries();
        for (const auto& entry : entries) {
            ImVec4 color = ImVec4(1, 1, 1, 1);
            switch (entry.level) {
            case spdlog::level::trace:   color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); break;
            case spdlog::level::info:    color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
            case spdlog::level::warn:    color = ImVec4(1.0f, 1.0f, 0.3f, 1.0f); break;
            case spdlog::level::err:     color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break;
            case spdlog::level::critical:color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); break;
            default: break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(entry.message.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void ConsolePanel::OnDestroy() {
    }
}