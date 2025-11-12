#pragma once

// INCLUDES
#include "Core/Core.hpp"
#include "Engine/MeshAsset.hpp"
#include "Entity.hpp"
//---------------------------
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
//-------------------------------
#include <string>
//---------------

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
		std::string name = "Transform Component";

		bool dirtyTransform = false;
		uint32_t p_TransformIndex;

		Entity parent;
		std::vector<Entity> children;

		const glm::vec3 &GetPosition() const { return Position; }
		const glm::quat &GetRotation() const { return Rotation; }
		const glm::vec3 &GetScale() const { return Scale; }

		void AddTranform(const glm::vec3 &pos, const glm::vec3 &rot, const glm::vec3 &scale)
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(rot));
			Position = pos;
			Rotation = rot;
			Scale = scale;
		}

		glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		TransformComponent(const glm::vec3 &position)
			: Position(position) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::eulerAngleYXZ(glm::radians(Rotation.y), glm::radians(Rotation.x), glm::radians(Rotation.z));
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), Position);
			glm::mat4 scaling = glm::scale(glm::mat4(1.0f), Scale);

			return translation * rotation * scaling;
		}
	};

	struct MeshComponent
	{
		std::string name = "Mesh Component";

		std::vector<uint32_t> p_SubMeshIndices;
		std::string p_Name;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
	};

	struct MaterialComponent
	{
		std::string name = "Material Component";

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent &) = default;
	};

	enum LightType
	{
		Point,
		Directional,
		Spot
	};

	struct DirectionalLightComponent
	{
		glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 Color = glm::vec3(1.0);
	
		LightType Type = LightType::Directional;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};

	struct PointLightComponent
	{
		glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Color = glm::vec3(1.0);
		float Intensity;
		float Radius;
		float Constant;
		float Linear;
		float Quadratic;

		LightType Type = LightType::Point;

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	struct SpotLightComponent
	{
		glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 Color = glm::vec3(1.0);
		float InnerCutoff;
		float OuterCutoff;
		float Constant;
		float Linear;
		float Quadratic;

		LightType Type = LightType::Spot;

		SpotLightComponent() = default;
		SpotLightComponent(const SpotLightComponent&) = default;
	};

	struct SkyboxComponent
	{
		int value;

		SkyboxComponent() = default;
		SkyboxComponent(const SkyboxComponent &) = default;
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
					   DirectionalLightComponent,
					   PointLightComponent,
					   SpotLightComponent,
					   SkyboxComponent,
					   PhysicsComponent,
					   BoxColliderComponent,
					   SphereColliderComponent,
					   CapsuleColliderComponent,
					   TagComponent,
					   MaterialComponent,
					   ScriptComponent>;

}