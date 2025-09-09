#pragma once

#include "../Engine/Application.hpp"
#include "../Core/Log.hpp"

extern Brisk::Application* CreateApplication();

int main(int args, char** argv) {
	Brisk::Log::Init();

	auto app = CreateApplication();
	app->Run();
	delete app;
}
