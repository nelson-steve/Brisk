#pragma once

#include "Renderer/Texture.hpp"
#include "Engine/MeshAsset.hpp"

#include <typeindex>

namespace Brisk {

    class Shader;
    class Sound;

    class AssetManager
    {
    public:
        template<typename T>
        std::shared_ptr<T> LoadAsset(const std::string& filePath, bool forceReload = false)
        {
            if (!forceReload && IsAssetLoaded<T>(filePath)) {
                return GetAsset<T>(filePath);
            }

            std::shared_ptr<T> asset = LoadAssetFromFile<T>(filePath);
            if (asset) {
                m_AssetMap[typeid(T)][filePath] = asset;
                return asset;
            }

            return nullptr;
        }

        template<typename T>
        std::shared_ptr<T> GetAsset(const std::string& filePath) const
        {
            auto it = m_AssetMap.find(typeid(T));
            if (it != m_AssetMap.end()) {
                auto assetIt = it->second.find(filePath);
                if (assetIt != it->second.end()) {
                    return std::static_pointer_cast<T>(assetIt->second);
                }
            }
            return nullptr;
        }

        template<typename T>
        bool IsAssetLoaded(const std::string& filePath) const
        {
            auto it = m_AssetMap.find(typeid(T));
            if (it != m_AssetMap.end()) {
                return it->second.find(filePath) != it->second.end();
            }
            return false;
        }

        template<typename T>
        void UnloadAsset(const std::string& filePath)
        {
            auto it = m_AssetMap.find(typeid(T));
            if (it != m_AssetMap.end()) {
                it->second.erase(filePath);
                if (it->second.empty()) {
                    m_AssetMap.erase(it);
                }
            }
        }

        void ClearAssets()
        {
            //auto it = m_AssetMap.find(typeid(MeshAsset));
            //if (it != m_AssetMap.end()) {
            //    for (auto& [name, assetPtr] : it->second) {
            //        // Cast back to MeshAsset
            //        auto mesh = std::static_pointer_cast<MeshAsset>(assetPtr);
            //        if (mesh) {
            //            mesh->Release(); // or mesh->Destroy(), etc.
            //        }
            //    }
            //}


            m_AssetMap.clear();
        }

        template<typename T>
        void RegisterCustomLoader(std::function<std::shared_ptr<T>(const std::string&)> loader)
        {
            customLoaders[typeid(T)] = [loader](const std::string& path) -> std::shared_ptr<void> {
                return loader(path);
                };
        }

    private:
        template<typename T>
        std::shared_ptr<T> LoadAssetFromFile(const std::string& filePath)
        {
            auto it = customLoaders.find(typeid(T));
            if (it != customLoaders.end()) {
                return std::static_pointer_cast<T>(it->second(filePath));
            }
            return nullptr;
        }

        std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<void>>> m_AssetMap;
        std::unordered_map<std::type_index, std::function<std::shared_ptr<void>(const std::string&)>> customLoaders;
    };

    //template<>
    //inline std::shared_ptr<MeshAsset> AssetManager::LoadAssetFromFile<MeshAsset>(const std::string& filePath)
    //{
    //    auto asset = std::make_shared<MeshAsset>();
    //    asset->Load(filePath);
    //    return asset;
    //}
}
