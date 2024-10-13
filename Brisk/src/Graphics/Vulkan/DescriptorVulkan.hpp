#pragma once 

#include "Engine/Renderer/Descriptor.hpp"

namespace Brisk
{
	class DescriptorVulkan : public Descriptor {
		virtual void AddLayout() override;
		virtual void Allocate() override;
		virtual void Update() override;
	};
}