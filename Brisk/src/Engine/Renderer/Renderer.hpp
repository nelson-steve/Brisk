#pragma once

namespace Brisk 
{
	//struct RenderTarget {
	//	//VkDescriptorSet pDescriptorSet;
	//	BriskTexture* pTexture;
	//};

	template<typename RendererClass>
	class Renderer {
	public:
		void Init() {
			RendererClass::Init();		
		}
	};

	//class Renderer {
	//public:
	//	virtual ~Renderer() {};

	//	virtual void Create() = 0;
	//	virtual void Release() = 0;

	//	virtual void AddRenderTarget(RenderTarget renderTarget) = 0;

	//	virtual void SetupRenderingPipeline(Swapchain* swapchain) = 0;

	//	virtual void PreRender() = 0;
	//	virtual void Render() = 0;
	//	virtual void PostRender() = 0;
	//	virtual void WaitDeviceIdle() = 0;
	//};
}