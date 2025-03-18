#pragma once

#include "Engine/Renderer/Texture.hpp"
#include "Pipeline.hpp"

#include <memory>
#include <string>

namespace Brisk 
{
	class IBLTextureMaker {
	public:
		void EquirectangularMapPath(std::string path);

		void Init();
	private:
		std::shared_ptr<Texture> m_EquirectangularMap;
		std::shared_ptr<Texture> m_Cubemap;
		std::shared_ptr<Pipeline> m_ComputePipeline;
	};
}