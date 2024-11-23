#pragma once

#include "Descriptor.hpp"
#include "Pipeline.hpp"

#include <vector>
#include <cstdint>

namespace Brisk 
{
	class Shader {
	public:
		enum ShaderType {
			VERTEX,
			FRAGMENT,
		};

		virtual void Init(std::pair<std::string, Pipeline::ShaderStage>) = 0;

		inline void AddDescriptorLayout(const std::shared_ptr<DescriptorLayout> descriptor) { m_DescriptorLayout = descriptor; }
		inline const std::shared_ptr<DescriptorLayout> GetDescriptorLayout() const { return m_DescriptorLayout; }

		static std::shared_ptr<Shader> Create();
	public:
		std::shared_ptr<DescriptorLayout> m_DescriptorLayout;
	};
}