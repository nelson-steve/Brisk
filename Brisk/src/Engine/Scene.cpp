#include "Scene.hpp"

namespace Brisk {

	void BriskScene::Update() {
		SceneRenderer::Begin(this);
		SceneRenderer::End();
	}
}