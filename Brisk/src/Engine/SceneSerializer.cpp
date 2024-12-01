#include "SceneSerializer.hpp"
#include "Entity.hpp"
#include "Component.hpp"

#include <fstream>
#include <glm/glm.hpp>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

}

namespace Albedo {

#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.GetValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		fieldInstance.SetValue(data);                  \
		break;                                         \
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static std::string RigidBody2DBodyTypeToString(Physics2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Physics2DComponent::BodyType::Static:    return "Static";
		case Physics2DComponent::BodyType::Dynamic:   return "Dynamic";
		case Physics2DComponent::BodyType::Kinematic: return "Kinematic";
		}

		Albedo_CORE_ASSERT(false, "Unknown body type");
		return {};
	}

	static Physics2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return Physics2DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic")   return Physics2DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Physics2DComponent::BodyType::Kinematic;

		Albedo_CORE_ASSERT(false, "Unknown body type");
		return Physics2DComponent::BodyType::Static;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene> scene)
		:m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << std::to_string((uint32_t)entity.GetEntityHandle()); // TODO: Entity ID goes here

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<ModelComponent>())
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap;
			auto& mc = entity.GetComponent<ModelComponent>();
			//out << YAML::Key << "Color" << YAML::Value << mc.Color;

			auto& mesh = mc.m_Model;
			out << YAML::Key << "Mesh" << YAML::Value;
			out << YAML::BeginMap;
			//out << YAML::Key << "Name" << YAML::Value << mesh->GetName();
			//out << YAML::Key << "Path" << YAML::Value << mesh->GetPath();
			out << YAML::EndMap;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<LightComponent>())
		{
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap;
			auto& lc = entity.GetComponent<LightComponent>();

			out << YAML::Key << "Type" << YAML::Value << lc.type;
			out << YAML::Key << "Direction" << YAML::Value << lc.direction;
			out << YAML::Key << "Position" << YAML::Value << lc.position;
			out << YAML::Key << "Ambient" << YAML::Value << lc.ambient;
			out << YAML::Key << "Diffuse" << YAML::Value << lc.diffuse;
			out << YAML::Key << "Specular" << YAML::Value << lc.specular;
			out << YAML::Key << "Cut Off" << YAML::Value << lc.cutOff;
			out << YAML::Key << "Outer Cut Off" << YAML::Value << lc.outerCutOff;
			out << YAML::Key << "Constant" << YAML::Value << lc.constant;
			out << YAML::Key << "Linear" << YAML::Value << lc.linear;
			out << YAML::Key << "Quadratic" << YAML::Value << lc.quadratic;
			out << YAML::Key << "Name Of Light" << YAML::Value << lc.nameOfLight;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<SkyboxComponent>())
		{
			out << YAML::Key << "SkyboxComponent";
			out << YAML::BeginMap;
			auto& skyc = entity.GetComponent<SkyboxComponent>();
			int i = 1;
		}

		if (entity.HasComponent<TextureComponent>())
		{
			out << YAML::Key << "TextureComponent";
			out << YAML::BeginMap;
			auto& tc = entity.GetComponent<TextureComponent>();

			out << YAML::Key << "Texture" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "Total Types" << YAML::Value << tc.totalTypes;
			out << YAML::Key << "Type" << YAML::Value << tc.type;

			auto& mc = entity.GetComponent<MaterialComponent>();
			//out << YAML::Key << "isPBR" << YAML::Value << mc.isPBR;
			//if (mc.isPBR)
			//{
			//	out << YAML::Key << "Albedo" << YAML::Value << tc.m_Textures[tc.TextureType::Albedo]->GetPath();
			//	out << YAML::Key << "Ambient Occlusion" << YAML::Value << tc.m_Textures[tc.TextureType::AmbientOcclusion]->GetPath();
			//	out << YAML::Key << "Metallic" << YAML::Value << tc.m_Textures[tc.TextureType::Metallic]->GetPath();
			//	out << YAML::Key << "Normal" << YAML::Value << tc.m_Textures[tc.TextureType::Normal]->GetPath();
			//	out << YAML::Key << "Roughness" << YAML::Value << tc.m_Textures[tc.TextureType::Roughness]->GetPath();
			//}
			//else
			//{
			//	out << YAML::Key << "Albedo" << YAML::Value << tc.m_Textures[tc.TextureType::Albedo]->GetPath();
			//}

			out << YAML::EndMap;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<ShaderComponent>())
		{
			out << YAML::Key << "ShaderComponent";
			out << YAML::BeginMap;
			auto& sc = entity.GetComponent<ShaderComponent>();
			auto& shader = sc.m_Shader;

			out << YAML::Key << "Shader" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "Path" << YAML::Value << shader->GetPath();
			out << YAML::EndMap;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<MaterialComponent>())
		{
			out << YAML::Key << "MaterialComponent";
			out << YAML::BeginMap; // MaterialComponent

			auto& tc = entity.GetComponent<MaterialComponent>();
			//out << YAML::Key << "Albedo Color" << YAML::Value << tc.m_Material->GetAlbedoColor();
			//out << YAML::Key << "Roughness" << YAML::Value << tc.m_Material->GetRoughnessScale();
			//out << YAML::Key << "Exposure" << YAML::Value << tc.m_Material->GetExposure();

			//out << YAML::Key << "isPBR" << YAML::Value << tc.isPBR;
			//out << YAML::Key << "Shininess" << YAML::Value << tc.shininess;
			//out << YAML::Key << "Specular" << YAML::Value << tc.specular;

			out << YAML::EndMap; // MaterialComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Position;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<Physics2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap; // Rigidbody2DComponent

			auto& rb2dComponent = entity.GetComponent<Physics2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap; // BoxCollider2DComponent

			auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<PhysicsComponent>())
		{
			out << YAML::Key << "PhysicsComponent";
			out << YAML::BeginMap; // PhysicsComponent

			auto& tc = entity.GetComponent<PhysicsComponent>();

			out << YAML::EndMap; // PhysicsComponent
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{
			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap; // ColliderComponent

			auto& tc = entity.GetComponent<BoxColliderComponent>();

			out << YAML::EndMap; // ColliderComponent
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;
#if 0
			// Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					out << YAML::BeginMap; // ScriptField
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

					out << YAML::Key << "Data" << YAML::Value;
					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float, float);
						WRITE_SCRIPT_FIELD(Double, double);
						WRITE_SCRIPT_FIELD(Bool, bool);
						WRITE_SCRIPT_FIELD(Char, char);
						WRITE_SCRIPT_FIELD(Byte, int8_t);
						WRITE_SCRIPT_FIELD(Short, int16_t);
						WRITE_SCRIPT_FIELD(Int, int32_t);
						WRITE_SCRIPT_FIELD(Long, int64_t);
						WRITE_SCRIPT_FIELD(UByte, uint8_t);
						WRITE_SCRIPT_FIELD(UShort, uint16_t);
						WRITE_SCRIPT_FIELD(UInt, uint32_t);
						WRITE_SCRIPT_FIELD(ULong, uint64_t);
						//WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
						WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
						WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
						WRITE_SCRIPT_FIELD(Entity, uint32_t);
					}
					out << YAML::EndMap; // ScriptFields
				}
				out << YAML::EndSeq;
			}
