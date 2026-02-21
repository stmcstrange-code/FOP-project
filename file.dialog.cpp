#include <windows.h>
#include <commdlg.h>
#include "file_dialog.h"

char * openSpriteDialog() {
    static char filename[MAX_PATH] = "";

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter =  "Image Files\0*.png;*.jpg*.bmp\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
        return filename;

    return nullptr;
}