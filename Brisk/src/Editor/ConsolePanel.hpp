#pragma once

#include "IEditorPanel.hpp"

#include <string>
#include <vector>

namespace Brisk 
{
    class ConsolePanel : public IEditorPanel {
    public:
        virtual void OnCreate() override;
        virtual void OnUpdate() override;
        virtual void OnDestroy() override;

        void Clear();
        void AddLog(const char* fmt, ...);
    private:
        ImGuiTextBuffer Buf;
        ImVector<int> LineOffsets;
        bool AutoScroll = true;
    };
}