#pragma once

#include "IEditorPanel.hpp"

#include <string>
#include <vector>

namespace Brisk 
{
    class InspectorPanel : public IEditorPanel {
    public:
        virtual void OnCreate() override;
        virtual void OnUpdate() override;
        virtual void OnDestroy() override;
    };
}