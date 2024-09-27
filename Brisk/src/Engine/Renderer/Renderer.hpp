#pragma once

#include "Graphics/Swapchain.hpp"
#include "Graphics/BriskTexture.hpp"

namespace Brisk 
{
	struct RenderTarget {
		//VkDescriptorSet pDescriptorSet;
		BriskTexture* pTexture;
	};

	template<typename TRenderer>
	class RenderManager {
	public:
		void TestRenderFunction() {
			TRenderer::Render();
		}
	};

	struct VulkanRenderer {
		static void Render() {

		}
	};

	struct DirectX12Renderer {
		static void Render() {

		}
	};

	class Renderer {
	public:
		virtual ~Renderer() {};

		virtual void Create() = 0;
		virtual void Release() = 0;

		virtual void AddRenderTarget(RenderTarget renderTarget) = 0;

		virtual void SetupRenderingPipeline(Swapchain* swapchain) = 0;

		virtual void PreRender() = 0;
		virtual void Render() = 0;
		virtual void PostRender() = 0;
		virtual void WaitDeviceIdle() = 0;
	};
}