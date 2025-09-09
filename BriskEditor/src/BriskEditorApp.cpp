#include <Entrypoint/Entrypoint.h>
#include "Editor.hpp"

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

Brisk::Application* CreateApplication() {
	return new Brisk::BriskEditorApp();
}