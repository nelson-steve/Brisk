#pragma once

#include <iostream>

// Custom global new operator
void* operator new(size_t size) {
    std::cout << "Allocating " << size << " bytes" << std::endl;
    void* p = std::malloc(size);
    if (!p) {
        throw std::bad_alloc();
    }
    return p;
}