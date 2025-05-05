#pragma once
// INCLUDES
#include "Core/Core.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Buffer.hpp"
#include "Engine/Renderer/Descriptor.hpp"
//---------------
#include <memory>
//---------------

namespace Brisk
{
	enum GpuDescriptorResourceType {
		MVPUBO,
		SceneLightsUBO,
		SceneTextures,
		BindlessTextures,
	};

	class GpuAdapter  : public std::enable_shared_from_this<GpuAdapter> {
		DEFINE_BASE_CLASS_CONSTRUCTOR(GpuAdapter)
	public:
		template<typename T>
		std::shared_ptr<T> GetDevice() {
			//Engine::s_Application->GetGpuAdapter()->GetDevice<T>()->GetDevice();
			return std::dynamic_pointer_cast<T>(shared_from_this());
		}
		virtual void SetupDescriptorSets(std::vector<std::shared_ptr<DescriptorLayout>> descriptorLayouts) = 0;
		virtual void AddResource(GpuDescriptorResourceType type, std::shared_ptr<Texture> texture, std::shared_ptr<Buffer> buffer, int bindingIndex) = 0;

		virtual void Init() = 0;

		static std::shared_ptr<GpuAdapter> Create();
	};
}