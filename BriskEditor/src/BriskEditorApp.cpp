#include <Entrypoint/Entrypoint.h>
#include "EditorLayer.hpp"

namespace Brisk 
{
	class BriskEditorApp : public Application {
	public:
		BriskEditorApp() 
			:Application("Brisk Editor") {
			PushLayer(new EditorLayer());
			BRISK_APP_INFO("Brisk Editor created");
		}
	};
}

std::unique_ptr<Brisk::Application> CreateApplication() {
	return std::make_unique<Brisk::BriskEditorApp>();
}