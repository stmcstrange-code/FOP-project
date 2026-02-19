#include "FileHandler.h"

void saveProject(const std::vector<Block>& script, const std::string & path) {
    std::ofstream file(path);
    if (file.is_open()) {
        for (const auto& b : script) {
            file << static_cast<int>(b.type) << " " << b.value << "\n";
        }
        file.close();
    }
}

void loadProject(std::vector<Block>& script, const std::string & path) {
    script.clear();
    std::ifstream file(path);
    if (file.is_open()) {
        int t;
        float v;
        while (file >> t >> v) {
            script.push_back({static_cast<BlockType>(t), v});
        }
        file.close();
    }
}