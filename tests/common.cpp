#include <filesystem>
#include <functional>
#include <iostream>
namespace fs = std::filesystem;

fs::path GetProjRoot() {
    // linux only line to acquire the path of the executable
    fs::path exec_path(fs::canonical("/proc/self/exe"));
    if (exec_path.empty()) {
        // if we're on windows try ./samples as the relative path
        return fs::path("."); // probably won't work
    } else {
        // if we're on linux, then get the directory the executable is stored in
        exec_path = exec_path.parent_path();
        // if that directory is the build/bin, then our relative path is ../../samples
        if (exec_path.string().find("build/debug/bin") != std::string::npos) {
            return fs::canonical(exec_path.string() + "/../../../");
        }
    }
    return fs::canonical(exec_path.string() + "/../../");
}

void TestWithDicomFiles(std::function<void(const fs::path&)> test){
    auto samples = fs::path(GetProjRoot().string() + "/samples/");
    // recurse sample directory, parse every file as a dicom
    std::cout << "reading from: " << samples << std::endl;
    fs::recursive_directory_iterator recursive_iter(samples);
    for(auto &entry : recursive_iter){
        auto path = entry.path();
        if(!fs::is_directory(path)) {
            if (path.extension().string() == ".DCM") {
                test(path);
            }
        }
    }
}

#include <random>
#include <sstream>

namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}