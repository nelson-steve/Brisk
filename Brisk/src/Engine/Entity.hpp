#pragma once

#include "Scene.hpp"

#include "entt.hpp"

namespace Brisk 
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			//Brisk_CORE_ASSERT(!HasComponent<T>(), "Entity already has a component!");
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			//Brisk_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		static T& GetComponent(entt::entity entity)
		{
			//Brisk_CORE_ASSERT(m_Scene->m_Registry.try_get<T>(m_EntityHandle), "Entity does not have component!");
			return m_Scene->m_Registry.get<T>(entity);
		}

		template<typename T>
		bool HasComponent()
		{
			if (m_Scene->m_Registry.try_get<T>(m_EntityHandle))
				return true;
			return false;
		}

		template<typename T>
		static bool HasComponent(entt::entity entity)
		{
			if (m_Scene->m_Registry.try_get<T>(entity))
				return true;
			return false;
		}

		template<typename T>
		void RemoveComponent()
		{
			Albedo_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		template<typename T>
		static void RemoveComponent(entt::entity entity)
		{
			//Brisk_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			m_Scene->m_Registry.remove<T>(entity);
		}

		void AddChild(Entity e) {
			m_Children.push_back(e);
		}

		const std::vector<Entity>& GetChildren() {
			return m_Children;
		}		

		entt::entity GetEntityHandle() const { return m_EntityHandle; }
		const std::string& GetName();

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
	private:
		entt::entity m_EntityHandle{ entt::null };
		std::vector<Entity> m_Children;
	};

}