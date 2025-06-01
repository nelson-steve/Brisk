project "fastgltf"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "include/**.hpp",        
        "src/**.cpp",        
        "src/**.ixx",        
    }

    includedirs {
        "%{IncludeDir.simdjson}",
        "%{IncludeDir.fastgltf}"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
