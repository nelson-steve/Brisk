#pragma once

#include "EditorPanel.hpp"

namespace Brisk {
    class ViewportPanel : public IEditorPanel {
    public:
        virtual void OnCreate() override;
        virtual void OnUpdate() override;
        virtual void OnDestroy() override;
    };
}