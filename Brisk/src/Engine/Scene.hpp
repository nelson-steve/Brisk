#pragma once

#include "Element.hpp"

#include <vector>
#include <string>

namespace Brisk 
{
    class BriskScene {
    public:
        int32_t SelectedElement = -1;
        std::vector<Element> Elements;
        int32_t p_ID = -1;
        int32_t CreateElement(const std::string& name, bool isRoot = true) {
            p_ID++;
            Element newElement{ p_ID };
            newElement.id = p_ID;
            newElement.name = name + std::to_string(p_ID);
            newElement.IsRoot = isRoot;
            Elements.push_back(newElement);
            return newElement.id;
        }

        int32_t AddChildElement(int32_t parentId) {
            CreateElement("Child", false);
            auto& parent = Elements[parentId];
            parent.children.push_back(p_ID);
            return p_ID;
        }

        int32_t AddChildElement(const Element& element) {
            CreateElement("Child", false);
            auto& parent = Elements[element.id];
            parent.children.push_back(p_ID);
            return p_ID;
        }

        void SelectElement(int32_t i) {
            SelectedElement = i;
        }
    };

}