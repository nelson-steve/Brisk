#include <string>

namespace Brisk 
{
	enum class GraphicsAPI {
		Vulkan,
		DirectX
	};

	struct EngineInfo {
		std::string EngineName = "Brisk";
		GraphicsAPI API = GraphicsAPI::Vulkan;
	};

	static class Engine 
	{
	public:
		static void Init();
	private:
		Engine();

	public:
		//static Engine* s_Instance;
		static EngineInfo s_EngineInfo;
	private:
	};
}