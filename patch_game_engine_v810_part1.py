import re, pathlib

engine_path = pathlib.Path("CustomEngine/native/GameEngine.cpp")
text = engine_path.read_text(encoding="utf-8")

print("Original GameEngine.cpp length:", len(text))

# 1. Add #include "Localization.h" and GAME_VERSION_TAG at top
if '#include "Localization.h"' not in text:
    top_anchor = '#include "LanNetwork.h"'
    top_replacement = '''#include "LanNetwork.h"
#include "Localization.h"

// Rule: Game version must be incremented on every update!
static const char* GAME_VERSION_TAG = "v8.1.0";
GameLanguage gCurrentLanguage = LANG_AR; // Default Arabic as requested
static bool gShowInviteModal = false;
'''
    text = text.replace(top_anchor, top_replacement)
    print("Added Localization.h and v8.1.0 tag")

# 2. Add language to GameSettingsConfig
if "int   language     = 0;" not in text:
    old_settings = "    int   deviceMode   = 0;     // 0=Auto-Detect, 1=PC (Keyboard/Mouse), 2=Mobile (Touchscreen)"
    new_settings = "    int   language     = 0;     // 0=Arabic (Default), 1=English\n    int   deviceMode   = 0;     // 0=Auto-Detect, 1=PC (Keyboard/Mouse), 2=Mobile (Touchscreen)"
    text = text.replace(old_settings, new_settings)
    print("Added language to GameSettingsConfig")

# 3. Update Font Loading with Arabic Codepoints
old_font_load = '''    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 18, 0, 0);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 18, 0, 0);'''

new_font_load = '''    std::vector<int> arCodepoints;
    for (int i = 32; i <= 126; i++) arCodepoints.push_back(i);
    for (int i = 0x0600; i <= 0x06FF; i++) arCodepoints.push_back(i);
    for (int i = 0xFE70; i <= 0xFEFF; i++) arCodepoints.push_back(i);
    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 24, arCodepoints.data(), (int)arCodepoints.size());
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 24, arCodepoints.data(), (int)arCodepoints.size());'''

if old_font_load in text:
    text = text.replace(old_font_load, new_font_load)
    print("Updated font loading with full Arabic and English codepoints")

engine_path.write_text(text, encoding="utf-8")
print("First pass applied successfully!")
