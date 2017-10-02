#include "debug.h"

#include <iostream>

namespace debug {
void printDebug(const std::string& msg) {
    std::cout << msg << std::endl;
    volatile unsigned int* p = reinterpret_cast<unsigned int*>(0xDEAD);
    *p;
}
}
