#pragma once

#include "Core/Core.hpp"
#include "Engine/Model.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <string>
#include "Renderer/Shader.hpp"

namespace Brisk
{
	struct TagComponent
	{
		std::string name = "Tag Component";
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent &) = default;
		TagComponent(const std::string &tag)
			: Tag(tag) {}
	};

	struct TransformComponent
	{
		std::string name = "Transform Compnent";

		const glm::vec3 &GetPosition() const { return Position; }
		const glm::vec3 &GetRotation() const { return Rotation; }
		const glm::vec3 &GetScale() const { return Scale; }

		void AddTranform(const glm::vec3 &pos)
		{
			Position = pos;
		}

		void AddTranform(const glm::vec4 &rot)
		{
			Rotation = rot;
		}

		void AddTranform(const glm::vec3 &pos, const glm::vec4 &rot)
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(rot));
			Position = pos;
			Rotation = rot;
		}

		void AddTranform(const glm::vec3 &pos, const glm::vec4 &rot, const glm::vec3 &scale)
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(rot));
			Position = pos;
			Rotation = rot;
			Scale = scale;
		}

		glm::vec3 Position = {0.0f, 0.0f, 0.0f};
		glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
		glm::vec3 Scale = {1.0f, 1.0f, 1.0f};

		TransformComponent() = default;
		TransformComponent(const TransformComponent &) = default;

		TransformComponent(const glm::vec3 &position)
			: Position(position) {}

		// void SetPosition(const glm::mat4& pos) { Position = pos; }

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));
			return glm::translate(glm::mat4(1.0f), Position) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct MeshComponent
	{
		std::string name = "Model Component";

		Ref<Mesh> pModel;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
	};

	struct MaterialComponent
	{
		std::string name = "Material Component";

		std::vector<std::shared_ptr<Shader>> pMaterials;

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent &) = default;
	};

	struct LightComponent
	{
		enum LightType
		{
			Point,
			Directional,
			Spot
		};

		LightType type = LightType::Directional;

		glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f);
		glm::vec3 position = glm::vec3(1.0);
		glm::vec3 ambient = glm::vec3(0.05f, 0.05f, 0.05f);
		glm::vec3 diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
		glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);

		float cutOff = 0.0;
		float outerCutOff = 0.0;
		float constant = 0.0;
		float linear = 0.0;
		float quadratic = 0.0;

		std::string nameOfLight = "Directional";
		// glm::vec3 color = glm::vec3(1.0);

		LightComponent() = default;
		LightComponent(const LightComponent &) = default;
	};

	struct SkyboxComponent
	{
		SkyboxComponent() = default;
		SkyboxComponent(const SkyboxComponent &) = default;
	};

	// Physics

	struct Physics2DComponent
	{
		enum class BodyType
		{
			Static = 0,
			Dynamic,
			Kinematic
		};
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;
		bool Gravity = false;

		// Storage for runtime
		void *RuntimeBody = nullptr;

		Physics2DComponent() = default;
		Physics2DComponent(const Physics2DComponent &) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = {0.0f, 0.0f};
		glm::vec2 Size = {0.5f, 0.5f};

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void *RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent &) = default;
	};

	struct PhysicsComponent
	{
		std::string name = "Physics Compnent";
		enum class BodyType
		{
			Static,
			Dynamic
		};
		enum class RigidBodyType
		{
			Static,
			Kinematic,
			Dynamic
		};

		BodyType bodyType = BodyType::Static;
		RigidBodyType rigidBodyType = RigidBodyType::Static;

		std::string bodyTypeName = "Static";
		std::string rigidBodyTypeName = "Static";

		// states
		bool enableGravity = true;
		// Body
		// Ref<StaticBody> StaticRuntimeBody = nullptr;
		// Ref<DynamicBody> DynamicRuntimeBody = nullptr;

		PhysicsComponent() = default;
		PhysicsComponent(const PhysicsComponent &) = default;
	};

	struct BoxColliderComponent
	{
		glm::vec3 offset{0.0f};
		glm::vec3 Size{1.0f}; // TODO: not a half size

		// Ref<BoxCollider> collider;

		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent &) = default;
	};

	struct SphereColliderComponent
	{
		float radius;

		// Ref<SphereCollider> collider;

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent &) = default;
	};

	struct CapsuleColliderComponent
	{
		float radius;
		float height;

		// Ref<CapsuleCollider> collider;

		CapsuleColliderComponent() = default;
		CapsuleColliderComponent(const CapsuleColliderComponent &) = default;
	};

	struct SpriteRendererComponent
	{
		std::string name = "Sprite Renderer Compnent";

		glm::vec4 Color{1.0f, 1.0f, 1.0f, 1.0f};

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent &) = default;
		SpriteRendererComponent(const glm::vec4 &color)
			: Color(color) {}
	};

	struct CameraComponent
	{
		std::string name = "Camera Compnent";

		// Ref<SceneCamera> Camera;
		bool Primary = true; // TODO: think about moving to Scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent &) = default;
	};

	struct ScriptComponent
	{
		std::string name = "Script Compnent";

		std::string ClassName = "Sandbox.Player";

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent &) = default;
	};

	template <typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<MeshComponent,
					   TransformComponent,
					   LightComponent,
					   SkyboxComponent,
					   Physics2DComponent,
					   BoxCollider2DComponent,
					   PhysicsComponent,
					   BoxColliderComponent,
					   SphereColliderComponent,
					   CapsuleColliderComponent,
					   TagComponent,
					   MaterialComponent,
					   ScriptComponent>;

}