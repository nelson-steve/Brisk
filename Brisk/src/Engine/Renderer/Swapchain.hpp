#pragma once

#include "Engine/Window.hpp"
#include <Engine/Renderer/Semaphore.hpp>
#include <Engine/Renderer/Texture.hpp>
#include <Engine/Renderer/CommandBuffer.hpp>
#include <Engine/Renderer/Fence.hpp>

namespace Brisk 
{
	struct SwapchainExtent2D {
		uint32_t Width;
		uint32_t Height;
	};

	class Swapchain {
	public:
		enum Mode {
			DOUBLE_BUFFERING = 2U,
			TRIPLE_BUFFERING = 3U,
		};
	public:
		virtual void Create(Mode mode = Mode::DOUBLE_BUFFERING) = 0;
		virtual void Release() = 0;
		virtual void Resize() = 0;

		virtual bool AcquireNextImage(uint64_t timeout, std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Fence> fence, uint32_t* pImageIndex) = 0;
		virtual void TransitionCurrentImage(std::shared_ptr<CommandBuffer> cmd, Texture::ImageBarrierParams params, int imageIndex) = 0;
		virtual void Blit(std::shared_ptr<CommandBuffer> cmd, std::shared_ptr<Texture> image, int imageIndex) = 0;
		uint32_t GetImageCount() const { return m_ImageCount; }

		virtual uint32_t GetExtentWidth() const = 0;
		virtual uint32_t GetExtentHeight() const = 0;
	protected:
		std::shared_ptr<Window> m_Window;
		uint32_t m_ImageCount;
	private:
		Mode m_SwapchainMode;
	};
}