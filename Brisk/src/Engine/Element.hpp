#pragma once

#include "Module.hpp"

#include <unordered_map>
#include <string>
#include <memory>

namespace Brisk {
    class Element {
    public:
        uint64_t id;
        std::vector<int> children;
        std::string name;

        Element(uint64_t id) : id(id) {
            AddModule(new Transform());
        }

        template<typename T>
        void AddModule(T* module) {
            auto it = Modules.find(typeid(T).name());
            if (it != Modules.end())
                return; // already added    

            Modules[typeid(T).name()] = module;
        }

        template<typename T>
        T* GetComponent() {
            auto it = Modules.find(typeid(T).name());
            return (it != Modules.end()) ? static_cast<T*>(it->second.get()) : nullptr;
        }

        void UpdateComponents(float deltaTime) {
            for (auto& pair : Modules) {
                //if (auto module = dynamic_cast<IUpdatable*>(pair.second.get())) { // not efficient
                //    component->Update(deltaTime);
                //}
            }
        }

    private:
        std::unordered_map<std::string, Module*> Modules;
    };
}