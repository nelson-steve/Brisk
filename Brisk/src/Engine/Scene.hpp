#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "entt.hpp"

#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace Brisk
{
	class Entity;

	struct SceneSetting
	{
		bool ShowCollider = true;
		bool enableGravity = true;
		float backgroundLight = 1.0;
	};

	class Scene
	{
	public:
		Scene(); // this is a constructor as you may remember. since it's name is the same as the class

		void InitDefaults();
		void InitScene();

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

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateSimulation(Ref<EditorCamera> camera, Timestep ts);
		void OnUpdateEditor(Ref<EditorCamera> camera, Timestep ts);
		void RenderScene(Camera* camera, Timestep ts);
		void OnUpdateResize(uint32_t width, uint32_t height);

		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();
		SceneSetting& GetSceneSetting() { return m_SceneSetting; }
		Entity FindEntityByName(std::string_view name);

		Entity DuplicateEntity(Entity entity);

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }

		void SetPaused(bool paused) { m_IsPaused = paused; }

		void SetMainFramebuffer(Ref<Framebuffer> fbo) { m_Framebuffer = fbo; }

		void Step(int frames = 1);

		void EnableGravity(bool gravity);

		entt::registry& Reg() { return m_Registry; }
		glm::vec3 lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);

		//TODO: make this private
		Ref<Texture2D> m_DepthMap;
		Ref<Framebuffer> m_DepthMapFBO;
		Ref<ShadowMap> m_ShadowMap;
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		// Colliders stuff
		Ref<Model> m_Cube;
		Ref<Model> m_Sphere;
		Ref<Model> m_Capsule;
		Ref<Shader> m_ColliderShader;
		//

		entt::registry m_Registry;
		Ref<Framebuffer> m_Framebuffer;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;
		bool m_IsSimulating = false;
		SceneSetting m_SceneSetting;

		b2World* m_PhysicsWorld = nullptr;

		Ref<PhysicsWorld> m_PhysicsWorld3D;
		Ref<Shader> m_SkyboxShader;

		Ref<Shader> m_CubeShader;

		glm::mat4 transform{ 1.0f };

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}