#include "pch.hpp"
#include "ImGuiLayer.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Vulkan/GpuAdapterVulkan.hpp"
#include "Graphics/Vulkan/RenderpassVulkan.hpp"
#include "Graphics/DirectX12/GpuAdapterDirectX12.hpp"
#include "Graphics/DirectX12/CommandBufferDirectX12.hpp"

#include "ImGuiBackends/imgui_impl_dx12.h"
#include <ImGuiBackends/imgui_impl_win32.h>

#include <memory>

namespace Brisk
{
    void SetLightGreenishTheme() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (light but not bright)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.85f, 0.9f, 0.88f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f };       // Sea-greenish
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.5f, 0.75f, 0.65f, 1.0f }; // Brighter sea-green on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.35f, 0.6f, 0.5f, 1.0f };   // Active state

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f };        // Sea-greenish buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.5f, 0.75f, 0.65f, 1.0f }; // Lighter hover
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.35f, 0.6f, 0.5f, 1.0f };   // Darker active

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.8f, 0.85f, 0.83f, 1.0f };       // Light frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.9f, 0.95f, 0.93f, 1.0f }; // Slightly brighter when hovered
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.7f, 0.75f, 0.73f, 1.0f };  // Active frame background

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f };            // Sea-greenish tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.5f, 0.75f, 0.65f, 1.0f };     // Brighter hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.45f, 0.7f, 0.6f, 1.0f };       // Slightly brighter when active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.35f, 0.6f, 0.5f, 1.0f };    // Dimmed when unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f }; // Slightly brighter when active and unfocused

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.35f, 0.6f, 0.5f, 1.0f };         // Sea-greenish title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.4f, 0.65f, 0.55f, 1.0f };  // Slightly brighter when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.3f, 0.55f, 0.45f, 1.0f }; // Dimmed for collapsed title

    }

    void SetDarkGreenishTheme() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (darker)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.15f, 0.13f, 1.0f }; // Dark, slight greenish tint

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f };       // Dark sea-greenish
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.2f, 0.55f, 0.45f, 1.0f }; // Brighter sea-green on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.1f, 0.35f, 0.3f, 1.0f };   // Slightly darker when active

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f };        // Dark sea-greenish buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.2f, 0.55f, 0.45f, 1.0f }; // Lighter hover
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.1f, 0.35f, 0.3f, 1.0f };   // Darker active

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.12f, 0.2f, 0.18f, 1.0f };       // Dark frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.15f, 0.35f, 0.3f, 1.0f }; // Sea-green hover
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.1f, 0.3f, 0.25f, 1.0f };  // Active state

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f };            // Sea-green tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.2f, 0.55f, 0.45f, 1.0f };     // Brighter on hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.18f, 0.5f, 0.4f, 1.0f };       // Slightly brighter when active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.12f, 0.35f, 0.3f, 1.0f };   // Dimmed when unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f }; // Bright when unfocused but active

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.1f, 0.3f, 0.25f, 1.0f };         // Dark sea-greenish title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.4f, 0.35f, 1.0f };  // Slightly brighter when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.08f, 0.25f, 0.2f, 1.0f }; // Dimmed for collapsed title

    }

    void LavenderTheme() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (dark base)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f }; // Darker, neutral tone

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f };        // Muted lavender
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.2f, 0.15f, 0.25f, 1.0f }; // Subtle pinkish lavender on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f }; // Slightly darker lavender when active

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f };        // Muted lavender buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.2f, 0.15f, 0.25f, 1.0f }; // Pinkish hover effect
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f }; // Slightly pinkish lavender for active

        // Frame Background
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.12f, 0.1f, 0.15f, 1.0f };       // Dark lavender frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.2f, 0.15f, 0.25f, 1.0f }; // Subtle pinkish lavender on hover
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f }; // Active state with muted lavender

        //colors[ImGuiCol_MenuBarBg] = ImVec4{ 1.0f, 0.14f, 0.0f, 1.0f }; // Active state with muted lavender
        colors[ImGuiCol_PopupBg] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f }; // Active state with muted lavender

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f };           // Muted lavender tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.2f, 0.15f, 0.25f, 1.0f };    // Subtle pinkish lavender on hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f };    // Slightly darker for active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.1f, 0.1f, 0.12f, 1.0f };   // Darker when unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.15f, 0.12f, 0.2f, 1.0f }; // Brighter when active and unfocused

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };         // Darker title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.18f, 0.14f, 0.22f, 1.0f }; // Muted lavender when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f }; // Very dark for collapsed title

        colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.08f, 0.08f, 0.1f, 1.0f }; // Very dark for collapsed title
    }

    void MonotoneDarkTheme()
    {
        auto& colors = ImGui::GetStyle().Colors;

        // Base background
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.08f, 0.08f, 0.08f, 1.0f };  // Dark gray
        colors[ImGuiCol_PopupBg] = ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };     // Slightly lighter for popups

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.12f, 0.12f, 0.12f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.12f, 0.12f, 0.12f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };

        // Frame background (inputs, sliders)
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.13f, 0.13f, 0.13f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.12f, 0.12f, 0.12f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.15f, 0.15f, 0.15f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.12f, 0.12f, 0.12f, 1.0f };

        // Title bar
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.08f, 0.08f, 0.08f, 1.0f };

        // Menu bar
        colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.08f, 0.08f, 0.08f, 1.0f };

        // Optional: other subtle elements
        colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };
        colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.2f, 0.2f, 0.2f, 1.0f };
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.25f, 0.25f, 0.25f, 1.0f };
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.22f, 0.22f, 0.22f, 1.0f };
    }


    void PinkTheme() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (bright, pinkish tone)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.9f, 0.78f, 0.85f, 1.0f }; // Bright pinkish background

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };        // Bright pink
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.9f, 0.6f, 0.75f, 1.0f };  // Slightly brighter pink on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.8f, 0.5f, 0.65f, 1.0f };   // Slightly darker pink when active

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };        // Bright pink buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.9f, 0.6f, 0.75f, 1.0f };  // Brighter pink hover effect
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.8f, 0.5f, 0.65f, 1.0f };   // Slightly darker pink for active

        // Frame Background
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.8f, 0.55f, 0.65f, 1.0f };       // Pink frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.9f, 0.6f, 0.75f, 1.0f }; // Brighter pink on hover
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f }; // Active state with bright pink

        // Popup
        colors[ImGuiCol_PopupBg] = ImVec4{ 0.9f, 0.65f, 0.75f, 1.0f };       // Bright pink popup background

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };           // Bright pink tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.9f, 0.6f, 0.75f, 1.0f };     // Brighter pink on hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.8f, 0.5f, 0.65f, 1.0f };      // Slightly darker for active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.75f, 0.5f, 0.6f, 1.0f };   // Slightly muted pink for unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f }; // Bright when active and unfocused

        // Text
        colors[ImGuiCol_Text] = ImVec4{ 0.75f, 0.45f, 0.65f, 1.0f };         // Bright pink title background
        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.75f, 0.45f, 0.65f, 1.0f };         // Bright pink title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };    // Brighter pink when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.7f, 0.4f, 0.6f, 1.0f };   // Muted pink for collapsed title

    }

    void PinkTheme2() {
        auto& colors = ImGui::GetStyle().Colors;

        // Background (less bright, solid tone)
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.25f, 0.2f, 0.28f, 1.0f }; // Darker, more solid background

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f };        // Darker, solid pink
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.7f, 0.4f, 0.55f, 1.0f };  // Slightly lighter on hover
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };   // Even darker when active

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f };        // Solid pink buttons
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.7f, 0.4f, 0.55f, 1.0f };  // Brighter on hover
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };   // Darker pink when active

        // Frame Background
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.55f, 0.3f, 0.4f, 1.0f };        // Solid pink frame background
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f }; // Brighter on hover
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };  // Darker when active

        // Popup
        colors[ImGuiCol_PopupBg] = ImVec4{ 0.3f, 0.25f, 0.35f, 1.0f };       // Solid background for popups

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };           // Darker pink tabs
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.7f, 0.4f, 0.55f, 1.0f };    // Lighter on hover
        colors[ImGuiCol_TabActive] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f };    // Brighter when active
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.4f, 0.25f, 0.35f, 1.0f }; // Muted pink for unfocused
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.55f, 0.3f, 0.45f, 1.0f }; // Brighter when active and unfocused

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.45f, 0.25f, 0.35f, 1.0f };         // Solid title background
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.55f, 0.3f, 0.45f, 1.0f };    // Brighter when active
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.35f, 0.2f, 0.3f, 1.0f };  // Darker for collapsed title

        // Text
        colors[ImGuiCol_Text] = ImVec4{ 0.9f, 0.9f, 0.9f, 1.0f };               // Brighter text for contrast

        // Scrollbars
        colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.25f, 0.2f, 0.28f, 1.0f };      // Solid scrollbar background
        colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.55f, 0.3f, 0.4f, 1.0f };     // Pink grab
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f }; // Lighter on hover
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.6f, 0.3f, 0.45f, 1.0f };  // Darker when active

        // Checkboxes
        colors[ImGuiCol_CheckMark] = ImVec4{ 0.9f, 0.9f, 0.9f, 1.0f };           // White for contrast on checkboxes

        // Slider
        colors[ImGuiCol_SliderGrab] = ImVec4{ 0.55f, 0.3f, 0.4f, 1.0f };         // Solid pink grab
        colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f };  // Brighter on active

        // Separators
        colors[ImGuiCol_Separator] = ImVec4{ 0.35f, 0.2f, 0.3f, 1.0f };          // Solid, darker separator
        colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.4f, 0.25f, 0.35f, 1.0f };  // Slightly brighter on hover
        colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.45f, 0.3f, 0.4f, 1.0f };    // Brighter when active

        // Resize grip
        colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.55f, 0.3f, 0.4f, 1.0f };         // Pink grip
        colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.65f, 0.35f, 0.5f, 1.0f }; // Brighter on hover
        colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.7f, 0.4f, 0.55f, 1.0f };   // Brighter when active

        // Modals
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4{ 0.2f, 0.15f, 0.25f, 0.75f }; // Dimmed background for modals

        // DragDrop
        colors[ImGuiCol_DragDropTarget] = ImVec4{ 0.85f, 0.55f, 0.7f, 1.0f };    // Brighter pink for drag-and-drop target

    }

    void ImGuiLayer::OnAttach() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.WantCaptureMouse = true;
        io.WantCaptureKeyboard = true;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;
        //io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        io.FontDefault = io.Fonts->AddFontFromFileTTF("../Assets/Fonts/Nunito/Nunito-Medium.ttf", 18.0f);

        MonotoneDarkTheme();
