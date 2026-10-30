with open(r"CustomEngine\native\GameEngine.cpp", "r", encoding="utf-8") as f:
    text = f.read()

text = text.replace("tex_palette.png", "Palette.png")
text = text.replace("T_Caustic_Base_02.png", "Caustic 1.png")
text = text.replace("T_Foam_01.png", "Foam 1.png")
text = text.replace("T_Foam_02.png", "Foam2.png")

target = 'Texture2D texFoam2           = LoadTexture(ResolveAsset("extracted_assets/textures/Foam2.png").c_str());'
replacement = target + """

    SetTextureFilter(texCaustic, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texFoam1,   TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texFoam2,   TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(texCaustic, TEXTURE_WRAP_REPEAT);
    SetTextureWrap(texFoam1,   TEXTURE_WRAP_REPEAT);
    SetTextureWrap(texFoam2,   TEXTURE_WRAP_REPEAT);

    int causticLoc = GetShaderLocation(shaderWater, "causticTex");
    int foam1Loc   = GetShaderLocation(shaderWater, "foam1Tex");
    int foam2Loc   = GetShaderLocation(shaderWater, "foam2Tex");

    shaderWater.locs[SHADER_LOC_MAP_DIFFUSE]  = causticLoc;
    shaderWater.locs[SHADER_LOC_MAP_NORMAL]   = foam1Loc;
    shaderWater.locs[SHADER_LOC_MAP_SPECULAR] = foam2Loc;"""

text = text.replace(target, replacement)

with open(r"CustomEngine\native\GameEngine.cpp", "w", encoding="utf-8") as f:
    f.write(text)

print("Textures fixed successfully")

