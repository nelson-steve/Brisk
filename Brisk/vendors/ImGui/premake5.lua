Vulkan_SDK = os.getenv("VULKAN_SDK")

if Vulkan_SDK == nil then
    error("VULKAN_SDK environment variable is not set. Please install the Vulkan SDK from LunarG.")
end

-- Convert backslashes to forward slashes
Vulkan_SDK = Vulkan_SDK:gsub("\\", "/")

IncludeDir = {}
IncludeDir["imgui"]  = "%{wks.location}/Brisk/vendors/ImGui"
IncludeDir["GLFW"]   = "%{wks.location}/Brisk/vendors/GLFW/include"

-- IncludeDir["Vulkan"] = Vulkan_SDK .. "/Include"

print("Vulkan SDK path: " .. (Vulkan_SDK or "NOT FOUND"))

project "ImGui"
    kind "StaticLib"
    language "C++"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        -- Core
        "imconfig.h",
        "imgui.h",
        "imgui_rectpack.h",
        "imgui_textedit.h",
        "imgui_truetype.h",        
        "imgui_internal.h",        
        
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "imgui_demo.cpp",

        -- Vulkan backend
        "ImGuiBackends/imgui_impl_vulkan.h",
        "ImGuiBackends/imgui_impl_vulkan.cpp",

        "ImGuiBackends/imgui_impl_glfw.h",
        "ImGuiBackends/imgui_impl_glfw.cpp"        
    }

    includedirs {
        "%{IncludeDir.imgui}",
        "%{IncludeDir.GLFW}",
        "ImGuiBackends/",
        Vulkan_SDK .. "/Include"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
