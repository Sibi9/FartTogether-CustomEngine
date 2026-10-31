#include "raylib.h"
#include <stdio.h>
#include <vector>

int main() {
    InitWindow(600, 300, "Test Arabic Render");
    std::vector<int> cp;
    for (int i = 32; i <= 126; i++) cp.push_back(i);
    for (int i = 0xFE70; i <= 0xFEFC; i++) cp.push_back(i);

    Font f = LoadFontEx("CustomEngine/extracted_assets/fonts/ui_font.ttf", 32, cp.data(), (int)cp.size());
    SetTextureFilter(f.texture, TEXTURE_FILTER_BILINEAR);

    BeginDrawing();
        ClearBackground(DARKGRAY);
        // "ابدأ اللعب" shaped:
        const char* txt = "\xEF\xBA\x90\xEF\xBB\x8C\xEF\xBB\x88\xEF\xBB\x87\xEF\xBA\x85 \xEF\xBA\x83\xEF\xBA\x94\xEF\xBA\x91\xEF\xBA\x85";
        DrawTextEx(f, txt, Vector2{ 50, 50 }, 32, 1.0f, WHITE);
        DrawTextEx(f, "PLAY GAME [EN]", Vector2{ 50, 120 }, 32, 1.0f, YELLOW);
    EndDrawing();

    TakeScreenshot("CustomEngine/screenshots/test_ar_render.png");
    UnloadFont(f);
    CloseWindow();
    return 0;
}

