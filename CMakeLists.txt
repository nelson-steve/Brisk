cmake_minimum_required(VERSION 3.2)

project(Brisk)

set (CMAKE_CXX_STANDARD 20)

set(SOURCES
    #Core
    Brisk/src/Core/Log.cpp
    #Editor
    Brisk/src/Editor/Editor.cpp        
    Brisk/src/Editor/AssetsPanel.cpp        
    Brisk/src/Editor/ConsolePanel.cpp        
    Brisk/src/Editor/GamePanel.cpp        
    Brisk/src/Editor/HeirarchyPanel.cpp        
    Brisk/src/Editor/InspectorPanel.cpp        
    Brisk/src/Editor/ScenePanel.cpp    
    #Engine
    Brisk/src/Engine/Application.cpp
    Brisk/src/Engine/Camera.cpp
    Brisk/src/Engine/Engine.cpp
    Brisk/src/Engine/Model.cpp
    Brisk/src/Engine/Scene.cpp
    Brisk/src/Engine/Window.cpp
    #Renderer
    Brisk/src/Engine/Renderer/Descriptor.cpp
    Brisk/src/Engine/Renderer/Shader.cpp
    Brisk/src/Engine/Renderer/GpuAdapter.cpp
    Brisk/src/Engine/Renderer/Pipeline.cpp
    Brisk/src/Engine/Renderer/Renderer.cpp
    Brisk/src/Engine/Renderer/RenderPass.cpp
    #Graphics
    Brisk/src/Graphics/BriskTexture.cpp
    Brisk/src/Graphics/Swapchain.cpp
    #Vulkan
    Brisk/src/Graphics/Vulkan/BufferVulkan.cpp
    Brisk/src/Graphics/Vulkan/CommandBufferVulkan.cpp
    Brisk/src/Graphics/Vulkan/DescriptorVulkan.cpp
    Brisk/src/Graphics/Vulkan/FramebufferVulkan.cpp
    Brisk/src/Graphics/Vulkan/GpuAdapterVulkan.cpp
    Brisk/src/Graphics/Vulkan/PipelineVulkan.cpp
    Brisk/src/Graphics/Vulkan/RenderPassVulkan.cpp
    Brisk/src/Graphics/Vulkan/ShaderVulkan.cpp
    Brisk/src/Graphics/Vulkan/SwapchainVulkan.cpp
    Brisk/src/Graphics/Vulkan/TextureVulkan.cpp
    Brisk/src/Graphics/Vulkan/UtilitiesVulkan.cpp
    Brisk/src/Graphics/Vulkan/volk.cpp
    #VulkanRenderer
    Brisk/src/Graphics/Vulkan/VulkanRenderer/RendererVulkan.cpp
    #main
    Brisk/src/Main/main.cpp
    #Windows
    Brisk/src/Platform/Windows/WindowsWindow.cpp

    Brisk/src/tiny_gltf.cc
)

