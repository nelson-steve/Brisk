#pragma once

#include "Element.hpp"

#include <vector>
#include <string>

namespace Brisk {
    class BriskScene {
    public:
        std::vector<Element> Elements;
        uint64_t p_ID = 0;
        int CreateElement(const std::string& name) {
            Element newElement{ p_ID };
            newElement.id = p_ID;
            newElement.name = name + std::to_string(p_ID);
            Elements.push_back(newElement);
            p_ID++;
            return newElement.id;
        }

        void AddChild(int parentId) {
            CreateElement("Child");
            auto& parent = Elements[parentId];
            parent.children.push_back(p_ID);
        }

        void AddChild(const Element& element) {
            CreateElement("Child");
            auto& parent = Elements[element.id];
            parent.children.push_back(p_ID);
        }

        // Additional methods for removal, updating transforms, etc.
    };

}

/*

RenderManager {
Renderer* renderer;

SetupGraphicsPipeline();
RenderCube(){
renderer->render();
assign resources and render the cube
}
ApplyPostProcessing(){
Take input from previos funciotn and apply post processing
}


}

*/