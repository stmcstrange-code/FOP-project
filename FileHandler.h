

#ifndef FOP_PROJECT_FILEHANDLER_H
#define FOP_PROJECT_FILEHANDLER_H

#include <vector>
#include <string>
#include <fstream>
#include "Block.h"

void saveProject(const std::vector<Block>& script, const std::string & path);
void loadProject(std::vector<Block>& script, const std::string & path);


#endif //FOP_PROJECT_FILEHANDLER_H