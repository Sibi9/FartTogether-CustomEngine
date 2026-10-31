import pathlib
from fontTools.ttLib import TTFont

# 1. Update Localization.h to remove checkmark symbol
loc_path = pathlib.Path("CustomEngine/native/Localization.h")
loc_text = loc_path.read_text(encoding="utf-8")
loc_text = loc_text.replace("✔", "")
loc_path.write_text(loc_text, encoding="utf-8")
print("Cleaned Localization.h")

# 2. Get exact list of codepoints needed
needed_cps = set(range(32, 127))
for c in loc_text:
    needed_cps.add(ord(c))

font = TTFont('CustomEngine/extracted_assets/fonts/ui_font.ttf')
chars = set()
for t in font['cmap'].tables:
    chars.update(t.cmap.keys())

valid_cps = sorted(list(needed_cps.intersection(chars)))
print(f"Total valid codepoints for font: {len(valid_cps)}")

# 3. Write codepoints array to Codepoints.h
cp_code = f"""#pragma once
static const int gArFontCodepoints[{len(valid_cps)}] = {{
    {', '.join(str(c) for c in valid_cps)}
}};
static const int gArFontCodepointCount = {len(valid_cps)};
"""
pathlib.Path("CustomEngine/native/Codepoints.h").write_text(cp_code, encoding="utf-8")

# 4. Update GameEngine.cpp to use Codepoints.h and remove pondRipple cyan box
engine_path = pathlib.Path("CustomEngine/native/GameEngine.cpp")
engine_text = engine_path.read_text(encoding="utf-8")

if '#include "Codepoints.h"' not in engine_text:
    engine_text = engine_text.replace('#include "Localization.h"', '#include "Localization.h"\n#include "Codepoints.h"')

# Remove pondRipple box
old_ripple = """            // Subtle water shimmer on the pond area
            float pondRipple = sinf(runTime * 2.0f) * 0.06f + 0.06f;
            DrawRectangleRounded(Rectangle{ 320.0f * scaleX, 410.0f * scaleY, 490.0f * scaleX, 185.0f * scaleY }, 0.45f, 6,
                                 Color{ 64, 224, 208, (unsigned char)(pondRipple * 255.0f) });"""

if old_ripple in engine_text:
    engine_text = engine_text.replace(old_ripple, "// Pond shimmer handled natively by artwork")
    print("Removed cyan pond ripple overlay")
else:
    # check alternate formatting
    old_ripple_sub = "DrawRectangleRounded(Rectangle{ 320.0f * scaleX, 410.0f * scaleY, 490.0f * scaleX, 185.0f * scaleY }"
    idx = engine_text.find(old_ripple_sub)
    if idx != -1:
        end_stmt = engine_text.find(");", idx)
        engine_text = engine_text[:idx] + "// Pond shimmer handled natively by artwork" + engine_text[end_stmt+2:]
        print("Removed cyan pond ripple overlay (substr)")

# Update font loading to use gArFontCodepoints
old_font_chunk = """    std::vector<int> arCodepoints;
    for (int i = 32; i <= 126; i++) arCodepoints.push_back(i);
    for (int i = 0x0600; i <= 0x06FF; i++) arCodepoints.push_back(i);
    for (int i = 0xFE70; i <= 0xFEFF; i++) arCodepoints.push_back(i);
    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 24, arCodepoints.data(), (int)arCodepoints.size());
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 24, arCodepoints.data(), (int)arCodepoints.size());"""

new_font_chunk = """    Font fontUI     = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font.ttf").c_str(), 26, (int*)gArFontCodepoints, gArFontCodepointCount);
    Font fontUIBold = LoadFontEx(ResolveAsset("extracted_assets/fonts/ui_font_bold.ttf").c_str(), 26, (int*)gArFontCodepoints, gArFontCodepointCount);"""

if old_font_chunk in engine_text:
    engine_text = engine_text.replace(old_font_chunk, new_font_chunk)
    print("Replaced font loading with valid codepoints array")

engine_path.write_text(engine_text, encoding="utf-8")
print("Engine updated!")

