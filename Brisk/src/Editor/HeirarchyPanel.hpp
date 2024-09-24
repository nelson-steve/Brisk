#pragma once

#include "IEditorPanel.hpp"

namespace Brisk 
{
    class HeirarchyPanel : public IEditorPanel {
    public:
        virtual void OnCreate() override;
        virtual void OnUpdate() override;
        virtual void OnDestroy() override;
    };
}