with open('CustomEngine/native/GameEngine.cpp', 'r', encoding='utf-8') as f:
    c = f.read()

# Normalize
c_norm = c.replace('\r\n', '\n').replace('\r', '\n')

old_block = """        // Authentic Player Overhead Name Tags (Projected 3D Billboards matching video)
        boatRotM = MatrixRotateY(raft.rotationY);
        Vector3 p1HeadLocal = { sinf(p1RimAngle) * 1.05f, 1.45f, cosf(p1RimAngle) * 1.05f };
        Vector3 p1WorldPos = Vector3Add(raft.position, Vector3Transform(p1HeadLocal, boatRotM));
        Vector2 p1Screen = GetWorldToScreen(p1WorldPos, camera);
        if (p1Screen.x > 30 && p1Screen.x < GetScreenWidth() - 30 && p1Screen.y > 20 && p1Screen.y < GetScreenHeight() - 20) {
            int w = MeasureText("tisk", 20);
            DrawRectangleRounded(Rectangle{ p1Screen.x - (float)w/2.0f - 8, p1Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, Color{ 10, 15, 25, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ p1Screen.x - (float)w/2.0f - 8, p1Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, 1.5f, Color{ 85, 232, 25, 180 });
            DrawText("tisk", (int)p1Screen.x - w / 2, (int)p1Screen.y + 2, 20, RAYWHITE);
        }

        Vector3 p2HeadLocal = { sinf(p2RimAngle) * 1.05f, 1.45f, cosf(p2RimAngle) * 1.05f };
        Vector3 p2WorldPos = Vector3Add(raft.position, Vector3Transform(p2HeadLocal, boatRotM));
        Vector2 p2Screen = GetWorldToScreen(p2WorldPos, camera);
        if (p2Screen.x > 30 && p2Screen.x < GetScreenWidth() - 30 && p2Screen.y > 20 && p2Screen.y < GetScreenHeight() - 20) {
            int w = MeasureText("Echos", 20);
            DrawRectangleRounded(Rectangle{ p2Screen.x - (float)w/2.0f - 8, p2Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, Color{ 10, 15, 25, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ p2Screen.x - (float)w/2.0f - 8, p2Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, 1.5f, Color{ 255, 190, 40, 180 });
            DrawText("Echos", (int)p2Screen.x - w / 2, (int)p2Screen.y + 2, 20, RAYWHITE);
        }"""

new_block = """        // Authentic Player Overhead Name Tags (Projected 3D Billboards matching video)
        boatRotM = MatrixRotateY(raft.rotationY);
        // Green Hippo (Player 1: "tisk")
        Vector3 p1HeadLocal = { sinf(p2RimAngle) * 1.05f, 1.45f, cosf(p2RimAngle) * 1.05f };
        Vector3 p1WorldPos = Vector3Add(raft.position, Vector3Transform(p1HeadLocal, boatRotM));
        Vector2 p1Screen = GetWorldToScreen(p1WorldPos, camera);
        if (p1Screen.x > 30 && p1Screen.x < GetScreenWidth() - 30 && p1Screen.y > 20 && p1Screen.y < GetScreenHeight() - 20) {
            int w = MeasureText("tisk", 20);
            DrawRectangleRounded(Rectangle{ p1Screen.x - (float)w/2.0f - 8, p1Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, Color{ 10, 15, 25, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ p1Screen.x - (float)w/2.0f - 8, p1Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, 1.5f, Color{ 85, 232, 25, 180 });
            DrawText("tisk", (int)p1Screen.x - w / 2, (int)p1Screen.y + 2, 20, RAYWHITE);
        }

        // Brown Hippo (Player 2 / AI Co-pilot: "Echos")
        Vector3 p2HeadLocal = { sinf(p1RimAngle) * 1.05f, 1.45f, cosf(p1RimAngle) * 1.05f };
        Vector3 p2WorldPos = Vector3Add(raft.position, Vector3Transform(p2HeadLocal, boatRotM));
        Vector2 p2Screen = GetWorldToScreen(p2WorldPos, camera);
        if (p2Screen.x > 30 && p2Screen.x < GetScreenWidth() - 30 && p2Screen.y > 20 && p2Screen.y < GetScreenHeight() - 20) {
            int w = MeasureText("Echos", 20);
            DrawRectangleRounded(Rectangle{ p2Screen.x - (float)w/2.0f - 8, p2Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, Color{ 10, 15, 25, 210 });
            DrawRectangleRoundedLinesEx(Rectangle{ p2Screen.x - (float)w/2.0f - 8, p2Screen.y - 2, (float)w + 16, 24 }, 0.4f, 4, 1.5f, Color{ 255, 190, 40, 180 });
            DrawText("Echos", (int)p2Screen.x - w / 2, (int)p2Screen.y + 2, 20, RAYWHITE);
        }"""

assert old_block in c_norm, "old_block not found"
c_norm = c_norm.replace(old_block, new_block)

with open('CustomEngine/native/GameEngine.cpp', 'wb') as f:
    f.write(c_norm.replace('\n', '\r\n').encode('utf-8'))

print("Swapped nametags to match hippo models perfectly!")
