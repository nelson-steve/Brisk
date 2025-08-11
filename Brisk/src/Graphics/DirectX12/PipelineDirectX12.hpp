#pragma once

// INCLUDES
#include "Engine/Renderer/Pipeline.hpp"
//-------------------------------------
#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <Core/Log.hpp>
//--------------------
using Microsoft::WRL::ComPtr;

namespace Brisk
{
	class PipelineDirectX12 : public Pipeline {
		struct ShaderResource {
			std::string p_Name;
			uint32_t p_Space;
			uint32_t p_Binding;
			uint32_t p_RootIndex;
			std::shared_ptr<Buffer> m_Buffer;
			std::shared_ptr<Buffer> m_Texture;
		};
	public:
		virtual void Init(const GraphicsPipelineSpecs& specs) override;
		virtual void Init(const ComputePipelineSpecs& specs) override;
		virtual void Release() override { BRISK_CORE_ASSERT(false); }

		virtual void Bind(std::shared_ptr<CommandBuffer> cmd) override;
		virtual void UpdateResources(const std::string& name, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<Buffer> buffer) override;
		virtual void BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t, void* data, uint32_t offset, bool vertexShader) override;
	private:
		std::vector<ShaderResource> m_ShaderResources;
		ComPtr<ID3D12RootSignature> m_RootSignature;
		ComPtr<ID3D12PipelineState> m_PipelineState;
		bool isDepth = false;
	};
}