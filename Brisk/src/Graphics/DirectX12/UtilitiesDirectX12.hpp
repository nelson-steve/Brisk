#pragma once

#include "Engine/Renderer/RHI.hpp"

#include <d3d12.h>

namespace Brisk 
{
    class UtilitiesDirectX12 {
    public:
        static DXGI_FORMAT FormatToDXGIFormat(Core::Format format) {
            switch (format) {
                    //case Core::Format::FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
                case Core::Format::FORMAT_R32G32B32A32_SFLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
                case Core::Format::FORMAT_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
                case Core::Format::FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
                    //case Core::Format::FORMAT_R32G32B32_TYPELESS: return DXGI_FORMAT_R32G32B32_TYPELESS;
                case Core::Format::FORMAT_R32G32B32_SFLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
                case Core::Format::FORMAT_R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
                case Core::Format::FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
                    //case Core::Format::FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
                    //case Core::Format::FORMAT_R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
                case Core::Format::FORMAT_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
                case Core::Format::FORMAT_R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
                case Core::Format::FORMAT_R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
                case Core::Format::FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
                    //case Core::Format::FORMAT_R32G32_TYPELESS: return DXGI_FORMAT_R32G32_TYPELESS;
                case Core::Format::FORMAT_R32G32_SFLOAT: return DXGI_FORMAT_R32G32_FLOAT;
                case Core::Format::FORMAT_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
                    //case Core::Format::FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
                    //case Core::Format::FORMAT_R32G8X24_TYPELESS: return DXGI_FORMAT_R32G8X24_TYPELESS;
                    //case Core::Format::FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                    //case Core::Format::FORMAT_R32_FLOAT_X8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                    //case Core::Format::FORMAT_X32_TYPELESS_G8X24_UINT: return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
                    //case Core::Format::FORMAT_R10G10B10A2_TYPELESS: return DXGI_FORMAT_R10G10B10A2_TYPELESS;
                    //case Core::Format::FORMAT_R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
                    //case Core::Format::FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;
                    //case Core::Format::FORMAT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
                    //case Core::Format::FORMAT_R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
                case Core::Format::FORMAT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
                    //case Core::Format::FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                case Core::Format::FORMAT_R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
                case Core::Format::FORMAT_R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
                case Core::Format::FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
                    //case Core::Format::FORMAT_R16G16_TYPELESS: return DXGI_FORMAT_R16G16_TYPELESS;
                    //case Core::Format::FORMAT_R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
                case Core::Format::FORMAT_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
                case Core::Format::FORMAT_R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
                case Core::Format::FORMAT_R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
                case Core::Format::FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
                    //case Core::Format::FORMAT_R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
                    //case Core::Format::FORMAT_D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
                    //case Core::Format::FORMAT_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
                case Core::Format::FORMAT_R32_UINT: return DXGI_FORMAT_R32_UINT;
                case Core::Format::FORMAT_R32_SINT: return DXGI_FORMAT_R32_SINT;
                    //case Core::Format::FORMAT_R24G8_TYPELESS: return DXGI_FORMAT_R24G8_TYPELESS;
                case Core::Format::FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
                    //case Core::Format::FORMAT_R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                    //case Core::Format::FORMAT_X24_TYPELESS_G8_UINT: return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
                    //case Core::Format::FORMAT_R8G8_TYPELESS: return DXGI_FORMAT_R8G8_TYPELESS;
                case Core::Format::FORMAT_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
                case Core::Format::FORMAT_R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
                case Core::Format::FORMAT_R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
                case Core::Format::FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
                    //case Core::Format::FORMAT_R16_TYPELESS: return DXGI_FORMAT_R16_TYPELESS;
                    //case Core::Format::FORMAT_R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
                case Core::Format::FORMAT_D16_UNORM: return DXGI_FORMAT_D16_UNORM;
                case Core::Format::FORMAT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
                case Core::Format::FORMAT_R16_UINT: return DXGI_FORMAT_R16_UINT;
                case Core::Format::FORMAT_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
                case Core::Format::FORMAT_R16_SINT: return DXGI_FORMAT_R16_SINT;
                    //case Core::Format::FORMAT_R8_TYPELESS: return DXGI_FORMAT_R8_TYPELESS;
                case Core::Format::FORMAT_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
                case Core::Format::FORMAT_R8_UINT: return DXGI_FORMAT_R8_UINT;
                case Core::Format::FORMAT_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
                case Core::Format::FORMAT_R8_SINT: return DXGI_FORMAT_R8_SINT;
                    //case Core::Format::FORMAT_A8_UNORM: return DXGI_FORMAT_A8_UNORM;
                    //case Core::Format::FORMAT_R1_UNORM: return DXGI_FORMAT_R1_UNORM;
                    //case Core::Format::FORMAT_R9G9B9E5_SHAREDEXP: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                    //case Core::Format::FORMAT_R8G8_B8G8_UNORM: return DXGI_FORMAT_R8G8_B8G8_UNORM;
                    //case Core::Format::FORMAT_G8R8_G8B8_UNORM: return DXGI_FORMAT_G8R8_G8B8_UNORM;
                    //case Core::Format::FORMAT_BC1_TYPELESS: return DXGI_FORMAT_BC1_TYPELESS;
                    //case Core::Format::FORMAT_BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
                    //case Core::Format::FORMAT_BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
                    //case Core::Format::FORMAT_BC2_TYPELESS: return DXGI_FORMAT_BC2_TYPELESS;
                    //case Core::Format::FORMAT_BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
                    //case Core::Format::FORMAT_BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
                    //case Core::Format::FORMAT_BC3_TYPELESS: return DXGI_FORMAT_BC3_TYPELESS;
                    //case Core::Format::FORMAT_BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
                    //case Core::Format::FORMAT_BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
                    //case Core::Format::FORMAT_BC4_TYPELESS: return DXGI_FORMAT_BC4_TYPELESS;
                    //case Core::Format::FORMAT_BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
                    //case Core::Format::FORMAT_BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
                    //case Core::Format::FORMAT_BC5_TYPELESS: return DXGI_FORMAT_BC5_TYPELESS;
                    //case Core::Format::FORMAT_BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
                    //case Core::Format::FORMAT_BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
                    //case Core::Format::FORMAT_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
                    //case Core::Format::FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
                case Core::Format::FORMAT_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
                    //case Core::Format::FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
                    //case Core::Format::FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
                    //case Core::Format::FORMAT_B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
                    //case Core::Format::FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                    //case Core::Format::FORMAT_B8G8R8X8_TYPELESS: return DXGI_FORMAT_B8G8R8X8_TYPELESS;
                    //case Core::Format::FORMAT_B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
                    //case Core::Format::FORMAT_BC6H_TYPELESS: return DXGI_FORMAT_BC6H_TYPELESS;
                    //case Core::Format::FORMAT_BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
                    //case Core::Format::FORMAT_BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
                    //case Core::Format::FORMAT_BC7_TYPELESS: return DXGI_FORMAT_BC7_TYPELESS;
                    //case Core::Format::FORMAT_BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
                    //case Core::Format::FORMAT_BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
                    //case Core::Format::FORMAT_AYUV: return DXGI_FORMAT_AYUV;
                    //case Core::Format::FORMAT_Y410: return DXGI_FORMAT_Y410;
                    //case Core::Format::FORMAT_Y416: return DXGI_FORMAT_Y416;
                    //case Core::Format::FORMAT_NV12: return DXGI_FORMAT_NV12;
                    //case Core::Format::FORMAT_P010: return DXGI_FORMAT_P010;
                    //case Core::Format::FORMAT_P016: return DXGI_FORMAT_P016;
                    //case Core::Format::FORMAT_420_OPAQUE: return DXGI_FORMAT_420_OPAQUE;
                    //case Core::Format::FORMAT_YUY2: return DXGI_FORMAT_YUY2;
                    //case Core::Format::FORMAT_Y210: return DXGI_FORMAT_Y210;
                    //case Core::Format::FORMAT_Y216: return DXGI_FORMAT_Y216;
                    //case Core::Format::FORMAT_NV11: return DXGI_FORMAT_NV11;
                    //case Core::Format::FORMAT_AI44: return DXGI_FORMAT_AI44;
                    //case Core::Format::FORMAT_IA44: return DXGI_FORMAT_IA44;
                    //case Core::Format::FORMAT_P8: return DXGI_FORMAT_P8;
                    //case Core::Format::FORMAT_A8P8: return DXGI_FORMAT_A8P8;
                    //case Core::Format::FORMAT_B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM;
            }
            assert(false);
        }

        inline static const char* SemanticFromLocation(uint32_t location) {
            switch (location) {
                case 0: return "POSITION";
                case 1: return "NORMAL";
                case 2: return "TEXCOORD0";
                case 3: return "TEXCOORD1";
                case 4: return "COLOR";
                case 5: return "TANGENT";
                case 6: return "JOINTINDICES";
                case 7: return "JOINTWEIGHTS";
            }
            assert(false);
        }

        inline static uint32_t SemanticIndexFromLocation(uint32_t location) {
            switch (location) {
                case 0: return 0; // POSITION
                case 1: return 0; // NORMAL
                case 2: return 0; // TEXCOORD0
                case 3: return 1; // TEXCOORD1
                case 4: return 0; // COLOR
                case 5: return 0; // TANGENT
                case 6: return 0; // JOINTINDICES
                case 7: return 0; // JOINTWEIGHTS
            }
            assert(false);
        }
    };
}