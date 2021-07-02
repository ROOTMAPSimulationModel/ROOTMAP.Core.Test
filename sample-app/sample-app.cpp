// This is an extremely barebones sample application. Its sole purpose is to embed the ROOTMAP.Core library in a runnable C++ application
// to ensure the library passes the most basic 'does it work' test.

#include "pch.h"
#include <iostream>
#include "core/common/RmVersion.h"

int main()
{
    std::cout << "ROOTMAP.Core sample client application." << std::endl;
    std::cout << "Version: " << rootmapVersion << std::endl;
}
