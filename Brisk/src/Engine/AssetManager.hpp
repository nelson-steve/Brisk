#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <typeindex>
#include <typeinfo>
#include <functional>
#include <stdexcept>
#include <vector>

class Texture;
class Model;
class Shader;
class Sound;
class Animation;

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

    template<typename T>
    void UnloadAsset(const std::string& filePath)
    {
        auto it = assetMap.find(typeid(T));
        if (it != assetMap.end()) {
            it->second.erase(filePath);
            if (it->second.empty()) {
                assetMap.erase(it);
            }
        }
    }

    void ClearAssets()
    {
        assetMap.clear();
    }

    template<typename T>
    void RegisterCustomLoader(std::function<std::shared_ptr<T>(const std::string&)> loader)
    {
        customLoaders[typeid(T)] = [loader](const std::string& path) {
            return loader(path);
            };
    }

private:
    template<typename T>
    std::shared_ptr<T> LoadAssetFromFile(const std::string& filePath)
    {
        if (customLoaders.find(typeid(T)) != customLoaders.end()) {
            auto loader = std::any_cast<std::function<std::shared_ptr<T>(const std::string&)>>(customLoaders.at(typeid(T)));
            return loader(filePath);
        }
        return nullptr;
    }

    std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<void>>> assetMap;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>(const std::string&)>> customLoaders;
};

template<>
std::shared_ptr<Texture> AssetManager::LoadAssetFromFile<Texture>(const std::string& filePath)
{
    return std::make_shared<Texture>(filePath);
}

template<>
std::shared_ptr<Model> AssetManager::LoadAssetFromFile<Model>(const std::string& filePath)
{
    return std::make_shared<Model>(filePath);
}

template<>
std::shared_ptr<Shader> AssetManager::LoadAssetFromFile<Shader>(const std::string& filePath)
{
    return std::make_shared<Shader>(filePath);
}

template<>
std::shared_ptr<Sound> AssetManager::LoadAssetFromFile<Sound>(const std::string& filePath)
{
    return std::make_shared<Sound>(filePath);
}

template<>
std::shared_ptr<Animation> AssetManager::LoadAssetFromFile<Animation>(const std::string& filePath)
{
    return std::make_shared<Animation>(filePath);
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

class Sound {
public:
    Sound(const std::string& filePath) {
        std::cout << "Sound loaded from: " << filePath << std::endl;
    }
};

class Animation {
public:
    Animation(const std::string& filePath) {
        std::cout << "Animation loaded from: " << filePath << std::endl;
    }
};

class Asset {
public:
    virtual ~Asset() = default;
    virtual void Load() = 0;
    virtual void Unload() = 0;
};

class TextureAsset : public Asset {
public:
    void Load() override {
        std::cout << "Texture asset loaded" << std::endl;
    }

    void Unload() override {
        std::cout << "Texture asset unloaded" << std::endl;
    }
};

class ModelAsset : public Asset {
public:
    void Load() override {
        std::cout << "Model asset loaded" << std::endl;
    }

    void Unload() override {
        std::cout << "Model asset unloaded" << std::endl;
    }
};

class AssetManagerV2
{
public:
    template<typename T>
    std::shared_ptr<T> Load(const std::string& filePath, bool forceReload = false)
    {
        if (!forceReload && IsAssetLoaded<T>(filePath)) {
            return Get<T>(filePath);
        }

        std::shared_ptr<T> asset = LoadFromFile<T>(filePath);
        if (asset) {
            assetRegistry[typeid(T)][filePath] = asset;
            return asset;
        }

        return nullptr;
    }

    template<typename T>
    std::shared_ptr<T> Get(const std::string& filePath) const
    {
        auto it = assetRegistry.find(typeid(T));
        if (it != assetRegistry.end()) {
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
        auto it = assetRegistry.find(typeid(T));
        if (it != assetRegistry.end()) {
            return it->second.find(filePath) != it->second.end();
        }
        return false;
    }

    template<typename T>
    void Unload(const std::string& filePath)
    {
        auto it = assetRegistry.find(typeid(T));
        if (it != assetRegistry.end()) {
            it->second.erase(filePath);
            if (it->second.empty()) {
                assetRegistry.erase(it);
            }
        }
    }

private:
    template<typename T>
    std::shared_ptr<T> LoadFromFile(const std::string& filePath)
    {
        if (customLoaders.find(typeid(T)) != customLoaders.end()) {
            auto loader = std::any_cast<std::function<std::shared_ptr<T>(const std::string&)>>(customLoaders.at(typeid(T)));
            return loader(filePath);
        }
        return nullptr;
    }

    std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<void>>> assetRegistry;
    std::unordered_map<std::type_index, std::function<std::shared_ptr<void>(const std::string&)>> customLoaders;
};

template<>
std::shared_ptr<Texture> AssetManagerV2::LoadFromFile<Texture>(const std::string& filePath)
{
    return std::make_shared<Texture>(filePath);
}

template<>
std::shared_ptr<Model> AssetManagerV2::LoadFromFile<Model>(const std::string& filePath)
{
    return std::make_shared<Model>(filePath);
}

template<>
std::shared_ptr<Shader> AssetManagerV2::LoadFromFile<Shader>(const std::string& filePath)
{
    return std::make_shared<Shader>(filePath);
}

template<>
std::shared_ptr<Sound> AssetManagerV2::LoadFromFile<Sound>(const std::string& filePath)
{
    return std::make_shared<Sound>(filePath);
}

template<>
std::shared_ptr<Animation> AssetManagerV2::LoadFromFile<Animation>(const std::string& filePath)
{
    return std::make_shared<Animation>(filePath);
}