set(HEADERS
    #Core
    Brisk/src/Core/Log.hpp
    Brisk/src/Core/Timer.hpp        
    #Editor
    Brisk/src/Editor/Editor.hpp        
    Brisk/src/Editor/IEditorPanel.hpp    
    Brisk/src/Editor/AssetsPanel.hpp    
    Brisk/src/Editor/GamePanel.hpp    
    Brisk/src/Editor/ConsolePanel.hpp    
    Brisk/src/Editor/ScenePanel.hpp
    Brisk/src/Editor/InspectorPanel.hpp    
    Brisk/src/Editor/HeirarchyPanel.hpp    
    #Engine
    Brisk/src/Engine/Application.hpp
    Brisk/src/Engine/AssetManager.hpp    
    Brisk/src/Engine/Camera.hpp 
    Brisk/src/Engine/Element.hpp
    Brisk/src/Engine/Engine.hpp
    Brisk/src/Engine/EngineGlobal.hpp
    Brisk/src/Engine/Model.hpp
    Brisk/src/Engine/Module.hpp
    Brisk/src/Engine/Scene.hpp
    Brisk/src/Engine/Window.hpp
    #Renderer
    Brisk/src/Engine/Renderer/Descriptor.hpp
    Brisk/src/Engine/Renderer/Framebuffer.hpp
    Brisk/src/Engine/Renderer/GpuAdapter.hpp
    Brisk/src/Engine/Renderer/GraphicsPipeline.hpp
    Brisk/src/Engine/Renderer/Pipeline.hpp
    Brisk/src/Engine/Renderer/Renderer.hpp
    Brisk/src/Engine/Renderer/RendererFactory.hpp
    Brisk/src/Engine/Renderer/RenderingPipeline.hpp
    Brisk/src/Engine/Renderer/RenderPass.hpp
    Brisk/src/Engine/Renderer/RHI.hpp
    Brisk/src/Engine/Renderer/Shader.hpp
    #Graphics
    Brisk/src/Graphics/BriskTexture.hpp
    Brisk/src/Graphics/Swapchain.hpp
    #Factories
    Brisk/src/Graphics/Factories/SwapchainFactory.hpp
    #Vulkan
    Brisk/src/Graphics/Vulkan/Defines.h
    Brisk/src/Graphics/Vulkan/BufferVulkan.hpp
    Brisk/src/Graphics/Vulkan/CommandBufferVulkan.hpp
    Brisk/src/Graphics/Vulkan/DescriptorVulkan.hpp
    Brisk/src/Graphics/Vulkan/FramebufferVulkan.hpp
    Brisk/src/Graphics/Vulkan/GpuAdapterVulkan.hpp
    Brisk/src/Graphics/Vulkan/PipelineVulkan.hpp
    Brisk/src/Graphics/Vulkan/RenderPassVulkan.hpp
    Brisk/src/Graphics/Vulkan/ShaderVulkan.hpp
    Brisk/src/Graphics/Vulkan/SurfaceFactoryVulkan.hpp
    Brisk/src/Graphics/Vulkan/SwapchainVulkan.hpp
    Brisk/src/Graphics/Vulkan/TextureVulkan.hpp
    Brisk/src/Graphics/Vulkan/UtilitiesVulkan.hpp
    #VulkanRenderer
    Brisk/src/Graphics/Vulkan/VulkanRenderer/RendererVulkan.hpp
    #Windows
    Brisk/src/Platform/Windows/WindowsWindow.hpp
)

add_subdirectory(${PROJECT_SOURCE_DIR}/Brisk/vendors/ImGui)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

add_definitions(-DVK_NO_PROTOTYPES)

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        ${PROJECT_SOURCE_DIR}/Brisk/src
        ${PROJECT_SOURCE_DIR}/Brisk/vendors/include
        ${PROJECT_SOURCE_DIR}/Brisk/vendors/tiny_gltf
        ${PROJECT_SOURCE_DIR}/Brisk/vendors/spdlog/include
        ${PROJECT_SOURCE_DIR}/Brisk/vendors/stb_image
        ${PROJECT_SOURCE_DIR}/Brisk/vendors/json
        ${PROJECT_SOURCE_DIR}/Brisk/vendors/entt/include
)


set(glm_DIR ${PROJECT_SOURCE_DIR}/Brisk/vendors/glm/cmake/glm)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/Brisk/vendors/spdlog/include
    ${PROJECT_SOURCE_DIR}/Brisk/vendors/include 
    ${Vulkan_INCLUDE_DIRS} 
    ${OPENGL_INCLUDE_DIR}
)

#target_link_libraries(${PROJECT_NAME} ImGui)
#target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
#target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/Brisk/vendors/lib/glfw3.lib)
#target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/Brisk/vendors/spdlog/lib/Debug/spdlogd.lib)
#target_link_libraries(${PROJECT_NAME} glm::glm)

target_link_libraries(${PROJECT_NAME} ImGui)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/Brisk/vendors/lib-mingw-w64/libglfw3.a)
# target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/Brisk/vendors/lib/glfw3.lib)
# target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/Brisk/vendors/spdlog/lib/Debug/spdlogd.lib)
target_link_libraries(${PROJECT_NAME} glm::glm)