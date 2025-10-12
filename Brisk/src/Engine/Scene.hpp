#pragma once

// INCLUDES
#include "Core/Core.hpp"
#include "Renderer/Buffer.hpp"
//---------------------
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "entt.hpp"
//----------------
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <filesystem>
#include <iostream>
//-------------------

namespace Brisk
{
	class Entity;

	struct SceneSetting
	{
		bool ShowCollider = true;
		bool enableGravity = true;
		float backgroundLight = 1.0;
	};

    struct Vertex {
        float vx, vy, vz; // 4 + 4 + 4 = 12
        float nx, ny, nz; // 4 + 4 + 4 = 12
        float tx, ty;     // 4 + 4 = 8
    };

    struct alignas(16) Mesh
    {
        glm::vec3 center;
        float radius;

        uint32_t vertexOffset;
        uint32_t vertexCount;
        uint32_t meshletCount;
        uint32_t meshletOffset;
    };

    struct alignas(16) Meshlet
    {
        glm::vec3 center;
        float radius;
        int8_t cone_axis[3];
        int8_t cone_cutoff;

        uint32_t dataOffset;
        uint32_t baseVertex;
        uint8_t vertexCount;
        uint8_t triangleCount;
        uint8_t shortRefs;
        uint8_t padding;
    };

    struct alignas(16) MeshDraw
    {
        glm::vec3 position;
        float scale;
        glm::quat orientation;

        uint32_t meshIndex;
        uint32_t materialIndex;
        uint32_t meshletCount;
        uint32_t meshletOffset;

        //VkDrawMeshTasksIndirectCommandEXT
        uint32_t groupCountX;
        uint32_t groupCountY;
        uint32_t groupCountZ;
    };

    struct Geometry
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Meshlet> meshlets;
        std::vector<uint32_t> meshletdata;
        std::vector<Mesh> meshes;
        std::vector<MeshDraw> draws;
    };

    struct alignas(16) MaterialData {
        uint32_t alphaMode; // use 4 bytes to avoid packing issues
        float alphaCutoff;
        float metallicFactor;
        float roughnessFactor;

        alignas(16) glm::vec4 baseColorFactor;
        alignas(16) glm::vec3 emissiveFactor;

        float emissiveStrength;

        uint32_t baseColorTextureIndex;
        uint32_t metallicRoughnessTextureIndex;
        uint32_t normalTextureIndex;
        uint32_t occlusionTextureIndex;
        uint32_t emissiveTextureIndex;
    };

	class Scene {
	public:
		Entity CreateCameraEntity(const std::string& name = "Camera");
		Entity CreateCubeEntity(const std::string& name = "Cube");
		Entity CreatePlaneEntity(const std::string& name = "Plane");
		Entity CreateLightEntity(const std::string& name = "Light");
		Entity CreateEntity(const std::string& name);
		void DestroyEntity(Entity entity);

		void SetSelectedEntity(entt::entity e) { m_SelectedEntity = e; }
		entt::entity GetSelectedEntity() { return m_SelectedEntity; }

		void OnViewportResize(uint32_t width, uint32_t height);

		SceneSetting& GetSceneSetting() { return m_SceneSetting; }
		Entity FindEntityByName(std::string_view name);

		Entity DuplicateEntity(Entity entity);

		void SetPaused(bool paused) { m_IsPaused = paused; }

		entt::registry& Reg() { return m_Registry; }
		glm::vec3 lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);

		void LoadGltfScene(const std::filesystem::path& gltfPath);
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
    public:
        static Geometry m_Geometry;
        uint32_t m_MeshletCount;
        std::vector<std::shared_ptr<Texture>> m_Textures;
        std::vector<MaterialData> m_Materials;
        static std::shared_ptr<Buffer> m_VertexBuffer;
        static std::shared_ptr<Buffer> m_IndexBuffer;
        static std::shared_ptr<Buffer> m_DrawsBuffer;
        static std::shared_ptr<Buffer> m_MeshletsBuffer;
        static std::shared_ptr<Buffer> m_MeshletDataBuffer;
        static std::shared_ptr<Buffer> m_MaterialStorageBuffer;
	private:
		entt::registry m_Registry;
		entt::entity m_SelectedEntity;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		SceneSetting m_SceneSetting;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}