#pragma once

// INCLUDES
#include "Engine/Scene.hpp"
//------------------------
#include <memory>
//--------------

namespace Brisk
{
    class SceneManager {
    public:
        SceneManager();
        void Init();
        void SetDefaultScene();
        void SwitchScene(std::shared_ptr<Scene> scene);
        bool IsSwitchingScene();
        
        static std::shared_ptr<Scene> pActiveScene;

        static SceneManager s_Instance;
    };
}