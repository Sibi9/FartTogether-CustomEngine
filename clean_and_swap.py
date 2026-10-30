import re

with open('CustomEngine/native/GameEngine.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# Normalize multiple blank lines to at most 1 blank line
lines = text.splitlines()
cleaned_lines = []
last_empty = False
for line in lines:
    is_empty = (line.strip() == '')
    if is_empty:
        if not last_empty:
            cleaned_lines.append('')
            last_empty = True
    else:
        cleaned_lines.append(line)
        last_empty = False

clean_text = '\n'.join(cleaned_lines)

# Target nametag section
idx_start = clean_text.find('// Authentic Player Overhead Name Tags')
idx_end = clean_text.find('// 2D HUD (Authentic Typography')

assert idx_start != -1 and idx_end != -1, "Sections not found"

replacement = """// Authentic Player Overhead Name Tags (Projected 3D Billboards matching video)
        boatRotM = MatrixRotateY(raft.rotationY);

        // Player 1: "tisk" (Green Hippo)
        Vector3 p1HeadLocal = { sinf(p2RimAngle) * 1.05f, 1.45f, cosf(p2RimAngle) * 1.05f };
        Vector3 p1WorldPos = Vector3Add(raft.position, Vector3Transform(p1HeadLocal, boatRotM));
        Vector2 p1Screen = GetWorldToScreen(p1WorldPos, camera);
        if (p1Screen.x > 30 && p1Screen.x < GetScreenWidth() - 30 && p1Screen.y > 20 && p1Screen.y < GetScreenHeight() - 20) {
            int w = MeasureText("tisk", 20);
            DrawRectangleRounded(Rectangle{ p1Screen.x - (float)w/2.0f - 8, p1Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, Color{ 10, 15, 25, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ p1Screen.x - (float)w/2.0f - 8, p1Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, 1.5f, Color{ 85, 232, 25, 180 });
            DrawText("tisk", (int)p1Screen.x - w / 2, (int)p1Screen.y + 2, 20, RAYWHITE);
        }

        // Player 2: "Echos" (Brown Hippo)
        Vector3 p2HeadLocal = { sinf(p1RimAngle) * 1.05f, 1.45f, cosf(p1RimAngle) * 1.05f };
        Vector3 p2WorldPos = Vector3Add(raft.position, Vector3Transform(p2HeadLocal, boatRotM));
        Vector2 p2Screen = GetWorldToScreen(p2WorldPos, camera);
        if (p2Screen.x > 30 && p2Screen.x < GetScreenWidth() - 30 && p2Screen.y > 20 && p2Screen.y < GetScreenHeight() - 20) {
            int w = MeasureText("Echos", 20);
            DrawRectangleRounded(Rectangle{ p2Screen.x - (float)w/2.0f - 8, p2Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, Color{ 10, 15, 25, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ p2Screen.x - (float)w/2.0f - 8, p2Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, 1.5f, Color{ 255, 190, 40, 180 });
            DrawText("Echos", (int)p2Screen.x - w / 2, (int)p2Screen.y + 2, 20, RAYWHITE);
        }

        """

new_text = clean_text[:idx_start] + replacement + clean_text[idx_end:]

with open('CustomEngine/native/GameEngine.cpp', 'wb') as f:
    f.write(new_text.replace('\n', '\r\n').encode('utf-8'))

print("Updated GameEngine.cpp cleanly!")
