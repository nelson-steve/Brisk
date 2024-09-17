#pragma once

#include "SceneObject.hpp"
#include "Camera.hpp"

namespace Brisk {
	class SceneCamera : public SceneObject {
	public:
		virtual void OnCreate() override;
		virtual void OnUpdate() override;
		virtual void OnDestroy() override;
	private:
		Camera* m_Camera;
	};
}