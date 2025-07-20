#pragma once

// INCLUDES
#include "Engine/Renderer/Buffer.hpp"
#include "Engine/Renderer/Texture.hpp"
//----------------------------
#include <cstdint>
#include <memory>
#include <vector>
//---------------

namespace Brisk
{
	class GPUResource {
	public:
		enum ResourceType {
			DESCRIPTOR_TYPE_SAMPLER = 0,
			DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
			DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
			DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
			DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
			DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
			DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
			DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
			DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
			DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
			DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
			DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK = 1000138000,
			DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR = 1000150000,
			DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
			DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM = 1000440000,
			DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM = 1000440001,
			DESCRIPTOR_TYPE_MUTABLE_EXT = 1000351000,
		};
		enum ShaderStageAccess {
			SHADER_STAGE_VERTEX_BIT,
			SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			SHADER_STAGE_GEOMETRY_BIT,
			SHADER_STAGE_FRAGMENT_BIT,
			SHADER_STAGE_COMPUTE_BIT,
			SHADER_STAGE_ALL_GRAPHICS,
			SHADER_STAGE_ALL,
			SHADER_STAGE_RAYGEN_BIT_KHR,
			SHADER_STAGE_ANY_HIT_BIT_KHR,
			SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
			SHADER_STAGE_MISS_BIT_KHR,
			SHADER_STAGE_INTERSECTION_BIT_KHR,
			SHADER_STAGE_CALLABLE_BIT_KHR,
			SHADER_STAGE_TASK_BIT_EXT,
			SHADER_STAGE_MESH_BIT_EXT,
			SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI,
			SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI,
		};
		struct ResourceBinding {
			uint32_t binding;
			std::shared_ptr<Texture> texture;
			std::shared_ptr<Buffer> buffer;
			ResourceType ResourceType;
		};

	public:
		void AddBinding(ResourceBinding binding) { m_Bindings.push_back(binding); }
	private:
		std::vector<ResourceBinding> m_Bindings;
	};

	class DescriptorLayout {
		DEFINE_BASE_CLASS_CONSTRUCTOR(DescriptorLayout)
	public:
		struct Layout {
			uint32_t p_Binding;
			uint32_t p_DescriptorCount;
			GPUResource::ResourceType p_Type;
			std::vector<GPUResource::ShaderStageAccess> pStageAccessFlags;
		};

		void AddBinding(uint32_t binding, uint32_t count, GPUResource::ResourceType type, std::vector<GPUResource::ShaderStageAccess> stages) {
			Layout layout{};
			layout.p_Binding = binding;
			layout.p_DescriptorCount = count;
			layout.p_Type = type;
			layout.pStageAccessFlags = stages;
			m_Layouts.push_back(layout);
		}
		void AddBindings(const std::vector<Layout> layouts) {
			for (const Layout &l : layouts)
			{
				Layout layout{};
				layout.p_Binding = l.p_Binding;
				layout.p_DescriptorCount = l.p_DescriptorCount;
				layout.p_Type = l.p_Type;
				m_Layouts.push_back(l);
			}
		}
	public:
		virtual void Init() = 0;
		static std::shared_ptr<DescriptorLayout> Create();
	protected:
		std::vector<Layout> m_Layouts;
	};
}