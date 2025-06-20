// INCLUDES
#include "SceneManager.hpp"
//------------------------
#include "Entity.hpp"
//------------------

namespace Brisk
{
    std::shared_ptr<Scene> SceneManager::pActiveScene;

    SceneManager::SceneManager() {

    }

    void SceneManager::Init() {
        pActiveScene = std::make_shared<Scene>();
    }

    void SceneManager::SetDefaultScene() {

    }

    void SceneManager::SwitchScene(std::shared_ptr<Scene> scene) {

    }

    bool SceneManager::IsSwitchingScene() {
        return false;
    }
}