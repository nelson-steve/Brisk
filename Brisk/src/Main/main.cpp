#include "Engine/Engine.hpp"

int allocatedSize = 0;
int deallocatedSize = 0;

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

int main(int args, char** argv) {
	Brisk::Engine::Init();
	Brisk::Engine::Update();
	Brisk::Engine::Terminate();
}