with open('CustomEngine/native/GameEngine.cpp', 'rb') as f:
    raw = f.read()

# Decode and normalize all line breaks to \n
text = raw.decode('utf-8', errors='ignore')
text = text.replace('\r\n', '\n').replace('\r', '\n')

# Collapse excessive double blank lines created by bad CR replacements if any
# But let's be precise:
text = text.replace('Matrix boatRotM = MatrixRotateY(raft.rotationY);', 'boatRotM = MatrixRotateY(raft.rotationY);')
text = text.replace('DrawRectangleRoundedLines(Rectangle{ p1Screen.x', 'DrawRectangleRoundedLinesEx(Rectangle{ p1Screen.x')
text = text.replace('DrawRectangleRoundedLines(Rectangle{ p2Screen.x', 'DrawRectangleRoundedLinesEx(Rectangle{ p2Screen.x')

with open('CustomEngine/native/GameEngine.cpp', 'wb') as f:
    f.write(text.replace('\n', '\r\n').encode('utf-8'))

print("Cleaned line endings and fixed declarations in GameEngine.cpp")
