#include "Engine/Engine.hpp"

int main(int args, char** argv) {
	Brisk::Engine::InitSystems();
	Brisk::Engine::Run();
	Brisk::Engine::Terminate();
}
