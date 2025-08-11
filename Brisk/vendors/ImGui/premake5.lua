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
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",        
        "imgui_internal.h",        
        
        "imgui.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "imgui_demo.cpp",

        -- Vulkan backend
        "ImGuiBackends/imgui_impl_vulkan.h",
        "ImGuiBackends/imgui_impl_vulkan.cpp",

        "ImGuiBackends/imgui_impl_dx12.h",
        "ImGuiBackends/imgui_impl_dx12.cpp",

        "ImGuiBackends/imgui_impl_win32.h",
        "ImGuiBackends/imgui_impl_win32.cpp",

        "ImGuiBackends/imgui_impl_glfw.h",
        "ImGuiBackends/imgui_impl_glfw.cpp"        
    }

    includedirs {
        "%{IncludeDir.imgui}",
        "%{IncludeDir.GLFW}",
        "ImGuiBackends/",
        Vulkan_SDK .. "/Include"
    }

    defines 
    {
        "IMGUI_IMPL_VULKAN_USE_VOLK"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
