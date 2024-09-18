#pragma once

#include "Element.hpp"

#include <vector>
#include <string>

namespace Brisk {
    class BriskScene {
    public:
        std::vector<Element> objects;
        int nextId = 0;

        int CreateElement(const std::string& name) {
            Element newObj{ nextId };
            newObj.id = nextId++;
            newObj.name = name;
            newObj.transform = { {0, 0, 0}, {0, 0, 0, 1}, {1, 1, 1} };
            objects.push_back(newObj);
            return newObj.id;
        }

        void AddChild(int parentId, int childId) {
            auto& parent = objects[parentId];
            parent.children.push_back(childId);
        }

        // Additional methods for removal, updating transforms, etc.
    };

}