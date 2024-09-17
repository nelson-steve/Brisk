#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Brisk {
	struct Transform {
		glm::vec3 Position;
		glm::quat Orientation;
		glm::vec3 Scale;

		Transform()
			: Position(glm::vec3(0.0f)), Orientation(glm::quat()), Scale(glm::vec3(1.0f)) { }

		Transform(glm::vec3 pos, glm::quat rot, glm::vec3 scale)
			:Position(pos), Orientation(rot), Scale(scale) { }
	};
}