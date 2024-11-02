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
    };

    class Component {
    };

    class MeshComponent : public Component {
    };

    class PhysicsComponent : public Component {
    };


    class Node {
    public:
        // Constructor and Destructor
        Node(const std::string& name = "Node");
        virtual ~Node();

        // Node Hierarchy Management
        void AddChild(std::shared_ptr<Node> child);
        void RemoveChild(const std::shared_ptr<Node>& child);
        std::shared_ptr<Node> GetParent() const;

        // Transform Functions
        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::quat& rotation);
        void SetScale(const glm::vec3& scale);
        glm::vec3 GetPosition() const;
        glm::quat GetRotation() const;
        glm::vec3 GetScale() const;

        // Component Management
        template <typename T, typename... Args>
        std::shared_ptr<T> AddComponent(Args&&... args);

        template <typename T>
        std::shared_ptr<T> GetComponent() const;

    private:
        std::string name;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

        std::weak_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> children;
        std::vector<std::shared_ptr<Component>> components; // Base class for components
    };

}