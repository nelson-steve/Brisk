workspace "Brisk"
    architecture "x64"
    startproject "Brisk"

    configurations 
    { 
        "Debug", 
        "Release" 
    }

    flags 
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

Vulkan_SDK = os.getenv("VULKAN_SDK")

if Vulkan_SDK == nil then
    error("VULKAN_SDK environment variable is not set. Please install the Vulkan SDK from LunarG.")
end

-- Convert backslashes to forward slashes
Vulkan_SDK = Vulkan_SDK:gsub("\\", "/")

IncludeDir = {}
IncludeDir["GLFW"]      = "%{wks.location}/Brisk/vendors/GLFW/include"
IncludeDir["imgui"]     = "%{wks.location}/Brisk/vendors/ImGui"
IncludeDir["Volk"]      = "%{wks.location}/Brisk/vendors/Volk"
IncludeDir["spdlog"]    = "%{wks.location}/Brisk/vendors/spdlog/include"
IncludeDir["stb_image"] = "%{wks.location}/Brisk/vendors/stb_image/include"
IncludeDir["json"]      = "%{wks.location}/Brisk/vendors/json/include"
IncludeDir["entt"]      = "%{wks.location}/Brisk/vendors/entt/include"
IncludeDir["glm"]       = "%{wks.location}/Brisk/vendors/glm"
IncludeDir["VMA"]       = "%{wks.location}/Brisk/vendors/VMA/include"
IncludeDir["SPIRV"]       = "%{wks.location}/Brisk/vendors/SPIRV-Reflect"
IncludeDir["fastgltf"]       = "%{wks.location}/Brisk/vendors/fastgltf/include"
IncludeDir["simdjson"]       = "%{wks.location}/Brisk/vendors/simdjson"
-- IncludeDir["Vulkan"] = Vulkan_SDK .. "/Include"

-- External libraries
group "Dependencies"
    include "Brisk/vendors/ImGui"
    include "Brisk/vendors/GLFW"
    include "Brisk/vendors/SPIRV-Reflect"
    include "Brisk/vendors/simdjson"
    include "Brisk/vendors/fastgltf"
group ""

project "Brisk"
    location "Brisk"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "Brisk/src/**.cpp",
        "Brisk/src/**.cc",
        "Brisk/src/**.hpp",
        "Brisk/src/**.h",
        "Brisk/src/**.c"
    }

    includedirs {
        "Brisk/src",
        "Brisk/vendors/GLFW/include",
        "Brisk/vendors/imgui",
        "Brisk/vendors/Volk",
        "Brisk/vendors/spdlog/include",
        "Brisk/vendors/stb_image/include",
        "Brisk/vendors/json/include",
        "Brisk/vendors/entt/include",
        "Brisk/vendors/glm",
        "Brisk/vendors/VMA/include",
        "Brisk/vendors/SPIRV-Reflect",
        "Brisk/vendors/simdjson",
        "Brisk/vendors/DirectX12Headers/include",
        "%{IncludeDir.fastgltf}",
        Vulkan_SDK .. "/Include"
    }

    libdirs {
        Vulkan_SDK .. "/Lib"
    }

    defines 
    { 
        "VK_NO_PROTOTYPES", 
        "_CRT_SECURE_NO_WARNINGS",
        'VMA_VOLK_HEADER_PATH="volk.h"'
    }


    links {
        "GLFW",
        "ImGui",
        "SPIRV-Reflect",
        "simdjson",
        "fastgltf",
    }

    filter "system:windows"
        systemversion "latest"
        defines 
        { 
            "BRISK_PLATFORM_WINDOWS", 
            "BRISK_ENABLE_DIRECTX12" 
        }
        links {
        "d3d12",
        "dxgi",
        "dxguid" -- For things like D3D12GetDebugInterface
    }
        -- links {
        --     "vulkan-1.lib"
        -- }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
