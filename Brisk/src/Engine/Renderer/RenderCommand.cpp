// INCLUDES
#include "RenderCommand.hpp"
#include "ComputeCommand.hpp"
//-------------------------

namespace Brisk
{
	RendererAPI* RenderCommand::s_RendererAPI = RendererAPI::Create();

	ComputeAPI* ComputeCommand::s_ComputeAPI = ComputeAPI::Create();
}