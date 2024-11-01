#pragma once

#include <memory>
#include <vector>
#include <string>

namespace Brisk
{
    enum NodeType
    {
        Root,
        Sprite,
    }

    class Node
    {
    public:
        NodeType p_Type;
        uint32_t p_ID = 0;
        std::string p_Name;
        std::vector<std::shared_ptr<Node>> p_Children;
    }

    class Sprite : public Node
    {
    public:
        std::shared_ptr<Texture> p_Texture;
    }

    class BriskScene
    {
    public:
        void AddNode(const std::string &name, bool isRoot = true)
        {
            Node node{};
            node.p_Name = name;
            p_Nodes.push_back(node);
            return newElement.id;
        }

        void AddNode(Ref<Node> parentNode, std::string name)
        {
            Node node{};
            node.p_Name = name;
            parentNode->AddNode(node)
        }

        void SelectElement(Ref<Node> node)
        {
            p_SelectedNode = node;
        }

    private:
        std::shared_ptr<Node> p_SelectedNode;
        std::vector<std::shared_ptr<Node>> p_Nodes;
        uint32_t p_SceneID = 0;
    };
}