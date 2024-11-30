#pragma once

#include "Scene.hpp"
#include "Core/Core.hpp"

namespace Brisk {

	class SceneSerializer {
	public:
		SceneSerializer(const Ref<Scene> scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool DeserializeRuntime(const std::string& filepath);
		bool Deserialize(const std::string& filepath);

	private:
		Ref<Scene> m_Scene;
	};

}