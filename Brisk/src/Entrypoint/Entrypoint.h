#pragma once

#include "../Engine/Application.hpp"
#include "../Core/Log.hpp"

extern std::unique_ptr<Brisk::Application> CreateApplication();

int main(int args, char** argv) {
	Brisk::Log::Init();

	{
		std::unique_ptr<Brisk::Application> app = CreateApplication();
		app->Run();
	}
}
