#pragma once

namespace Brisk {
	class Model;
	struct Module {
		virtual ~Module() {}
	};

	struct Transform : public Module {
		glm::vec3 pPosition;
		glm::quat pRotation;
		glm::vec3 pScale;
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