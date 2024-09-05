#pragma once

#include "EditorPanel.hpp"

namespace Brisk {
    class ViewportPanel : public IEditorPanel {
        virtual void OnCreate() override;
        virtual void OnUpdate() override;
        virtual void OnDestroy() override;
    };
}