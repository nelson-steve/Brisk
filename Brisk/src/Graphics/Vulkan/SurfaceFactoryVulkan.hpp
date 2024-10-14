#pragma once

#include "Core/Log.hpp"
#include "Engine/Engine.hpp"

#include <Volk/volk.h>
#include <vulkan/vulkan_win32.h>
#include <glfw3.h>

//#ifdef _WIN32
//#define VK_USE_PLATFORM_WIN32_KHR
//#elif defined(__ANDROID__)
//#define VK_USE_PLATFORM_ANDROID_KHR
//#elif defined(__APPLE__)
//#define VK_USE_PLATFORM_MACOS_MVK
//#elif defined(__linux__)
//#define VK_USE_PLATFORM_XCB_KHR
//#endif

namespace Brisk {
    // forward declaration
    class SurfaceFactoryVulkan;

    class SurfaceVulkan {
    public:
        inline const VkSurfaceKHR GetSurface() const { return m_Surface; }
        inline VkSurfaceKHR& GetRef() { return m_Surface; }
    private:
        SurfaceVulkan() = default;
    private:
        VkSurfaceKHR m_Surface;

        friend class SurfaceFactoryVulkan;
    };

    class SurfaceFactoryVulkan {
    public:
        [[nodiscard]] static std::shared_ptr<SurfaceVulkan> CreateNativeSurface(VkInstance instance) {
            std::shared_ptr<SurfaceVulkan> surface = std::make_shared<SurfaceVulkan>();
            VkResult err;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hinstance = (HINSTANCE)platformHandle;
            surfaceCreateInfo.hwnd = (HWND)platformWindow;
            err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
            VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.window = window;
            err = vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, NULL, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
            VkIOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
            surfaceCreateInfo.pNext = NULL;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.pView = view;
            err = vkCreateIOSSurfaceMVK(instance, &surfaceCreateInfo, nullptr, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
            VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
            surfaceCreateInfo.pNext = NULL;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.pView = view;
            err = vkCreateMacOSSurfaceMVK(instance, &surfaceCreateInfo, NULL, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
            VkSurfaceKHR surface;
            VkXlibSurfaceCreateInfoKHR createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            createInfo.dpy = display; // Pointer to the Xlib Display
            createInfo.window = window; // Xlib Window
            err = vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_GGP)
            VkSurfaceKHR surface;
            VkStreamDescriptorSurfaceCreateInfoGGP createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP;
            createInfo.streamDescriptor = streamDescriptor; // GGP Stream Descriptor
            vkCreateGgpSurfaceKHR(instance, &createInfo, nullptr, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_METAL_EXT) // TODO: FIX
            VkSurfaceKHR surface;
            VkDisplaySurfaceCreateInfoKHR createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
            // Additional setup specific to the display and mode...
            vkCreateDisplayPlaneSurfaceKHR(instance, &createInfo, nullptr, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
            VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
            surfaceCreateInfo.pNext = NULL;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.pLayer = metalLayer;
            err = vkCreateMetalSurfaceEXT(instance, &surfaceCreateInfo, NULL, &surface->m_Surface);
#elif defined(_DIRECT2DISPLAY)
            createDirect2DisplaySurface(width, height);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
            VkDirectFBSurfaceCreateInfoEXT surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_DIRECTFB_SURFACE_CREATE_INFO_EXT;
            surfaceCreateInfo.dfb = dfb;
            surfaceCreateInfo.surface = window;
            err = vkCreateDirectFBSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
            VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.display = display;
            surfaceCreateInfo.surface = window;
            err = vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
            VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.connection = connection;
            surfaceCreateInfo.window = window;
            err = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
#elif defined(VK_USE_PLATFORM_HEADLESS_EXT)
            VkHeadlessSurfaceCreateInfoEXT surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_HEADLESS_SURFACE_CREATE_INFO_EXT;
            PFN_vkCreateHeadlessSurfaceEXT fpCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateHeadlessSurfaceEXT");
            if (!fpCreateHeadlessSurfaceEXT) {
                vks::tools::exitFatal("Could not fetch function pointer for the headless extension!", -1);
            }
            err = fpCreateHeadlessSurfaceEXT(instance, &surfaceCreateInfo, nullptr, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_SCREEN_QNX)
            VkScreenSurfaceCreateInfoQNX surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
            surfaceCreateInfo.pNext = NULL;
            surfaceCreateInfo.flags = 0;
            surfaceCreateInfo.context = screen_context;
            surfaceCreateInfo.window = screen_window;
            err = vkCreateScreenSurfaceQNX(instance, &surfaceCreateInfo, NULL, &surface->m_Surface);
#elif defined(VK_USE_PLATFORM_DIRECTFB_EXT)
            VkDirectFBSurfaceCreateInfoEXT surfaceCreateInfo{}
                surfaceCreateInfo.
                err = vkCreateDirectFBSurfaceEXT
#elif defined(VK_USE_PLATFORM_VI_NN)
            err = vkCreateViSurfaceNN()
#else
            if (glfwCreateWindowSurface(instance, (GLFWwindow*)Engine::s_Application->GetWindow()->GetWindowHandle(), nullptr, &surface->m_Surface) != VK_SUCCESS) {
                BRISK_CORE_ERROR("Failed to create window surface!");
#endif
            }
            return surface;
        }
    };
}