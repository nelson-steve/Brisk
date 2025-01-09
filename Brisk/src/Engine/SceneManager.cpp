#include "SceneManager.hpp"

namespace Brisk
{
    std::shared_ptr<Scene> SceneManager::pActiveScene;

    SceneManager::SceneManager() {

    }

    void SceneManager::Init() {

    }

    void SceneManager::SetDefaultScene() {

    }

    void SceneManager::SwitchScene(std::shared_ptr<Scene> scene) {

    }

    bool SceneManager::IsSwitchingScene() {
        return false;
    }
}