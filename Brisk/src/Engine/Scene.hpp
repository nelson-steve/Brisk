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

	struct SceneSetting {
		bool ShowCollider = true;
		bool enableGravity = true;
		float backgroundLight = 1.0;
	};

    struct Vertex {
        float vx, vy, vz;         // 4 + 4 + 4 = 12
        float nx, ny, nz;         // 4 + 4 + 4 = 12
        float tx, ty;             // 4 + 4 = 8
        float tpx, tpy, tpz, tpw; // 4 + 4 + 4 + 4 = 16
    };

    struct alignas(16) Mesh {
        glm::vec3 center;
        float radius;

        uint32_t vertexOffset;
        uint32_t vertexCount;
        uint32_t meshletCount;
        uint32_t materialIndex;

        uint32_t meshletOffset;
        uint32_t indexCount;
        uint32_t indexOffset;
        uint32_t _pad;
    };

    struct alignas(16) Meshlet {
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

    struct alignas(16) MeshTransform {
        glm::vec3 position;
        float scale;
        glm::vec4 orientation;
    };

    struct alignas(16) MeshDraw {
        uint32_t transformIndex;
        uint32_t meshIndex;
        uint32_t materialIndex;
        uint32_t meshletCount;
        uint32_t meshletOffset;

        //VkDrawMeshTasksIndirectCommandEXT
        uint32_t groupCountX;
        uint32_t groupCountY;
        uint32_t groupCountZ;
    };

    struct alignas(16) MaterialData {
        uint32_t alphaMode;
        float alphaCutoff;
        float metallicFactor;
        float roughnessFactor;

        alignas(16) glm::vec4 baseColorFactor;
        alignas(16) glm::vec3 emissiveFactor;

        float emissiveStrength;

        int32_t baseColorTextureIndex = -1;
        int32_t metallicRoughnessTextureIndex = -1;
        int32_t normalTextureIndex = -1;
        int32_t occlusionTextureIndex = -1;
        int32_t emissiveTextureIndex = -1;
    };

    struct Geometry {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Meshlet> meshlets;
        std::vector<uint32_t> meshletdata;
        std::vector<Mesh> meshes;
        std::vector<MeshTransform> transforms;
        std::vector<MeshDraw> draws;
        std::vector<MaterialData> materials;
    };

	class Scene {
	public:
		Entity CreateCameraEntity(const std::string& name = "Camera");
		Entity CreateCubeEntity(const std::string& name = "Cube");
		Entity CreatePlaneEntity(const std::string& name = "Plane");
		Entity CreateLightEntity(const std::string& name = "Light");
		Entity CreateEntity(const std::string& name);
		void DestroyEntity(Entity entity);
		Entity DuplicateEntity(Entity entity);
		void SetSelectedEntity(entt::entity e) { m_SelectedEntity = e; }
		entt::entity GetSelectedEntity() { return m_SelectedEntity; }
		Entity FindEntityByName(std::string_view name);

		void OnViewportResize(uint32_t width, uint32_t height);
		SceneSetting& GetSceneSetting() { return m_SceneSetting; }
		void LoadGltfScene(const std::filesystem::path& gltfPath);
		void SetPaused(bool paused) { m_IsPaused = paused; }
        void UpdateTransforms();

        uint32_t GetDrawsCount() { return m_Geometry.draws.size(); }
        const std::vector<Mesh>& GetMeshes() const { return m_Geometry.meshes; }
        const std::vector<MeshDraw>& GetDraws() const { return m_Geometry.draws; }
        const std::vector<MeshTransform>& GetTransforms() const { return m_Geometry.transforms; }

		entt::registry& Reg() { return m_Registry; }
		glm::vec3 lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
    public:
        std::vector<std::shared_ptr<Texture>> m_Textures;
	private:
        uint32_t m_MeshletCount;
        Geometry m_Geometry;
		entt::registry m_Registry;
		entt::entity m_SelectedEntity;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		SceneSetting m_SceneSetting;
        bool m_SceneUpdated = false;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}