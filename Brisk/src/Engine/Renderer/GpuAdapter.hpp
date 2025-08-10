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
	// Predefined descriptor set values
	constexpr uint32_t SET_FRAME_GLOBAL = 0; // Camera, lights, materials
	constexpr uint32_t SET_BINDLESS_TEXTURES = 1; // Global bindless textures
	constexpr uint32_t SET_PER_MESH = 2; //  Deferred textures
	constexpr uint32_t SET_CLUSTERED_LIGHTING = 3; // Clustered lighting
	//

	class GpuAdapter  : public std::enable_shared_from_this<GpuAdapter> {
		DEFINE_BASE_CLASS_CONSTRUCTOR(GpuAdapter)
	public:
		template<typename T>
		std::shared_ptr<T> GetDevice() {
			//Engine::s_Application->GetGpuAdapter()->GetDevice<T>()->GetDevice();
			return std::dynamic_pointer_cast<T>(shared_from_this());
		}
		virtual void WaitIdle() = 0;

		virtual void Init() = 0;
		virtual void Release() = 0;
		virtual void ReleasePools() = 0;

		static std::shared_ptr<GpuAdapter> Create();
	};
}