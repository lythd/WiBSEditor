/* buildermanager.h
PURPOSE:
- Interfaces with the builder
*/
#ifndef BUILDERMANAGER_H
#define BUILDERMANAGER_H

#include <string>

class BuilderManager {
public:
    std::string build(const std::string& cmd);

private:
    std::string execCommand(const std::string& cmd);
    
    const std::string dependencyDir = 
#ifdef _WIN32
        std::string(getenv("LOCALAPPDATA")) + "\\.wibsbuilder\\build\\wibsb.exe";
#else
        std::string(getenv("HOME")) + "/.wibsbuilder/build/wibsb";
#endif
};

#endif // BUILDERMANAGER_H
