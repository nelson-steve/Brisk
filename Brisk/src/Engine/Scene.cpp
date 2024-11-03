#include "Scene.hpp"

#include <algorithm>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

namespace Brisk {

    // Node constructor
    Node::Node(const std::string& name)
        : name(name), position(0.0f), rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), scale(1.0f) {
    }

    // Node destructor
    Node::~Node() {
        // Destructor logic if needed
    }

    // Add a child node
    void Node::AddChild(std::shared_ptr<Node> child) {
        if (child) {
            child->parent = shared_from_this();
            children.push_back(child);
        }
    }

    // Remove a child node
    void Node::RemoveChild(const std::shared_ptr<Node>& child) {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
        if (child) {
            child->parent.reset();
        }
    }

    // Get the parent node
    std::shared_ptr<Node> Node::GetParent() const {
        return parent.lock();
    }

    // Set position
    void Node::SetPosition(const glm::vec3& position) {
        this->position = position;
    }

    // Get position
    glm::vec3 Node::GetPosition() const {
        return position;
    }

    // Set rotation
    void Node::SetRotation(const glm::quat& rotation) {
        this->rotation = rotation;
    }

    // Get rotation
    glm::quat Node::GetRotation() const {
        return rotation;
    }

    // Set scale
    void Node::SetScale(const glm::vec3& scale) {
        this->scale = scale;
    }

    // Get scale
    glm::vec3 Node::GetScale() const {
        return scale;
    }

    // Add component
    template <typename T, typename... Args>
    std::shared_ptr<T> Node::AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
        auto component = std::make_shared<T>(std::forward<Args>(args)...);
        components.push_back(component);
        return component;
    }

    // Get component
    template <typename T>
    std::shared_ptr<T> Node::GetComponent() const {
        for (const auto& component : components) {
            if (auto casted = std::dynamic_pointer_cast<T>(component)) {
                return casted;
            }
        }
        return nullptr;
    }

} // namespace Brisk
