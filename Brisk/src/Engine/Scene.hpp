#pragma once

#include "SceneObject.hpp"

#include <vector>

namespace Brisk {
	class Scnene {
	public:
		void Create() {};
		void Update() {};
		void Destroy() {};

		const std::vector<SceneObject*> GetObjects() const { return m_Objects; }

		void AddObject(SceneObject* obj) {
			m_Objects.push_back(obj);
		}
	private:
		std::vector<SceneObject*> m_Objects;
	};
}