#include "GraphicsDeviceVulkan.hpp"
#include "Engine/Engine.hpp"
#include "Defines.h"

#include <GLFW/glfw3.h>

namespace Brisk 
{
	VkInstance GraphicsDeviceVulkan::s_Instance;

	void GraphicsDeviceVulkan::Create(){
		VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
		appInfo.pApplicationName = "Demo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = Engine::s_EngineInfo.EngineName.c_str();
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> extensions = GetRequiredExtensions();

		VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.enabledExtensionCount = static_cast<int>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		
		VK_LOG(vkCreateInstance(&createInfo, nullptr, &s_Instance), "Failed to create Vulkan instance");
	}

	std::vector<const char*> GraphicsDeviceVulkan::GetRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		return extensions;
	}
}