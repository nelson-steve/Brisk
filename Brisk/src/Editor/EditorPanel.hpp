#pragma once

#include "imgui.h"

namespace Brisk {
    class IEditorPanel {
    public:
        virtual void OnCreate() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnDestroy() = 0;
    protected:
        bool m_Disabled;
        bool m_Hidden;
        bool m_Dragging;
    };
}