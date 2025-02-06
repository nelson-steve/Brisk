#pragma once

// INCLUDES
#include "Engine/Renderer/Shader.hpp"
#include "Core/Core.hpp"
#include "Model.hpp"
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

	struct RendererableDataRef {
		uint32_t pVertexCount;
		uint32_t pIndexCount;
		std::vector<MeshData> pMeshDataPtr;
		std::vector<uint32_t> pIndicesDataPtr;
	};

	class Scene
	{
	public:
		Scene(); // this is a constructor as you may remember. since it's name is the same as the class

		void InitDefaults();
		void InitScene();

		void LoadGLTFFile(std::string path, Entity e);

		Entity CreateMeshEntity(const std::string& name = "Mesh");
		Entity CreateCameraEntity(const std::string& name = "Camera");
		Entity CreateCubeEntity(const std::string& name = "Cube");
		Entity CreatePlaneEntity(const std::string& name = "Plane");
		Entity CreateLightEntity(const std::string& name = "Light");
		Entity CreateSkyboxEntity(const std::string& name = "Skybox");
		Entity CreateEntity(const std::string& name);
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void LoadNode(Entity parent, const tinygltf::Node& node, uint32_t node_index, const tinygltf::Model& model, std::shared_ptr<RendererableDataRef> renderableRef);
		void LoadMaterials(tinygltf::Model model, std::shared_ptr<RendererableDataRef> ref);

		//void OnUpdateRuntime(Timestep ts);
		//void OnUpdateSimulation(Ref<EditorCamera> camera, Timestep ts);
		//void OnUpdateEditor(Ref<EditorCamera> camera, Timestep ts);
		//void RenderScene(Camera* camera, Timestep ts);
		//void OnUpdateResize(uint32_t width, uint32_t height);

		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();
		SceneSetting& GetSceneSetting() { return m_SceneSetting; }
		Entity FindEntityByName(std::string_view name);

		Entity DuplicateEntity(Entity entity);

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void SetPaused(bool paused) { m_IsPaused = paused; }

		//void SetMainFramebuffer(Ref<Framebuffer> fbo) { m_Framebuffer = fbo; }

		void Step(int frames = 1);

		void EnableGravity(bool gravity);

		entt::registry& Reg() { return m_Registry; }
		glm::vec3 lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;
		bool m_IsSimulating = false;
		SceneSetting m_SceneSetting;

		uint32_t m_vertex_pos = 0;
		uint32_t m_index_pos = 0;

		std::vector<std::shared_ptr<Texture>> mTextures;
		std::vector<GLTF_Node*> pNodes;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}