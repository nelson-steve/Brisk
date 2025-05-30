workspace "Brisk"
    architecture "x86_64"
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
    cppdialect "C++20"
    staticruntime "on"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

Vulkan_SDK = os.getenv("VULKAN_SDK")

if Vulkan_SDK == nil then
    error("VULKAN_SDK environment variable is not set. Please install the Vulkan SDK from LunarG.")
end

LibraryDirVulkan = Vulkan_SDK .. "/Lib"


IncludeDir = {}
IncludeDir["GLFW"]      = "%{wks.location}/Brisk/vendors/GLFW/include"
IncludeDir["imgui"]     = "%{wks.location}/Brisk/vendors/ImGui"
IncludeDir["Volk"]      = "%{wks.location}/Brisk/vendors/Volk"
IncludeDir["spdlog"]    = "%{wks.location}/Brisk/vendors/spdlog/include"
IncludeDir["stb_image"] = "%{wks.location}/Brisk/vendors/stb_image/include"
IncludeDir["json"]      = "%{wks.location}/Brisk/vendors/json/include"
IncludeDir["entt"]      = "%{wks.location}/Brisk/vendors/entt/include"
IncludeDir["glm"]       = "%{wks.location}/Brisk/vendors/glm"
IncludeDir["tiny_gltf"]       = "%{wks.location}/Brisk/vendors/tiny_gltf/include"
IncludeDir["Vulkan"]    = Vulkan_SDK .. "/Include"

-- External libraries
group "Dependencies"
    include "Brisk/vendors/ImGui"
    include "Brisk/vendors/GLFW"
group ""

project "Brisk"
    location "Brisk"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "Brisk/src/**.cpp",
        "Brisk/src/**.cc",
        "Brisk/src/**.hpp",
        "Brisk/src/**.h",
        "Brisk/src/**.c"
    }

    print("Resolved GLFW IncludeDir: ", path.getabsolute("vendors/GLFW/include"))
    print("Resolved GLFW IncludeDir: ", Vulkan_SDK)

    includedirs {
        "Brisk/src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.Volk}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.json}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.tiny_gltf}",
        "%{IncludeDir.Vulkan}",


        -- "vendors/spdlog/include",
        -- "vendors/tiny_gltf",
        -- "vendors/stb_image",
        -- "vendors/json",
        -- "vendors/entt/include",
        -- "vendors/include", -- optional general include
        -- "vendors/glm",
        -- "vendors/ImGui",
        -- "vendors/glfw/include"
    }

    defines { "VK_NO_PROTOTYPES" }

    links {
        "ImGui",
        "GLFW",
    }

    filter "system:windows"
        systemversion "latest"
        defines { "BRISK_PLATFORM_WINDOWS" }
        -- links {
        --     "vulkan-1.lib"
        -- }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
