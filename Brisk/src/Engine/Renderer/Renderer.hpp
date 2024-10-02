#pragma once

namespace Brisk 
{
	template<typename RendererClass>
	class Renderer {
	public:
		void Init() {
			RendererClass::Init();		
		}
	};
}