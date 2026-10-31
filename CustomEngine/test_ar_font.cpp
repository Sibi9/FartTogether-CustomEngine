#include "raylib.h"
#include <stdio.h>
#include <vector>

int main() {
    InitWindow(400, 200, "Test Arabic Font");
    std::vector<int> cp;
    for (int i = 32; i <= 126; i++) cp.push_back(i);
    for (int i = 0x0600; i <= 0x06FF; i++) cp.push_back(i);
    for (int i = 0xFE70; i <= 0xFEFF; i++) cp.push_back(i);
    Font f = LoadFontEx("CustomEngine/extracted_assets/fonts/ui_font.ttf", 32, cp.data(), (int)cp.size());
    printf("Font glyphCount: %d, texture.id: %u\n", f.glyphCount, f.texture.id);
    UnloadFont(f);
    CloseWindow();
    return 0;
}
