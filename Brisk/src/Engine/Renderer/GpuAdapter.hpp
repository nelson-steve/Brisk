#pragma once
// INCLUDES
#include "Core/Core.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Buffer.hpp"
//---------------
#include <memory>
//---------------

namespace Brisk
{
	enum GpuResourceType {
		MVPUBO,
		SceneLights,
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

		virtual void AddResource(GpuResourceType type, std::shared_ptr<Texture> texture) = 0;
		virtual void AddResource(GpuResourceType type, std::shared_ptr<Buffer> buffer) = 0;

		virtual void Init() = 0;

		static std::shared_ptr<GpuAdapter> Create();
	};
}