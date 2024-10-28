/* buildermanager.cpp
PURPOSE:
- Interfaces with the builder
*/
#include "buildermanager.h"
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

std::string BuilderManager::execCommand(const std::string& cmd) {
    std::string result;

#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe) {
        throw std::runtime_error("Failed to open pipe");
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return result;
}

std::string BuilderManager::build(const std::string& cmd) {
    return execCommand(dependencyDir + " " + cmd);
}