#endif
			out << YAML::EndMap; // ScriptComponent
		}

		if (false && entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			//out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			//out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			//out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			//out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			//out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			//out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			//out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}
		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity = { entityID, m_Scene.get() };
				if (!entity)
					return;

				SerializeEntity(out, entity);
			});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data = YAML::LoadFile(filepath);
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		Albedo_Core_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				Albedo_Core_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntity(name);

				auto meshComponent = entity["MeshComponent"];
				if (meshComponent)
				{
					//auto& mc = deserializedEntity.GetComponent<MeshComponent>();
					//mc.Color = meshComponent["Color"].as<glm::vec4>();
					auto& mesh = meshComponent["Mesh"];
					std::string& name = mesh["Name"].as<std::string>();
					std::string& path = mesh["Path"].as<std::string>();
					//deserializedEntity.AddComponent<MeshComponent>().AddMesh(m_AssetManager->LoadModelusingAssimp(path), (uint32_t)deserializedEntity);
				}

				auto lightComponent = entity["LightComponent"];
				if (lightComponent)
				{
					auto& lc = deserializedEntity.AddComponent<LightComponent>();

					lc.type = (LightComponent::LightType)lightComponent["Type"].as<int>();
					lc.direction = lightComponent["Direction"].as<glm::vec3>();
					lc.position = lightComponent["Position"].as<glm::vec3>();
					lc.ambient = lightComponent["Ambient"].as<glm::vec3>();
					lc.diffuse = lightComponent["Diffuse"].as<glm::vec3>();
					lc.specular = lightComponent["Specular"].as<glm::vec3>();
					lc.cutOff = lightComponent["Cut Off"].as<float>();
					lc.outerCutOff = lightComponent["Outer Cut Off"].as<float>();
					lc.constant = lightComponent["Constant"].as<float>();
					lc.linear = lightComponent["Linear"].as<float>();
					lc.quadratic = lightComponent["Quadratic"].as<float>();
					lc.nameOfLight = lightComponent["Name Of Light"].as<std::string>();
				}

				auto skyboxComponent = entity["SkyboxComponent"];
				if (skyboxComponent)
				{
					std::vector <std::string> faces;
					faces.push_back(skyboxComponent["Face1"].as<std::string>());
					faces.push_back(skyboxComponent["Face2"].as<std::string>());
					faces.push_back(skyboxComponent["Face3"].as<std::string>());
					faces.push_back(skyboxComponent["Face4"].as<std::string>());
					faces.push_back(skyboxComponent["Face5"].as<std::string>());
					faces.push_back(skyboxComponent["Face6"].as<std::string>());

					deserializedEntity.AddComponent<SkyboxComponent>().m_Skybox = Texture2D::Create(faces);
				}

				auto textureComponent = entity["TextureComponent"];
				if (textureComponent)
				{
					auto& tc = deserializedEntity.AddComponent<TextureComponent>();

					auto& texture = textureComponent["Texture"];
					bool isPBR = texture["isPBR"].as<bool>();
					if (isPBR)
					{
						std::string& albedo = texture["Albedo"].as<std::string>();
						std::string& ambientOcclusion = texture["Ambient Occlusion"].as<std::string>();
						std::string& metallic = texture["Metallic"].as<std::string>();
						std::string& normal = texture["Normal"].as<std::string>();
						std::string& roughness = texture["Roughness"].as<std::string>();

						tc.type = tc.TextureType::Albedo;
						tc.AddTexture(m_AssetManager->LoadTexture(albedo), (int)TextureComponent::TextureType::Albedo);
						tc.type = tc.TextureType::AmbientOcclusion;
						tc.AddTexture(m_AssetManager->LoadTexture(ambientOcclusion), (int)TextureComponent::TextureType::AmbientOcclusion);
						tc.type = tc.TextureType::Metallic;
						tc.AddTexture(m_AssetManager->LoadTexture(metallic), (int)TextureComponent::TextureType::Metallic);
						tc.type = tc.TextureType::Normal;
						tc.AddTexture(m_AssetManager->LoadTexture(normal), (int)TextureComponent::TextureType::Normal);
						tc.type = tc.TextureType::Roughness;
						tc.AddTexture(m_AssetManager->LoadTexture(roughness), (int)TextureComponent::TextureType::Roughness);
					}
					else
					{
						std::string& albedo = texture["Albedo"].as<std::string>();

						tc.type = tc.TextureType::Albedo;
						tc.AddTexture(m_AssetManager->LoadTexture(albedo), (int)TextureComponent::TextureType::Albedo);
					}
				}

				auto shaderComponent = entity["ShaderComponent"];
				if (shaderComponent)
				{
					auto& shader = shaderComponent["Shader"];
					std::string& path = shader["Path"].as<std::string>();
					deserializedEntity.AddComponent<ShaderComponent>().AddShader(m_AssetManager->LoadShader(path));
				}

				auto materialComponent = entity["MaterialComponent"];
				if (materialComponent)
				{
					auto& mc = deserializedEntity.AddComponent<MaterialComponent>();

					//mc.m_Material = std::make_shared<Material>();
					//mc.m_Material->SetAlbedoColor(materialComponent["Albedo Color"].as<glm::vec3>());
					//mc.m_Material->SetExposure(materialComponent["Exposure"].as<float>());
					//mc.m_Material->SetRoughnessScale(materialComponent["Roughness"].as<float>());
					//mc.m_Material->SetPBRStatus(materialComponent["isPBR"].as<bool>());

					//mc.isPBR = materialComponent["isPBR"].as<bool>();
					//mc.specular = materialComponent["Specular"].as<glm::vec3>();
					//mc.shininess = materialComponent["Shininess"].as<float>();
				}

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transforms
					auto& tc = deserializedEntity.AddComponent<TransformComponent>();
					tc.Position = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent)
				{
					auto& rb2d = deserializedEntity.AddComponent<Physics2DComponent>();
					rb2d.Type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
					rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent)
				{
					auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
					bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
					bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
					bc2d.Density = boxCollider2DComponent["Density"].as<float>();
					bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
					bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
					bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto physicsComponent = entity["PhysicsComponent"];
				if (physicsComponent)
				{
					auto& tc = deserializedEntity.AddComponent<PhysicsComponent>();

				}

				auto BoxColliderComponent = entity["BoxColliderComponent"];
				if (BoxColliderComponent)
				{
					//auto& tc = deserializedEntity.AddComponent<BoxColliderComponent>();
					//
					//tc.ColliderPosition = colliderComponent["Collider Position"].as<glm::vec3>();
					//tc.ColliderSize = colliderComponent["Collider Size"].as<glm::vec3>();
					//tc.ColliderOrientation = colliderComponent["Collider Orientation"].as<glm::vec3>();
					//tc.ColliderRadius = colliderComponent["Collider Radius"].as<float>();
					//
					//tc.colliderType = (ColliderComponent::ColliderType)colliderComponent["Collider Type"].as<uint32_t>();
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
					sc.ClassName = scriptComponent["ClassName"].as<std::string>();
#if 0

					auto scriptFields = scriptComponent["ScriptFields"];
					if (scriptFields)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
						if (entityClass)
						{
							const auto& fields = entityClass->GetFields();
							auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();
								std::string typeString = scriptField["Type"].as<std::string>();
								ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

								ScriptFieldInstance& fieldInstance = entityFields[name];

								// TODO: turn this assert into AlbedoEditor log warning
								Albedo_CORE_ASSERT(fields.find(name) != fields.end(), "field not found");

								if (fields.find(name) == fields.end())
									continue;

								fieldInstance.Field = fields.at(name);

								switch (type)
								{
									READ_SCRIPT_FIELD(Float, float);
									READ_SCRIPT_FIELD(Double, double);
									READ_SCRIPT_FIELD(Bool, bool);
									READ_SCRIPT_FIELD(Char, char);
									READ_SCRIPT_FIELD(Byte, int8_t);
									READ_SCRIPT_FIELD(Short, int16_t);
									READ_SCRIPT_FIELD(Int, int32_t);
									READ_SCRIPT_FIELD(Long, int64_t);
									READ_SCRIPT_FIELD(UByte, uint8_t);
									READ_SCRIPT_FIELD(UShort, uint16_t);
									READ_SCRIPT_FIELD(UInt, uint32_t);
									READ_SCRIPT_FIELD(ULong, uint64_t);
									READ_SCRIPT_FIELD(Vector2, glm::vec2);
									READ_SCRIPT_FIELD(Vector3, glm::vec3);
									READ_SCRIPT_FIELD(Vector4, glm::vec4);
									READ_SCRIPT_FIELD(Entity, uint32_t);
								}
							}
						}
#endif
					}

					auto spriteRendererComponent = entity["SpriteRendererComponent"];
					if (spriteRendererComponent)
					{
						auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
						src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
					}

					auto cameraComponent = entity["CameraComponent"];
#if 0
					if (cameraComponent)
					{
						auto& cc = deserializedEntity.AddComponent<CameraComponent>();

						auto& cameraProps = cameraComponent["Camera"];
						cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

						cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
						cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
						cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

						cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
						cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
						cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

						cc.Primary = cameraComponent["Primary"].as<bool>();
						cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
					}
#endif

					auto nativeScriptComponent = entity["NativeScriptComponent"];
					if (nativeScriptComponent)
					{

					}
				}
			}
			return true;
		}

		void SceneSerializer::SerializeRuntime(const std::string & filepath)
		{
			// Not implemented
			//Albedo_CORE_ASSERT(false);
		}

		bool SceneSerializer::DeserializeRuntime(const std::string & filepath)
		{
			// Not implemented
			//Albedo_CORE_ASSERT(false);
			return false;
		}

	}
