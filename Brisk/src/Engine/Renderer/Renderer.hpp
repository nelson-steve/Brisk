#pragma once

#include <memory>

namespace Brisk 
{
	class Renderer {
	public:
		void Init() {
		}

		static std::unique_ptr<Renderer> Create();
	};
}