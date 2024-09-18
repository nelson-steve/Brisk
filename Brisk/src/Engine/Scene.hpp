#pragma once

#include "Element.hpp"

#include <vector>
#include <string>

namespace Brisk {
    class BriskScene {
    public:
        std::vector<Element> Objects;
        int NextID = 0;

        int CreateElement(const std::string& name) {
            Element newElement{ NextID };
            newElement.id = NextID++;
            newElement.name = name + std::to_string(NextID);
            newElement.transform = { {0, 0, 0}, {0, 0, 0, 1}, {1, 1, 1} };
            Objects.push_back(newElement);
            return newElement.id;
        }

        void AddChild(int parentId, int childId) {
            CreateElement("Child");
            auto& parent = Objects[parentId];
            parent.children.push_back(childId);
        }

        // Additional methods for removal, updating transforms, etc.
    };

}