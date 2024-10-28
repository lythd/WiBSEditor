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
    void updateBuilder();

private:
    std::string execCommand(const std::string& cmd);
    bool dependencyExists();
    std::string getInstalledVersion();
    std::string getLatestVersion();
    void downloadAndBuildDependency();
    void updateVersionFile(const std::string& version);
    
    const std::string repoUrl = "https://github.com/username/project.git";
#ifdef _WIN32
    const std::string dependencyDir = std::string(getenv("LOCALAPPDATA")) + "\\.wibsbuilder";
    const std::string versionFile = dependencyDir + "\\VERSION";
    const std::string buildDir = dependencyDir + "\\build";
    const std::string targetExec = buildDir + "\\wibsb.exe";
#else
    const std::string dependencyDir = std::string(getenv("HOME")) + "/.wibsbuilder";
    const std::string versionFile = dependencyDir + "/VERSION";
    const std::string buildDir = dependencyDir + "/build";
    const std::string targetExec = buildDir + "/wibsb";
#endif

};

#endif // BUILDERMANAGER_H
