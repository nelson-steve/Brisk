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

		enum DescriptorType {
			UNIFORM_BUFFER,
			STORAGE_BUFFER,
			SAMPLER,
			COMBINED_IMAGE_SAMPLER,
			INPUT_ATTACHMENT,
		};

		struct DescriptorLayout {
			uint32_t p_Binding;
			DescriptorType p_DescriptorType;
			uint32_t p_DescriptorCount;
		};

		struct ShaderSpecs {
			ShaderType p_ShaderType;
			DescriptorLayout p_DescriptorLayout;
		};

		virtual void Init(std::pair<std::string, Pipeline::ShaderStage>) = 0;
		//virtual inline void AddDescriptor(const std::shared_ptr<Descriptor> descriptor) { m_Descriptors.push_back(descriptor); }

		//const std::vector<std::shared_ptr<Descriptor>> GetDescriptors() const { return m_Descriptors; }

		static std::shared_ptr<Shader> Create();
	public:
		//std::vector<std::shared_ptr<Descriptor>> m_Descriptors;
	};
}