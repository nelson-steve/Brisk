project "SPIRV-Reflect"
    kind "StaticLib"
    language "C++"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "spirv_reflect.h",
        "spirv_reflect.c",

        "include/spirv/unified1/spirv.h",
    }

    includedirs {
        "%{IncludeDir.imgui}",
        Vulkan_SDK .. "/Include"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
