#include "FileHandler.h"
#include <iostream>

void saveProject(const std::vector<VisualBlock>& workspace, const std::string& path) {
    std::ofstream file(path);
    if (file.is_open()) {
        for (const auto& vb : workspace) {

            file << static_cast<int>(vb.data.type) << " "
                 << vb.data.value << " "
                 << vb.rect.x << " "
                 << vb.rect.y << "\n";
        }
        file.close();
        std::cout << "Project saved successfully to " << path << std::endl;
    }
}

void loadProject(std::vector<VisualBlock>& workspace, const std::string& path) {
    workspace.clear();
    std::ifstream file(path);
    if (file.is_open()) {
        int t, x, y;
        float v;

        while (file >> t >> v >> x >> y) {
            VisualBlock vb;

            vb.data.type = static_cast<BlockType>(t);
            vb.data.value = v;
            vb.data.iterations = 0;


            vb.rect = {x, y, 120, 40};
            vb.isDragging = false;



            workspace.push_back(vb);
        }
        file.close();
        std::cout << "Project loaded successfully from " << path << std::endl;
    }
}