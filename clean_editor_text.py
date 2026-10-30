with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    c = f.read()

# Replace unicode symbols with clean ASCII in editor GUI
c = c.replace('"↺ -15°"', '"Rot -15"')
c = c.replace('"↻ +15°"', '"Rot +15"')
c = c.replace('"📋 DUPLICATE"', '"DUPLICATE"')
c = c.replace('"🗑 DELETE"', '"DELETE"')
c = c.replace('"💾 SAVE LEVEL TO JSON [Ctrl+S]"', '"SAVE LEVEL TO JSON [Ctrl+S]"')
c = c.replace('"• Left Click', '"- Left Click')
c = c.replace('"• J/L:', '"- J/L:')
c = c.replace('"• R/T:', '"- R/T:')
c = c.replace('"• Right Click', '"- Right Click')
c = c.replace('"• F1 / E:', '"- F1 / E:')

# Also set editor camera focus when testEditorMode is active
old_ed_pos = 'Vector3 editorCamPos = testEditorMode ? Vector3{ 17.5f, 3.8f, 72.0f } : Vector3{ 15.55f, 6.0f, 75.0f };'
new_ed_pos = 'Vector3 editorCamPos = testEditorMode ? Vector3{ 17.2f, 2.8f, 70.8f } : Vector3{ 15.55f, 6.0f, 75.0f };'
c = c.replace(old_ed_pos, new_ed_pos)

with open("CustomEngine/native/GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(c)

print("Replaced unicode with ASCII for clean text rendering!")

