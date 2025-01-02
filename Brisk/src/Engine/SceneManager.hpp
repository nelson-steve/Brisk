#pragma once

#include "Engine/Scene.hpp"

#include <memory>

namespace Brisk
{
    class SceneManager {
    public:
        SceneManager();
        void Init();
        void SetDefaultScene();
        void SwitchScene(std::shared_ptr<Scene> scene);
        bool IsSwitchingScene();

        SceneManager& Instance(){return s_Instance;}
    private:
        std::shared_ptr<Scene> m_ActiveScene;

        SceneManager s_Instance;
    };
}