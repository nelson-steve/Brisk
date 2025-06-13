// INCLUDES
#include "Scene.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "Engine/Engine.hpp"
#include "Core/Log.hpp"
//--------------------------
#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stack>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
//-------------------------------------

namespace Brisk 
{
	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	void Scene::LoadFileSystemGLTFFile(std::filesystem::path path, Entity e) {
		if (!std::filesystem::exists(path)) {
			std::cout << "Failed to find " << path << '\n';
		}

		if constexpr (std::is_same_v<std::filesystem::path::value_type, wchar_t>) {
			std::wcout << "Loading " << path << '\n';
		}
		else {
			std::cout << "Loading " << path << '\n';
		}

		fastgltf::Asset asset;

		// Parse the glTF file and get the constructed asset
		{
			static constexpr auto supportedExtensions =
				fastgltf::Extensions::KHR_mesh_quantization |
				fastgltf::Extensions::KHR_texture_transform |
				fastgltf::Extensions::KHR_materials_variants;

			fastgltf::Parser parser(supportedExtensions);

			constexpr auto gltfOptions =
				fastgltf::Options::DontRequireValidAssetMember |
				//fastgltf::Options::AllowDouble |
				fastgltf::Options::LoadExternalBuffers |
				fastgltf::Options::LoadExternalImages |
				fastgltf::Options::GenerateMeshIndices;

			auto gltfFile = fastgltf::MappedGltfFile::FromPath(path);
			if (!bool(gltfFile)) {
				std::cerr << "Failed to open glTF file: " << fastgltf::getErrorMessage(gltfFile.error()) << '\n';
			}

			auto a = parser.loadGltf(gltfFile.get(), path.parent_path(), gltfOptions);
			if (a.error() != fastgltf::Error::None) {
				std::cerr << "Failed to load glTF: " << fastgltf::getErrorMessage(a.error()) << '\n';
			}

			asset = std::move(a.get());
		}

		if (asset.buffers.empty()) {
			throw std::runtime_error("GLTF has no buffers");
		}

		for (const auto& mesh : asset.meshes) {
			for (auto it = mesh.primitives.begin(); it != mesh.primitives.end(); ++it) {
				auto* positionIt = it->findAttribute("POSITION");
				auto* normalIt = it->findAttribute("NORMAL");
				auto* texCoord0It = it->findAttribute("TEXCOORD_0");
				auto* texCoord1It = it->findAttribute("TEXCOORD_1");

				assert(positionIt != it->attributes.end());
				assert(it->indicesAccessor.has_value());

				auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
				if (!positionAccessor.bufferViewIndex.has_value())
					continue;

				// Load positions
				std::vector<fastgltf::math::fvec3> positions;
				if (positionAccessor.componentType == fastgltf::ComponentType::Float &&
					positionAccessor.type == fastgltf::AccessorType::Vec3) {
					positions.resize(positionAccessor.count);
					fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, positionAccessor, positions.data());
				}

				// Load normals
				std::vector<fastgltf::math::fvec3> normals;
				bool hasNormals = (normalIt != it->attributes.end());
				if (hasNormals) {
					auto& normalAccessor = asset.accessors[normalIt->accessorIndex];
					if (normalAccessor.componentType == fastgltf::ComponentType::Float &&
						normalAccessor.type == fastgltf::AccessorType::Vec3 &&
						normalAccessor.bufferViewIndex.has_value()) {
						normals.resize(normalAccessor.count);
						fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, normalAccessor, normals.data());
					}
					else {
						hasNormals = false;
					}
				}

				// Load texcoords
				std::vector<fastgltf::math::fvec2> texcoords0;
				bool hasTexcoords0 = (texCoord0It != it->attributes.end());
				if (hasTexcoords0) {
					auto& texcoordAccessor = asset.accessors[texCoord0It->accessorIndex];
					if (texcoordAccessor.componentType == fastgltf::ComponentType::Float &&
						texcoordAccessor.type == fastgltf::AccessorType::Vec2 &&
						texcoordAccessor.bufferViewIndex.has_value()) {
						texcoords0.resize(texcoordAccessor.count);
						fastgltf::copyFromAccessor<fastgltf::math::fvec2>(asset, texcoordAccessor, texcoords0.data());
					}
					else {
						hasTexcoords0 = false;
					}
				}

				// Load texcoords
				std::vector<fastgltf::math::fvec2> texcoords1;
				bool hasTexcoords1 = (texCoord1It != it->attributes.end());
				if (hasTexcoords1) {
					auto& texcoordAccessor = asset.accessors[texCoord1It->accessorIndex];
					if (texcoordAccessor.componentType == fastgltf::ComponentType::Float &&
						texcoordAccessor.type == fastgltf::AccessorType::Vec2 &&
						texcoordAccessor.bufferViewIndex.has_value()) {
						texcoords1.resize(texcoordAccessor.count);
						fastgltf::copyFromAccessor<fastgltf::math::fvec2>(asset, texcoordAccessor, texcoords1.data());
					}
					else {
						hasTexcoords1 = false;
					}
				}

				// Combine into vertex struct
				for (size_t i = 0; i < positions.size(); ++i) {
					MeshData data{};
					data.Position = glm::vec3(positions[i].x(), positions[i].y(), positions[i].z());

					if (hasNormals && i < normals.size())
						data.Normal = glm::vec3(normals[i].x(), normals[i].y(), normals[i].z());
					else
						data.Normal = glm::vec3(0.0f);

					if (hasTexcoords0 && i < texcoords0.size())
						data.UV0 = glm::vec2(texcoords0[i].x(), texcoords0[i].y());
					else
						data.UV0 = glm::vec2(0.0f);

					if (hasTexcoords1 && i < texcoords1.size())
						data.UV1 = glm::vec2(texcoords1[i].x(), texcoords1[i].y());
					else
						data.UV1 = glm::vec2(0.0f);

					//render.pMeshDataPtr.push_back(data);
				}


				size_t primitiveIndex = std::distance(mesh.primitives.begin(), it);
				const auto& primitive = *it;

				//// Load indices
				//if (it->indicesAccessor.has_value()) {
				//	const auto& indexAccessor = asset.accessors[it->indicesAccessor.value()];

				//	switch (indexAccessor.componentType) {
				//	case fastgltf::ComponentType::UnsignedByte: {
				//		std::vector<uint8_t> indices(indexAccessor.count);
				//		fastgltf::copyFromAccessor<uint8_t>(asset, indexAccessor, indices.data());
				//		for (uint8_t i : indices)
				//			render.pIndicesDataPtr.push_back(static_cast<uint32_t>(i));
				//		break;
				//	}
				//	case fastgltf::ComponentType::UnsignedShort: {
				//		std::vector<uint16_t> indices(indexAccessor.count);
				//		fastgltf::copyFromAccessor<uint16_t>(asset, indexAccessor, indices.data());
				//		for (uint16_t i : indices)
				//			render.pIndicesDataPtr.push_back(static_cast<uint32_t>(i));
				//		break;
				//	}
				//	case fastgltf::ComponentType::UnsignedInt: {
				//		std::vector<uint32_t> indices(indexAccessor.count);
				//		fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, indices.data());
				//		render.pIndicesDataPtr.insert(render.pIndicesDataPtr.end(), indices.begin(), indices.end());
				//		break;
				//	}
				//	default:
				//		throw std::runtime_error("Unsupported index component type.");
				//	}
				//}
			}
		}
	}

	Entity Scene::CreateMeshEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		std::vector<std::string> paths = {
			/* 0 */"../Data/Models/Cube/Cube.gltf",
			/* 1 */"../Data/Models/revolver/revolver.gltf",
			/* 2 */"../Data/Models/gltf_models/Sponza/glTF/Sponza.gltf",
			/* 3 */"../Data/Models/damaged_helmet/DamagedHelmet.gltf",
			/* 4 */"../Data/Models/revolver/revolver.gltf",
			/* 5 */"../Data/Models/cerberus/cerberus.gltf",
			/* 6 */"../Data/Models/gltf_models/BoomBox/glTF/BoomBox.gltf",
			/* 7 */"../Data/Models/spaceship/scene.gltf",
		};
		auto start = std::chrono::high_resolution_clock::now();
		LoadFileSystemGLTFFile(paths[2], entity);
		auto end = std::chrono::high_resolution_clock::now();

		// Calculate duration
		std::chrono::duration<double, std::milli> duration = end - start;
		std::cout << "Time taken: " << duration.count() << " ms\n";

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateCubeEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<LocalTransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreatePlaneEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<LocalTransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateLightEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<LocalTransformComponent>();
		entity.AddComponent<LightComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<LocalTransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateCameraEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<LocalTransformComponent>();
		entity.AddComponent<CameraComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			//if (!cameraComponent.FixedAspectRatio)
				//cameraComponent.Camera->SetViewportSize(width, height);
		}
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<LocalTransformComponent>(Entity entity, LocalTransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<WorldTransformComponent>(Entity entity, WorldTransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<LightComponent>(Entity entity, LightComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkyboxComponent>(Entity entity, SkyboxComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CapsuleColliderComponent>(Entity entity, CapsuleColliderComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MaterialComponent>(Entity entity, MaterialComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<PhysicsComponent>(Entity entity, PhysicsComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<Physics2DComponent>(Entity entity, Physics2DComponent& component)
	{
	}

}
