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
    public:
        std::string meshFile;  // Path to mesh file (could be .obj, .fbx, etc.)
        std::shared_ptr<Model> model;  // Loaded mesh data
        bool isVisible = true;

        MeshComponent() = default;
        MeshComponent(const std::string& filePath) : meshFile(filePath) {
            // Load the mesh from the file, or set to nullptr if failed
            loadMesh(filePath);
        }

        void loadMesh(const std::string& filePath) {
            // For example, load the mesh from a file (the Mesh class would need to be defined elsewhere)
            meshFile = filePath;
            //mesh = MeshLoader::load(filePath);  // Placeholder for a mesh loading function
        }

        void toggleVisibility() { isVisible = !isVisible; }
    };

    class PhysicsComponent : public Component {
    public:
        glm::vec3 velocity{ 0.0f, 0.0f, 0.0f };
        glm::vec3 acceleration{ 0.0f, 0.0f, 0.0f };
        glm::vec3 force{ 0.0f, 0.0f, 0.0f };
        glm::vec3 lastPosition{ 0.0f, 0.0f, 0.0f };
        float mass = 1.0f;  // Default mass for physics simulations

        PhysicsComponent() = default;

        void applyForce(const glm::vec3& f) { force += f; }
        void clearForces() { force = glm::vec3(0.0f); }

        void update(float deltaTime) {
            // Basic physics: F = ma -> acceleration = force / mass
            acceleration = force / mass;
            velocity += acceleration * deltaTime;
            lastPosition = velocity * deltaTime;
        }

        glm::vec3 getVelocity() const { return velocity; }
    };

    class AudioComponent : public Component {
    public:
        std::string soundFile;  // Path to sound file (e.g., .wav, .mp3)
        bool isLooping = false;
        float volume = 1.0f;     // 0.0 (muted) to 1.0 (full volume)
        float pitch = 1.0f;      // 1.0 is normal pitch

        AudioComponent(const std::string& filePath, bool loop = false, float vol = 1.0f, float ptch = 1.0f)
            : soundFile(filePath), isLooping(loop), volume(vol), pitch(ptch) {}

        void play() {
            // Logic to play the sound using the sound file, volume, pitch, and loop settings
        }

        void stop() {
            // Logic to stop the sound
        }

        void setVolume(float vol) { volume = vol; }
        void setPitch(float ptch) { pitch = ptch; }
        void setLooping(bool loop) { isLooping = loop; }
    };

    class AnimationComponent : public Component {
    public:
    //    std::vector<std::shared_ptr<Animation>> animations;  // List of animations
    //    std::shared_ptr<Animation> currentAnimation;  // Currently playing animation
    //    float speed = 1.0f;  // Speed multiplier for animation
    //    bool isPlaying = false;

    //    AnimationComponent() = default;

    //    void playAnimation(const std::string& animationName) {
    //        // Find animation by name and start playing
    //        auto anim = findAnimationByName(animationName);
    //        if (anim) {
    //            currentAnimation = anim;
    //            isPlaying = true;
    //        }
    //    }

    //    void stopAnimation() {
    //        isPlaying = false;
    //    }

    //    void update(float deltaTime) {
    //        if (isPlaying && currentAnimation) {
    //            currentAnimation->update(deltaTime * speed);
    //        }
    //    }

    //private:
    //    std::shared_ptr<Animation> findAnimationByName(const std::string& name) {
    //        for (const auto& anim : animations) {
    //            if (anim->getName() == name) {
    //                return anim;
    //            }
    //        }
    //        return nullptr;
    //    }
    };

    class ParticleSystemComponent : public Component {
    public:
        //std::shared_ptr<ParticleSystem> particleSystem;  // Particle system for handling particles
        bool isActive = true;

        //ParticleSystemComponent() {
        //    // Initialize the particle system
        //    particleSystem = std::make_shared<ParticleSystem>();
        //}

        //void setActive(bool active) {
        //    isActive = active;
        //    if (isActive) {
        //        particleSystem->start();
        //    }
        //    else {
        //        particleSystem->stop();
        //    }
        //}

        //void update(float deltaTime) {
        //    if (isActive) {
        //        particleSystem->update(deltaTime);
        //    }
        //}

        //void emitParticles(int count) {
        //    if (isActive) {
        //        particleSystem->emit(count);
        //    }
        //}
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


        std::vector<Node> m_Nodes;
    };
}