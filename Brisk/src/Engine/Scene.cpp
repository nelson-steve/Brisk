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
//-------------------------------------

namespace Brisk 
{

	//extern std::unique_ptr<AssetSystem> m_AssetManager;

	Scene::Scene()
	{
	}

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

	void Scene::InitDefaults()
	{
	}

	void Scene::InitScene()
	{
		//m_PhysicsWorld3D = std::make_shared<PhysicsWorld>();
		//m_PhysicsWorld3D->Init();
	}

	void Scene::LoadNode(Entity parent, const tinygltf::Node& node, uint32_t node_index, const tinygltf::Model& model, std::shared_ptr<RendererableDataRef> renderableRef) {
		//GLTF_Node* new_node = new GLTF_Node();
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.GetComponent<TransformComponent>().parent = parent;
		entity.AddComponent<TagComponent>().Tag = node.name;
		//new_node->index = node_index;
		//new_node->matrix = glm::mat4(1.0f);

		if (node.translation.size() == 3) {
			entity.GetComponent<TransformComponent>().Position = glm::make_vec3(node.translation.data());
		}
		if (node.rotation.size() == 4) {
			//new_node->rotation = glm::make_quat(node.rotation.data());
			entity.GetComponent<TransformComponent>().Rotation = glm::make_vec3(node.rotation.data());
		}
		if (node.scale.size() == 4) {
			entity.GetComponent<TransformComponent>().Scale = glm::make_vec3(node.scale.data());
		}
		if (node.matrix.size() == 16) {
			entity.GetComponent<TransformComponent>().Matrix = glm::make_mat4x4(node.matrix.data());
		}

		if (node.children.size() > 0) {
			for (auto& node_index : node.children)
				LoadNode(entity, model.nodes[node_index], node_index, model, renderableRef);
		}

		if (node.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			entity.AddComponent<MeshComponent>();
			for (auto& primitive : mesh.primitives) {
				uint32_t vertex_start = m_vertex_pos;
				uint32_t index_start = m_index_pos;
				uint32_t vertex_count = 0;
				uint32_t index_count = 0;
				// Vertices
				{
					const float* buffer_pos = nullptr;
					const float* buffer_normals = nullptr;
					const float* buffer_uv_set0 = nullptr;
					const float* buffer_uv_set1 = nullptr;
					const float* buffer_color_set0 = nullptr;
					const void* buffer_joints = nullptr;
					const float* buffer_weights = nullptr;

					int posByteStride = 0;
					int normByteStride = 0;
					int uv0ByteStride = 0;
					int uv1ByteStride = 0;
					int color0ByteStride = 0;
					//int jointByteStride;
					//int weightByteStride;

					if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
						const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
						const tinygltf::BufferView& pos_view = model.bufferViews[pos_accessor.bufferView];
						vertex_count = static_cast<uint32_t>(pos_accessor.count);
						buffer_pos = reinterpret_cast<const float*>(&(model.buffers[pos_view.buffer].data[pos_accessor.byteOffset + pos_view.byteOffset]));
						posByteStride = pos_accessor.ByteStride(pos_view) ? (pos_accessor.ByteStride(pos_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}
					else {
						assert(primitive.attributes.find("POSITION") != primitive.attributes.end());
					}

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
						const tinygltf::Accessor& normal_accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& normal_view = model.bufferViews[normal_accessor.bufferView];
						buffer_normals = reinterpret_cast<const float*>(&(model.buffers[normal_view.buffer].data[normal_accessor.byteOffset + normal_view.byteOffset]));
						normByteStride = normal_accessor.ByteStride(normal_view) ? (normal_accessor.ByteStride(normal_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& uv0_accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& uv0_view = model.bufferViews[uv0_accessor.bufferView];
						buffer_uv_set0 = reinterpret_cast<const float*>(&(model.buffers[uv0_view.buffer].data[uv0_accessor.byteOffset + uv0_view.byteOffset]));
						uv0ByteStride = uv0_accessor.ByteStride(uv0_view) ? (uv0_accessor.ByteStride(uv0_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
						const tinygltf::Accessor& uv1_accessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
						const tinygltf::BufferView& uv1_view = model.bufferViews[uv1_accessor.bufferView];
						buffer_uv_set1 = reinterpret_cast<const float*>(&(model.buffers[uv1_view.buffer].data[uv1_accessor.byteOffset + uv1_view.byteOffset]));
						uv1ByteStride = uv1_accessor.ByteStride(uv1_view) ? (uv1_accessor.ByteStride(uv1_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& color0_accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
						const tinygltf::BufferView& uv1_view = model.bufferViews[color0_accessor.bufferView];
						buffer_color_set0 = reinterpret_cast<const float*>(&(model.buffers[uv1_view.buffer].data[color0_accessor.byteOffset + uv1_view.byteOffset]));
						color0ByteStride = color0_accessor.ByteStride(uv1_view) ? (color0_accessor.ByteStride(uv1_view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					const tinygltf::Accessor& pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
					for (size_t v = 0; v < pos_accessor.count; v++) {
						MeshData& vert = renderableRef->pMeshDataPtr[m_vertex_pos];
						vert.Position = glm::vec4(glm::make_vec3(&buffer_pos[v * posByteStride]), 1.0f);
						vert.Normal = glm::normalize(glm::vec3(buffer_normals ? glm::make_vec3(&buffer_normals[v * normByteStride]) : glm::vec3(0.0f)));
						vert.UV0 = buffer_uv_set0 ? glm::make_vec2(&buffer_uv_set0[v * uv0ByteStride]) : glm::vec2(0.0f);
						//vert.UV1 = buffer_uv_set1 ? glm::make_vec2(&buffer_uv_set1[v * uv1ByteStride]) : glm::vec2(0.0f);
						//vert.Color = buffer_color_set0 ? glm::make_vec3(&buffer_color_set0[v * color0ByteStride]) : glm::vec3(1.0f);

						m_vertex_pos++;
					}

				}
				bool has_indices = primitive.indices > -1;
				if (has_indices) {
					const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
					const tinygltf::BufferView& buffer_view = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[buffer_view.buffer];

					index_count = static_cast<uint32_t>(accessor.count);
					const void* data_ptr = &(buffer.data[accessor.byteOffset + buffer_view.byteOffset]);

					switch (accessor.componentType) {
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
						const uint32_t* buf = static_cast<const uint32_t*>(data_ptr);
						for (size_t index = 0; index < accessor.count; index++) {
							renderableRef->pIndicesDataPtr[m_index_pos] = buf[index] + vertex_start;
							m_index_pos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
						const uint16_t* buf = static_cast<const uint16_t*>(data_ptr);
						for (size_t index = 0; index < accessor.count; index++) {
							renderableRef->pIndicesDataPtr[m_index_pos] = buf[index] + vertex_start;
							m_index_pos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
						const uint8_t* buf = static_cast<const uint8_t*>(data_ptr);
						for (size_t index = 0; index < accessor.count; index++) {
							renderableRef->pIndicesDataPtr[m_index_pos] = buf[index] + vertex_start;
							m_index_pos++;
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}
				else {
					assert(false);
				}
				uint32_t mat_index = primitive.material > -1 ? primitive.material : -1;
				SubMesh subMesh{};
				subMesh.first_index = index_start;
				subMesh.index_count = index_count;
				subMesh.vertex_count = vertex_count;
				subMesh.vertex_count = vertex_count;
				subMesh.material_index = mat_index;
				entity.GetComponent<MeshComponent>().subMeshes.push_back(subMesh);
			}
		}
		if (parent) {
			parent.GetComponent<TransformComponent>().children.push_back(entity);
		}
	}

	// TODO: Move GLTF specific functions to a namespace or static class GLTF_Utility
	void GetNodeProps(const tinygltf::Node& rootNode, const tinygltf::Model& model, std::shared_ptr<RendererableDataRef> ref) {
		if (rootNode.children.size() > 0) {
			for (size_t i = 0; i < rootNode.children.size(); i++) {
				GetNodeProps(model.nodes[rootNode.children[i]], model, ref);
			}
		}
		if (rootNode.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[rootNode.mesh];
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				auto primitive = mesh.primitives[i];
				ref->pVertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;
				if (primitive.indices > -1) {
					ref->pIndexCount += model.accessors[primitive.indices].count;
				}
			}
		}
	}

	void Scene::LoadMaterials(tinygltf::Model model) {
		for (tinygltf::Material& mat : model.materials) {
			MaterialData material{};
			material.doubleSided = mat.doubleSided;
			if (mat.values.find("baseColorTexture") != mat.values.end()) {
				material.baseColorTextureIndex = mat.values["baseColorTexture"].TextureIndex();
				material.texCoordSets.baseColor = mat.values["baseColorTexture"].TextureTexCoord();
			}
			if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
				material.metallicRoughnessTextureIndex = mat.values["metallicRoughnessTexture"].TextureIndex();
				material.texCoordSets.metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
			}
			if (mat.values.find("roughnessFactor") != mat.values.end()) {
				material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
			}
			if (mat.values.find("metallicFactor") != mat.values.end()) {
				material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
			}
			if (mat.values.find("baseColorFactor") != mat.values.end()) {
				material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
			}
			if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
				material.normalTextureIndex = mat.additionalValues["normalTexture"].TextureIndex();
				material.texCoordSets.normal = mat.additionalValues["normalTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
				material.emissiveTextureIndex = mat.additionalValues["emissiveTexture"].TextureIndex();
				material.texCoordSets.emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
				material.occlusionTextureIndex = mat.additionalValues["occlusionTexture"].TextureIndex();
				material.texCoordSets.occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
			}
			if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
				tinygltf::Parameter param = mat.additionalValues["alphaMode"];
				if (param.string_value == "BLEND") {
					material.alphaMode = MaterialData::ALPHAMODE_BLEND;
				}
				if (param.string_value == "MASK") {
					material.alphaCutoff = 0.5f;
					material.alphaMode = MaterialData::ALPHAMODE_MASK;
				}
			}
			if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
				material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCutoff"].Factor());
			}
			if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
				material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
			}

			// Extensions
			// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
			if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				if (ext->second.Has("specularGlossinessTexture")) {
					auto index = ext->second.Get("specularGlossinessTexture").Get("index");
					material.extension.specularGlossinessTexture = mTextures[index.Get<int>()];
					auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
					material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
					material.pbrWorkflows.specularGlossiness = true;
				}
				if (ext->second.Has("diffuseTexture")) {
					auto index = ext->second.Get("diffuseTexture").Get("index");
					material.extension.diffuseTexture = mTextures[index.Get<int>()];
				}
				if (ext->second.Has("diffuseFactor")) {
					auto factor = ext->second.Get("diffuseFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
						auto val = factor.Get(i);
						material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
				if (ext->second.Has("specularFactor")) {
					auto factor = ext->second.Get("specularFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++) {
						auto val = factor.Get(i);
						material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
			}

			if (mat.extensions.find("KHR_materials_unlit") != mat.extensions.end()) {
				material.unlit = true;
			}

			if (mat.extensions.find("KHR_materials_emissive_strength") != mat.extensions.end()) {
				auto ext = mat.extensions.find("KHR_materials_emissive_strength");
				if (ext->second.Has("emissiveStrength")) {
					auto value = ext->second.Get("emissiveStrength");
					material.emissiveStrength = (float)value.Get<double>();
				}
			}

			material.index = static_cast<uint32_t>(mMaterials.size());
			mMaterials.push_back(material);
		}
		// Push a default material at the end of the list for meshes with no material assigned
		mMaterials.push_back(MaterialData());
	}


	void CalculateGLTFMeshSize(const tinygltf::Scene& scene, const tinygltf::Model& model, std::shared_ptr<RendererableDataRef> ref) {
		for (auto& node_index : scene.nodes) {
			GetNodeProps(model.nodes[node_index], model, ref);
		}
	}

	void Scene::LoadGLTFFile(std::string path, Entity entity) {
		tinygltf::TinyGLTF loader;
		tinygltf::Model model;
		std::string error;
		std::string warning;

		bool binary = false;
		size_t extpos = path.rfind('.', path.length());
		if (extpos != std::string::npos) {
			binary = (path.substr(extpos + 1, path.length() - extpos) == "glb");
		}

		bool file_loaded = false;
		if (binary) {
			file_loaded = loader.LoadBinaryFromFile(&model, &error, &warning, path.c_str());
		}
		else {
			file_loaded = loader.LoadASCIIFromFile(&model, &error, &warning, path.c_str());
		}

		if (!file_loaded) {
			BRISK_CORE_ERROR("gltf model file could not be loaded");
			return;
		}

		int textureIndex = 0;
		for (tinygltf::Texture& tex : model.textures) {
			tinygltf::Image image = model.images[tex.source];

			Texture::TextureSampler texture_sampler{};
			// No sampler specified, use a default one
			texture_sampler.min_filter = Texture::FILTER_LINEAR;
			texture_sampler.mag_filter = Texture::FILTER_LINEAR;
			texture_sampler.address_modeU = Texture::SAMPLER_ADDRESS_MODE_REPEAT;
			texture_sampler.address_modeV = Texture::SAMPLER_ADDRESS_MODE_REPEAT;
			texture_sampler.address_modeW = Texture::SAMPLER_ADDRESS_MODE_REPEAT;

			std::shared_ptr<Texture> texture;
			texture = Texture::Create();
			texture->Init(image, texture_sampler);
			mTextures.push_back(texture);

			Engine::s_Application->GetGpuAdapter()->AddResource(Brisk::BindlessTextures, texture, nullptr, textureIndex);
			textureIndex++;
		}

		LoadMaterials(model);

		//Entity entity = { m_Registry.create(), this };
		entity.AddComponent<MeshComponent>();
		entity.GetComponent<MeshComponent>().renderableRef = std::make_shared<RendererableDataRef>();

		const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
		CalculateGLTFMeshSize(scene, model, entity.GetComponent<MeshComponent>().renderableRef);

		assert(entity.GetComponent<MeshComponent>().renderableRef->pVertexCount > 0);
		entity.GetComponent<MeshComponent>().renderableRef->pMeshDataPtr.resize(entity.GetComponent<MeshComponent>().renderableRef->pVertexCount);
		entity.GetComponent<MeshComponent>().renderableRef->pIndicesDataPtr.resize(entity.GetComponent<MeshComponent>().renderableRef->pIndexCount);

		entity.AddComponent<RootComponent>();
		entity.AddComponent<TransformComponent>();

		for (auto& node_index : scene.nodes) {
			const tinygltf::Node node = model.nodes[node_index];
			LoadNode(entity, node, node_index, model, entity.GetComponent<MeshComponent>().renderableRef);
		}

		entity.GetComponent<RootComponent>().m_VertexBuffer = Buffer::Create();
		entity.GetComponent<RootComponent>().m_VertexBuffer->Init(sizeof(entity.GetComponent<MeshComponent>().renderableRef->pMeshDataPtr[0]) * entity.GetComponent<MeshComponent>().renderableRef->pVertexCount,
			entity.GetComponent<MeshComponent>().renderableRef->pMeshDataPtr.data(),
			Core::BufferUsage::VertexBuffer | Core::BufferUsage::TransferDst,
			Core::MemoryProperty::DeviceLocal,
			false);

		if (entity.GetComponent<MeshComponent>().renderableRef->pIndexCount > 0) {
			entity.GetComponent<RootComponent>().m_IndexBuffer = Buffer::Create();
			entity.GetComponent<RootComponent>().m_IndexBuffer->Init(sizeof(entity.GetComponent<MeshComponent>().renderableRef->pIndicesDataPtr[0]) * entity.GetComponent<MeshComponent>().renderableRef->pIndexCount,
				entity.GetComponent<MeshComponent>().renderableRef->pIndicesDataPtr.data(),
				Core::BufferUsage::IndexBuffer | Core::BufferUsage::TransferDst,
				Core::MemoryProperty::DeviceLocal,
				true);
		}

		entity.GetComponent<MeshComponent>().renderableRef->pMeshDataPtr.clear();
		entity.GetComponent<MeshComponent>().renderableRef->pIndicesDataPtr.clear();
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
		LoadGLTFFile(paths[2], entity);

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateCubeEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<ScriptComponent>();
		entity.AddComponent<PhysicsComponent>();
		entity.AddComponent<BoxColliderComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreatePlaneEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<ScriptComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateLightEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		entity.AddComponent<LightComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		return entity;
	}

	Entity Scene::CreateCameraEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
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
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
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
