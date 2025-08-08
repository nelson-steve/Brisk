#pragma once

// INCLUDES
#include "Engine/Renderer/Pipeline.hpp"
//-------------------------------------
#include <cassert>
#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
//--------------------
using Microsoft::WRL::ComPtr;

namespace Brisk
{
	class PipelineDirectX12 : public Pipeline {
	public:
		virtual void Init(const GraphicsPipelineSpecs& specs) override;
		virtual void Init(const ComputePipelineSpecs& specs) override;
		virtual void Release() override { assert(false); }

		virtual void Bind(std::shared_ptr<CommandBuffer> cmd) override;
		virtual void UpdateResources(const std::string& name, std::vector<std::shared_ptr<Texture>> textures, std::shared_ptr<Buffer> buffer) override {}
		virtual void BindPushConstant(std::shared_ptr<CommandBuffer> cmd, uint32_t, void* data, uint32_t offset, bool vertexShader) override;
	private:
		ComPtr<ID3D12RootSignature> m_RootSignature;
		ComPtr<ID3D12PipelineState> m_PipelineState;
		bool isDepth = false;
	};
}