#pragma once

#include "Core/Transform.hpp"

namespace Brisk {
	class SceneObject {
	public:
		virtual void OnCreate() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnDestroy() = 0;
	private:
		Transform m_Transform;
	};
}