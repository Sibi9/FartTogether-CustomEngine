import pathlib

# 1. Update Localization.h with STR_DEVICE_PROFILE alias
loc_p = pathlib.Path("CustomEngine/native/Localization.h")
loc_t = loc_p.read_text(encoding="utf-8")
if "STR_DEVICE_PROFILE" not in loc_t:
    loc_t += "\ninline const LocalizedString& STR_DEVICE_PROFILE = STR_DEVICE_MODE;\n"
loc_p.write_text(loc_t, encoding="utf-8")
print("Added STR_DEVICE_PROFILE to Localization.h")

# 2. Update GameEngine.cpp
engine_p = pathlib.Path("CustomEngine/native/GameEngine.cpp")
engine_t = engine_p.read_text(encoding="utf-8")

# Add texture declarations for texIconTikGreen, texMarkerPlayer, texKeySpace, texIconKEnter
load_target = 'Texture2D texMarkerYou       = LoadTexture(ResolveAsset("extracted_assets/textures/Marker_BlueYou.png").c_str());'
load_replacement = """Texture2D texMarkerYou       = LoadTexture(ResolveAsset("extracted_assets/textures/Marker_BlueYou.png").c_str());
    Texture2D texIconTikGreen   = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_TikGreen.png").c_str());
    Texture2D texMarkerPlayer   = LoadTexture(ResolveAsset("extracted_assets/textures/Marker_BluePlayer.png").c_str());
    Texture2D texKeySpace       = LoadTexture(ResolveAsset("extracted_assets/textures/keyboard_space.png").c_str());
    Texture2D texIconKEnter     = LoadTexture(ResolveAsset("extracted_assets/textures/Icon_KEnter.png").c_str());
    SetTextureFilter(texIconTikGreen, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texMarkerPlayer, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texKeySpace, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(texIconKEnter, TEXTURE_FILTER_BILINEAR);"""

if load_target in engine_t and 'texIconTikGreen' not in engine_t[:engine_t.find(load_target)]:
    engine_t = engine_t.replace(load_target, load_replacement)
    print("Added texture loading for lobby icons!")

# Fix texMenuLogo -> texMainLogo
engine_t = engine_t.replace("texMenuLogo", "texMainLogo")

# Fix ResetBoatToStart() -> inline reset
boat_reset_target = "ResetBoatToStart();"
boat_reset_rep = """raft.position = Vector3{ 15.55f, 0.58f, 90.80f };
                        raft.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
                        raft.rotationY = 3.14159f;
                        raft.isDestroyed = false;
                        raft.currentCP = 0;
                        gasP1.gasEnergy = GAS_MAX;
                        gasP1.gasDepleted = false;"""
engine_t = engine_t.replace(boat_reset_target, boat_reset_rep)

# Fix gLanRole -> LanGetRole()
engine_t = engine_t.replace("gLanRole == LAN_HOST", "LanGetRole() == LAN_HOST")

# Unload textures
unload_target = "UnloadTexture(texMarkerYou);"
unload_rep = """UnloadTexture(texMarkerYou);
    UnloadTexture(texIconTikGreen);
    UnloadTexture(texMarkerPlayer);
    UnloadTexture(texKeySpace);
    UnloadTexture(texIconKEnter);"""
engine_t = engine_t.replace(unload_target, unload_rep)

engine_p.write_text(engine_t, encoding="utf-8")
print("GameEngine.cpp compilation fixes applied!")

