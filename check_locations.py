with open("CustomEngine/native/GameEngine.cpp", "r", encoding="utf-8") as f:
    text = f.read()

# Let's inspect where uRednessLoc is set
print("uRednessLoc occurrences:")
for i, line in enumerate(text.splitlines()):
    if "uRednessLoc" in line:
        print(f"  Line {i+1}: {line.strip()}")

