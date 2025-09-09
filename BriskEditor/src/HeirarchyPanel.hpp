#pragma once

#include "IEditorPanel.hpp"
#include "Engine/Entity.hpp"

namespace Brisk 
{
    class HeirarchyPanel : public IEditorPanel {
    public:
        void DrawEntityNode(Entity entity);

        virtual void OnCreate() override;
        virtual void OnUpdate() override;
        virtual void OnDestroy() override;
    };
}