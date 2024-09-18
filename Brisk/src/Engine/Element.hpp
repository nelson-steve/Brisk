#pragma once

#include "Module.hpp"

#include <unordered_map>
#include <string>
#include <memory>

namespace Brisk {
    struct Vector3 {
        float x, y, z;
    };

    struct Quaternion {
        float x, y, z, w; // Simple representation of rotation
    };

    struct Transform {
        Vector3 position;
        Quaternion rotation;
        Vector3 scale;
    };

    class Element {
    public:
        int id;
        Transform transform;
        std::vector<int> children;
        std::string name;

        Element(int id) : id(id), transform({ {0, 0, 0}, {0, 0, 0, 1}, {1, 1, 1} }) {}

        //template<typename T>
        //void AddComponent(std::unique_ptr<T> component) {
        //    components[typeid(T).name()] = std::move(component);
        //}

        //template<typename T>
        //T* GetComponent() {
        //    auto it = components.find(typeid(T).name());
        //    return (it != components.end()) ? static_cast<T*>(it->second.get()) : nullptr;
        //}

        //void UpdateComponents(float deltaTime) {
        //    for (auto& pair : components) {
        //        //if (auto component = dynamic_cast<IUpdatable*>(pair.second.get())) {
        //        //    component->Update(deltaTime);
        //        //}
        //    }
        //}

    private:
        //std::unordered_map<std::string, std::unique_ptr<Module>> components;
    };
}