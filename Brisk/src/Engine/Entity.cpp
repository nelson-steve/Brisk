// INCLUDES
#include "pch.hpp"
#include "Entity.hpp"
#include "Component.hpp"
//---------------------

#define STATIC(a) decltype(a) a

STATIC(Brisk::Entity::m_Scene);

#undef STATIC(a) decltype(a) a

namespace Brisk 
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle)
	{
		m_Scene = scene;
	}

	const std::string& Entity::GetName()
	{
		return GetComponent<TagComponent>().Tag;
	}

}