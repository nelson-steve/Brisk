#pragma once

// INCLUDES
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

	class Scene
	{
	public:
		void LoadFileSystemGLTFFile(std::filesystem::path path, Entity e);

		Entity CreateMeshEntity(const std::string& name = "Mesh");
		Entity CreateCameraEntity(const std::string& name = "Camera");
		Entity CreateCubeEntity(const std::string& name = "Cube");
		Entity CreatePlaneEntity(const std::string& name = "Plane");
		Entity CreateLightEntity(const std::string& name = "Light");
		Entity CreateEntity(const std::string& name);
		void DestroyEntity(Entity entity);

		void OnViewportResize(uint32_t width, uint32_t height);

		SceneSetting& GetSceneSetting() { return m_SceneSetting; }
		Entity FindEntityByName(std::string_view name);

		Entity DuplicateEntity(Entity entity);

		void SetPaused(bool paused) { m_IsPaused = paused; }

		entt::registry& Reg() { return m_Registry; }
		glm::vec3 lightPos = glm::vec3(-2.0f, 4.0f, -1.0f);

		std::vector<MaterialData> mMaterials;
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		SceneSetting m_SceneSetting;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}