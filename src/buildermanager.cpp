/* buildermanager.cpp
PURPOSE:
- Interfaces with the builder
*/
#include "buildermanager.h"
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstdlib>

#ifdef _WIN32
    #define GIT "git"
    #define CURL "curl"
    #define CMAKE "cmake"
    #define BUILD_COMMAND "cmake --build . --config Release"
    #include <windows.h>
#else
    #define GIT "git"
    #define CURL "curl"
    #define MAKE "make"
    #define BUILD_COMMAND "make"
    #include <unistd.h>
#endif

namespace fs = std::filesystem;

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
    return execCommand(targetExec + " " + cmd);
}

bool BuilderManager::dependencyExists() {
    return fs::exists(targetExec) && fs::exists(versionFile);
}

std::string BuilderManager::getInstalledVersion() {
    std::ifstream vFile(versionFile);
    std::string version;
    if (vFile.is_open()) {
        std::getline(vFile, version);
        vFile.close();
    }
    return version;
}

std::string BuilderManager::getLatestVersion() {
    std::string latestVersion;
    std::string command = CURL " -s https://api.github.com/repos/username/projectA/releases/latest | grep '\"tag_name\"' | cut -d '\"' -f 4 > latest_version";
    std::system(command.c_str());
    
    std::ifstream latestVersionFile("latest_version");
    if (latestVersionFile.is_open()) {
        std::getline(latestVersionFile, latestVersion);
        latestVersionFile.close();
    }
    
    fs::remove("latest_version");
    return latestVersion;
}

void BuilderManager::downloadAndBuildDependency() {
    if (!fs::exists(dependencyDir)) {
        fs::create_directory(dependencyDir);
    }

    // Clone the repository
    std::string cloneCommand = GIT " clone " + repoUrl + " " + dependencyDir;
    std::system(cloneCommand.c_str());

    // Create build directory if it doesn't exist (only needed for CMake on Windows)
    if (!fs::exists(buildDir)) {
        fs::create_directory(buildDir);
    }

#ifdef _WIN32
    // Windows: Use CMake to generate and build
    std::string cmakeCommand = "cmake -S " + dependencyDir + " -B " + buildDir;
    std::system(cmakeCommand.c_str());
    std::system(BUILD_COMMAND.c_str());
#else
    // Linux/macOS: Use Make directly in the cloned repository
    std::system(("make -C " + dependencyDir).c_str());
#endif
}

void BuilderManager::updateVersionFile(const std::string& version) {
    std::ofstream vFile(versionFile, std::ios::trunc);
    if (vFile.is_open()) {
        vFile << version;
        vFile.close();
    }
}

void BuilderManager::updateBuilder() {
    std::cout << "Checking for dependency in " << dependencyDir << "...\n";
    
    // Get installed and latest versions
    std::string installedVersion = getInstalledVersion();
    std::string latestVersion = getLatestVersion();
    
    // Check if installation is needed
    if (installedVersion != latestVersion || !dependencyExists()) {
        std::cout << "Updating dependency...\n";
        downloadAndBuildDependency();
        updateVersionFile(latestVersion);
        std::cout << "Dependency updated to version " << latestVersion << ".\n";
    } else {
        std::cout << "Dependency is already up-to-date (version " << installedVersion << ").\n";
    }
}
