#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Brisk {
	class Model;
	struct Module {
		virtual ~Module() {}
	};

	struct Transform : public Module {
		glm::vec3 pPosition;
		glm::vec3 pRotation;
		glm::vec3 pScale;

		glm::mat4 GetMat() {
			glm::mat4 matrix = glm::mat4(1.0f); // Identity matrix

			// Apply translation
			matrix = glm::translate(matrix, pPosition);

			// Apply rotation (pitch, yaw, roll) - order: ZYX
			matrix = glm::rotate(matrix, glm::radians(pRotation.z), glm::vec3(0.0f, 0.0f, 1.0f)); // Roll (Z-axis)
			matrix = glm::rotate(matrix, glm::radians(pRotation.y), glm::vec3(0.0f, 1.0f, 0.0f)); // Yaw (Y-axis)
			matrix = glm::rotate(matrix, glm::radians(pRotation.x), glm::vec3(1.0f, 0.0f, 0.0f)); // Pitch (X-axis)

			// Apply scale
			matrix = glm::scale(matrix, pScale);

			return matrix;
		}
		Transform()
			: pPosition(glm::vec3(0.0f)), pRotation(glm::vec3(0.0f)), pScale(glm::vec3(1.0f)) {}
	};

	struct Renderable : public Module {
		Model* p_Model;

		Renderable() {
			//p_Model = new Model();
		}
	};

	struct BriskMaterial : public Module {
		//std::vector<Texture> p_Textures;

		BriskMaterial() {
			//p_Textures = new Textures();
		}
	};

	struct BriskShader : public Module {
		//std::vector<Shader> pShader;

		BriskShader() {
			//pShader = new Shader();
		}
	};
}