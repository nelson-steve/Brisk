#pragma once

#include <memory>

namespace Brisk 
{
	class Renderer {
	public:
		void Init();
		//void Destroy();
  //      void BeginScene();
  //      void EndScene();


  //      void Present();

		static std::unique_ptr<Renderer> Create() { return nullptr; }
	};
}