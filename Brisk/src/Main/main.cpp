#include "Engine/Engine.hpp"

int allocatedSize = 0;
int deallocatedSize = 0;

#include <d3d12.h>

//void* operator new(size_t size) {
//    std::cout << "Allocating " << size << " bytes" << std::endl;
//    allocatedSize += size;
//    std::cout << "Total allocated: " << allocatedSize << " bytes" << std::endl;
//    if (size >= 256) {
//        std::cout << "";
//    }
//
//    void* p = std::malloc(size);
//    if (!p) {
//        throw std::bad_alloc();
//    }
//    return p;
//}
//
//void operator delete(void* p, size_t size) noexcept {
//    deallocatedSize += size;
//    std::cout << "Total deallocated: " << deallocatedSize << " bytes" << std::endl;
//    std::cout << "Deallocating " << size << " bytes" << std::endl;
//    std::free(p);
//}

#ifdef _WIN32

// Handle to the window
HWND hwnd = NULL;

// name of the window (not the title)
LPCTSTR WindowName = L"BzTutsApp";

// title of the window
LPCTSTR WindowTitle = L"Bz Window";

// width and height of the window
int Width = 800;
int Height = 600;

// is window full screen?
bool FullScreen = false;

// create a window
bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool fullscreen) {
	if (fullscreen)
	{
		HMONITOR hmon = MonitorFromWindow(hwnd,
			MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	}
}

// main application loop
void mainloop();


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	// create the window
	if (!InitializeWindow(hInstance, nCmdShow, Width, Height, FullScreen))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}
}

#else

int main(int args, char** argv) {
	Brisk::Engine::Init();
	Brisk::Engine::Update();
	Brisk::Engine::Terminate();

	return 0;
}

#endif