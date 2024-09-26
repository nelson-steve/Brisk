#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <typeindex>
#include <typeinfo>

class Texture;
class Model;
class Shader;

class AssetManager
{
public:
    template<typename T>
    std::shared_ptr<T> LoadAsset(const std::string& filePath)
    {
        if (IsAssetLoaded<T>(filePath)) {
            return GetAsset<T>(filePath);
        }

        std::shared_ptr<T> asset = LoadAssetFromFile<T>(filePath);
        if (asset) {
            assetMap[typeid(T)][filePath] = asset;
            return asset;
        }

        return nullptr;
    }

    template<typename T>
    std::shared_ptr<T> GetAsset(const std::string& filePath) const
    {
        auto it = assetMap.find(typeid(T));
        if (it != assetMap.end()) {
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
        auto it = assetMap.find(typeid(T));
        if (it != assetMap.end()) {
            return it->second.find(filePath) != it->second.end();
        }
        return false;
    }

    void ClearAssets()
    {
        assetMap.clear();
    }

private:
    template<typename T>
    std::shared_ptr<T> LoadAssetFromFile(const std::string& filePath)
    {
        return nullptr;
    }

    std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<void>>> assetMap;
};

template<>
std::shared_ptr<Texture> AssetManager::LoadAssetFromFile<Texture>(const std::string& filePath)
{
    return nullptr;
}

template<>
std::shared_ptr<Model> AssetManager::LoadAssetFromFile<Model>(const std::string& filePath)
{
    return nullptr;
}

template<>
std::shared_ptr<Shader> AssetManager::LoadAssetFromFile<Shader>(const std::string& filePath)
{
    return nullptr;
}

class Texture {
public:
    Texture(const std::string& filePath) {
        std::cout << "Texture loaded from: " << filePath << std::endl;
    }
};

class Model {
public:
    Model(const std::string& filePath) {
        std::cout << "Model loaded from: " << filePath << std::endl;
    }
};

class Shader {
public:
    Shader(const std::string& filePath) {
        std::cout << "Shader loaded from: " << filePath << std::endl;
    }
};
