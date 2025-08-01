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
	class GpuAdapter  : public std::enable_shared_from_this<GpuAdapter> {
		DEFINE_BASE_CLASS_CONSTRUCTOR(GpuAdapter)
	public:
		template<typename T>
		std::shared_ptr<T> GetDevice() {
			//Engine::s_Application->GetGpuAdapter()->GetDevice<T>()->GetDevice();
			return std::dynamic_pointer_cast<T>(shared_from_this());
		}
		virtual void WaitIdle() = 0;

		virtual void LogDirectXDebugs() = 0; // DirectX12 only

		virtual void Init() = 0;
		virtual void Release() = 0;
		virtual void ReleasePools() = 0;

		static std::shared_ptr<GpuAdapter> Create();
	};
}