#ifdef BRISK_ENABLE_DIRECTX12
        auto gpuAdapter = std::static_pointer_cast<GpuAdapterDirectX12>(Application::GetGpuAdapter());
        // 1. Create descriptor heap for ImGui fonts/textures
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        desc.NodeMask = 0;
        gpuAdapter->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&imguiSrvDescHeap));

        uint32_t numFramesInFlight = 3;
        ImGui_ImplWin32_Init(Application::GetWindow()->GetHWNDWindowHandle());
        ImGui_ImplDX12_Init(gpuAdapter->GetDevice().Get(),
            numFramesInFlight,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            imguiSrvDescHeap,
            imguiSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            imguiSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

#else
        ImGui_ImplGlfw_InitForVulkan((GLFWwindow*)Application::GetWindow()->GetWindowHandle(), true);

        auto gpuAdapter = std::static_pointer_cast<GpuAdapterVulkan>(Application::GetGpuAdapter());

        ImGui_ImplVulkan_InitInfo info{};
        info.Instance = gpuAdapter->GetInstance();
        info.PhysicalDevice = gpuAdapter->GetPhysicalDevice();
        info.Device = gpuAdapter->GetDevice();
        info.QueueFamily = 0;
        info.Queue = gpuAdapter->GetGraphicsQueue();
        info.DescriptorPool = gpuAdapter->GetDescriptorPool();
        info.RenderPass = std::static_pointer_cast<RenderPassVulkan>(Application::GetRenderer()->m_UIPass)->GetRenderPass();
        info.ImageCount = 2;
        info.MinImageCount = 2;
        info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        info.Allocator = nullptr;

        ImGui_ImplVulkan_Init(&info);

        Application::GetRenderer()->m_ImGuiIdScene = Application::GetRenderer()->m_TonemapOutput->AddTextureToImGui();
        Application::GetRenderer()->m_ImGuiIdShadowMap0 = Application::GetRenderer()->m_ShadowMapLOD0->AddTextureToImGui();
        Application::GetRenderer()->m_ImGuiIdShadowMap1 = Application::GetRenderer()->m_ShadowMapLOD1->AddTextureToImGui();
        Application::GetRenderer()->m_ImGuiIdShadowMap2 = Application::GetRenderer()->m_ShadowMapLOD2->AddTextureToImGui();
        Application::GetRenderer()->m_ImGuiIdShadowMap3 = Application::GetRenderer()->m_ShadowMapLOD3->AddTextureToImGui();


        //VkCommandBuffer commandBuffer = std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get();

        //vkResetCommandPool(info.Device, gpuAdapter->GetGraphicsCommandPool(), 0);

        //VkCommandBufferBeginInfo beginInfo = {};
        //beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        ////vkBeginCommandBuffer(commandBuffer, &beginInfo);

        ////vkEndCommandBuffer(commandBuffer);

        //VkSubmitInfo submitInfo = {};
        //submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        //submitInfo.commandBufferCount = 1;
        ////submitInfo.pCommandBuffers = &commandBuffer;

        //vkQueueSubmit(info.Queue, 1, &submitInfo, VK_NULL_HANDLE);
        //vkDeviceWaitIdle(info.Device); // Wait until done
#endif

        // 6. Destroy Temporary Font Upload Resources
        //ImGui_ImplVulkan_DestroyFontUploadObjects(); // REQUIRED!
    }

    void ImGuiLayer::Begin() {
#ifdef BRISK_ENABLE_DIRECTX12
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
#else
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
#endif
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    }

    void ImGuiLayer::End() {
        ImGui::Render();
    }

    void ImGuiLayer::OnEvent(Event& e) {

    }

    void ImGuiLayer::Render(std::shared_ptr<CommandBuffer> cmd) {
#ifdef BRISK_ENABLE_DIRECTX12
        std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get()->SetDescriptorHeaps(1, &imguiSrvDescHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), std::static_pointer_cast<CommandBufferDirectX12>(cmd)->Get().Get());
#else
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), std::static_pointer_cast<CommandBufferVulkan>(cmd)->Get());
#endif
    }

    void ImGuiLayer::OnDetach() {
#ifdef BRISK_ENABLE_DIRECTX12

#else
        ImGui_ImplVulkan_Shutdown();
#endif
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

