#pragma once
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
	public:
		std::vector<Binding> 
	};
}