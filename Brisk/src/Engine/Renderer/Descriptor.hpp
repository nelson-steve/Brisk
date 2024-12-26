#pragma once

#include "Engine/Renderer/Buffer.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace Brisk
{
	enum ResourceType
	{
		Texture,
		Buffer,
	};

	class GPUResource
	{
	public:
		struct ResourceBinding
		{
			uint32_t binding;
			std::shared_ptr<Texture> texture;
			std::shared_ptr<Buffer> buffer;
			ResourceType ResourceType;
		};

	public:
		void Allocate(std::shared_ptr<Pipeline> pipeline);
		void UpdateResource();
		void Bind();

		void AddBinding(ResourceBinding binding) { m_Bindings.push_back(binding); }

	private:
		std::vector<ResourceBinding> m_Bindings;
	};

	class DescriptorLayout
	{
	public:
		struct Layout
		{
			uint32_t p_Binding;
			uint32_t p_DescriptorCount;
			ResourceType p_Type;
		};
		void AddBindingLayout(uint32_t binding, uint32_t count, ResourceType type)
		{
			Layout layout{};
			layout.p_Binding = binding;
			layout.p_DescriptorCount = count;
			layout.p_Type = type;
			m_Layouts.push_back(layout);
		}
		void AddBindingLayout(const std::vector<Layout> layouts)
		{
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