#include "Engine/Engine.hpp"

namespace Brisk
{
	Engine::Engine() {

	}

	void Engine::Init() {
		if (Instance != nullptr) {
			delete Instance;
		}
		Instance = new Engine();
	}
}