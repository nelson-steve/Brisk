#include "Engine/Engine.hpp"

int allocatedSize = 0;
int deallocatedSize = 0;

#include <d3d12.h>
#include <string>
#include <windows.h>

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

//#ifdef _WIN32
#ifdef dddd

#define MAX_NAME_STRING 256
#define HInstance() GetModuleHandle(NULL)

WCHAR			WindowClass[MAX_NAME_STRING];
WCHAR			WindowTitle[MAX_NAME_STRING];

INT				WindowWidth;
INT				WindowHeight;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int nCmdShow)
{
	/* - Initialize Global Variables - */

	wcscpy_s(WindowClass, TEXT("TutorialOneClass"));
	wcscpy_s(WindowTitle, TEXT("Our First Window"));
	WindowWidth = 1366;
	WindowHeight = 768;

	/* - Create Window Class - */

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;

	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);

	wcex.hIcon = LoadIcon(0, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(0, IDI_APPLICATION);

	wcex.lpszClassName = WindowClass;

	wcex.lpszMenuName = nullptr;

	wcex.hInstance = HInstance();

	wcex.lpfnWndProc = DefWindowProc;

	RegisterClassEx(&wcex);

	/* - Create and Display our Window  - */

	HWND hWnd = CreateWindow(WindowClass, WindowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, WindowWidth, WindowHeight, nullptr, nullptr, HInstance(), nullptr);
	if (!hWnd) {
		MessageBox(0, L"Failed to Create Window!.", 0, 0);
		return 0;
	}


	ShowWindow(hWnd, SW_SHOW);

	/* - Listen for Message events - */

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	return 0;
}

#else

int main(int args, char** argv) {
	Brisk::Engine::Init();
	Brisk::Engine::Update();
	Brisk::Engine::Terminate();

	return 0;
}

#endif