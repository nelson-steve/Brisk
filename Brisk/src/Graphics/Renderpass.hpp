#pragma once

namespace Brisk {
	class RenderPass {
	public:
		virtual void Create() = 0;
		virtual void Release() = 0;
		
		virtual void BindPipeline(void* pipeline) = 0;
		virtual void BeginRenderPass() = 0;
		virtual void EndRenderPass() = 0;
	};
}
