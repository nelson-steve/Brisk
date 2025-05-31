project "GLFW"
    kind "StaticLib"
    language "C"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "include/GLFW/glfw3.h",
        "include/GLFW/glfw3native.h",
        "src/glfw_config.h",
        "src/context.c",
        "src/init.c",
        "src/input.c",
        "src/monitor.c",

        "src/null_init.c",
        "src/null_joystick.c",
        "src/null_monitor.c",
        "src/null_window.c",

        "src/platform.c",
        "src/vulkan.c",
        "src/window.c",
    }

    filter "system:linux"
        pic "On"

        systemversion "latest"
        
        files
        {
            "src/x11_init.c",
            "src/x11_monitor.c",
            "src/x11_window.c",
            "src/xkb_unicode.c",
            "src/posix_time.c",
            "src/posix_thread.c",
            "src/glx_context.c",
            "src/egl_context.c",
            "src/osmesa_context.c",
            "src/linux_joystick.c"
        }

        defines
        {
            "_GLFW_X11"
        }

    filter "system:windows"
        systemversion "latest"

        files
        {
            "src/win32_init.c",
            "src/win32_joystick.c",
            "src/win32_module.c",
            "src/win32_monitor.c",
            "src/win32_time.c",
            "src/win32_thread.c",
            "src/win32_window.c",
            "src/wgl_context.c",
            "src/egl_context.c",
            "src/osmesa_context.c"
        }

        defines 
        { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

        links
        {
            "Dwmapi.lib"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

-- IncludeDir = {}
-- IncludeDir["GLFW"] = "%{wks.location}/Brisk/vendors/GLFW/include"
-- GLFWRoot = "%{wks.location}/Brisk/vendors/GLFW"

-- project "GLFW"
--     kind "StaticLib"
--     language "C"
--     staticruntime "on"

--     targetdir ("bin/" .. outputdir .. "/%{prj.name}")
--     objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

--     files {
--         GLFWRoot .. "/include/GLFW/glfw3.h",
--         GLFWRoot .. "/include/GLFW/glfw3native.h",

--         GLFWRoot .. "/src/context.c",
--         GLFWRoot .. "/src/init.c",
--         GLFWRoot .. "/src/input.c",
--         GLFWRoot .. "/src/monitor.c",
--         GLFWRoot .. "/src/vulkan.c",
--         GLFWRoot .. "/src/window.c",
--     }

--     filter "system:windows"
--         files {
--             GLFWRoot .. "/src/win32_init.c",
--             GLFWRoot .. "/src/win32_joystick.c",
--             GLFWRoot .. "/src/win32_monitor.c",
--             GLFWRoot .. "/src/win32_module.c",
--             GLFWRoot .. "/src/win32_time.c",
--             GLFWRoot .. "/src/win32_thread.c",
--             GLFWRoot .. "/src/win32_window.c",
--             GLFWRoot .. "/src/win32_platform.c",
--             GLFWRoot .. "/src/wgl_context.c",
--             GLFWRoot .. "/src/egl_context.c",
--             GLFWRoot .. "/src/platform.c",

--             -- GLFWRoot .. "/src/osmesa_context.c",  -- add this
--             -- GLFWRoot .. "/src/null_init.c",       -- add this
--             -- GLFWRoot .. "/src/null_platform.h",   -- add this            
--         }
--         defines { 
--             "_GLFW_WIN32",
--             "_CRT_SECURE_NO_WARNINGS",
--             "GLFW_USE_OSMESA=0",
--             "GLFW_USE_PLATFORM_NULL=0",            
--         }
--         links {
--             "gdi32",
--             "user32",
--             "shell32",
--             "Dwmapi.lib"
--         }
--         systemversion "latest"

--     includedirs {
--         GLFWRoot .. "/include",
--     }

--     filter "configurations:Debug"
--         symbols "On"

--     filter "configurations:Release"
--         optimize "On"
