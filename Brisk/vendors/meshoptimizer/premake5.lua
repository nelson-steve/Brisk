project "MeshOptimizer"
    kind "StaticLib"
    language "C++"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "src/meshoptimizer.h",
        
        "src/allocator.cpp",
        "src/clusterizer.cpp",
        "src/indexanalyzer.cpp",
        "src/indexcodec.cpp",
        "src/indexgenerator.cpp",
        "src/overdrawoptimizer.cpp",
        "src/partition.cpp",
        "src/quantization.cpp",
        "src/rasterizer.cpp",
        "src/simplifier.cpp",
        "src/spatialorder.cpp",
        "src/stripifier.cpp",
        "src/vcacheoptimizer.cpp",
        "src/vertexcodec.cpp",
        "src/vertexfilter.cpp",
        "src/vfetchoptimizer.cpp",
    }

    includedirs {
        -- "%{IncludeDir.imgui}",
        -- "%{IncludeDir.GLFW}",
        -- "ImGuiBackends/",
        -- Vulkan_SDK .. "/Include"
    }

    defines 
    {
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
