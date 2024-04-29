#include "Engine/Engine.hpp"

int main(int args, char** argv) {
	Brisk::Engine::Init();
	Brisk::Engine::Update();
	Brisk::Engine::Terminate();
}