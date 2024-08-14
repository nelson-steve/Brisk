#pragma once

namespace Brisk {
	class RenderPass {
	public:
		static RenderPass* Create();
		static void Release(RenderPass* renderPass);
		
		virtual void BindPipeline(void* pipeline) = 0;
		virtual void BeginRenderPass() = 0;
		virtual void EndRenderPass() = 0;
	};
}
