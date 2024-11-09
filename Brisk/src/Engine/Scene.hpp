#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace Brisk
{
    enum NodeType
    {
        Root,
        Sprite,
    };

    class Component {
    };

    class TransformComponent : public Component {
    public:
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f }; // Quaternion for rotation
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;

        // Methods for transforming the component
        void setPosition(const glm::vec3& pos) { position = pos; }
        void setRotation(const glm::quat& rot) { rotation = rot; }
        void setScale(const glm::vec3& scl) { scale = scl; }

        glm::mat4 getTransformMatrix() const {
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 rot = glm::mat4_cast(rotation);
            glm::mat4 scl = glm::scale(glm::mat4(1.0f), scale);
            return trans * rot * scl;
        }
    };

    class LightComponent : public Component {
    public:
        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
        bool enabled = true;

        LightComponent(const glm::vec3& col = { 1.0f, 1.0f, 1.0f }, float intens = 1.0f)
            : color(col), intensity(intens) {}

        void setColor(const glm::vec3& col) { color = col; }
        void setIntensity(float intens) { intensity = intens; }
        void toggle() { enabled = !enabled; }
    };

    class CameraComponent : public Component {
    public:
        float fov = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::vec3 lookAt{ 0.0f, 0.0f, -1.0f };
        glm::vec3 up{ 0.0f, 1.0f, 0.0f };

        CameraComponent() = default;

        glm::mat4 getViewMatrix() const {
            return glm::lookAt(position, position + lookAt, up);
        }

        glm::mat4 getProjectionMatrix(float aspectRatio) const {
            return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }
    };

    class ScriptComponent : public Component {
    public:
        std::string scriptName;
        std::function<void()> onUpdate;

        ScriptComponent(const std::string& name) : scriptName(name) {}

        void setUpdateFunction(std::function<void()> func) {
            onUpdate = func;
        }

        void update() {
            if (onUpdate) {
                onUpdate();  // Calls the update function if it exists
            }
        }
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


    class Scene {
    public:
        void AddNode();
        void RemoveNode(int index);
    private:
        std::vector<Node> m_Nodes;
    };
}