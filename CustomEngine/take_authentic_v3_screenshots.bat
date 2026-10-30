@echo off
cd /d "%~dp0"
if not exist "screenshots" mkdir screenshots

echo [1/6] Capturing pirates butt, eyes, and tree colors...
FartTogetherNative.exe --test-cam 15.55 2.15 92.50 15.55 1.10 88.00 --shot-frames 45 --shot-name screenshots/01_pirates_butt_and_eyes.png

echo [2/6] Capturing canal wall bumpers and OBB path...
FartTogetherNative.exe --test-cam 16.50 4.20 68.00 22.00 0.80 48.00 --shot-frames 30 --shot-name screenshots/02_canal_obb_and_bumpers.png

echo [3/6] Capturing cartoon skull gas cloud explosion...
FartTogetherNative.exe --test-cam 15.55 3.50 96.00 15.55 1.40 88.00 --test-death --shot-frames 35 --shot-name screenshots/03_skull_gas_explosion.png

echo [4/6] Capturing animated obstacles C5 and F11...
FartTogetherNative.exe --test-cam -42.00 13.50 -88.00 -52.00 9.80 -100.00 --shot-frames 40 --shot-name screenshots/04_animated_obstacles_c5_f11.png

echo [5/6] Capturing Godot editor with 3D model preview and scale controls...
FartTogetherNative.exe --test-editor --test-select 0 --test-cam 18.00 6.00 75.00 15.55 1.00 85.00 --shot-frames 30 --shot-name screenshots/05_godot_editor_with_3d_preview.png

echo [6/6] Capturing LAN multiplayer HUD...
FartTogetherNative.exe --test-lan --test-cam 15.55 3.20 96.50 15.55 1.00 84.00 --shot-frames 30 --shot-name screenshots/06_lan_multiplayer_hud.png

echo Copying screenshots to project root...
if not exist "..\screenshots" mkdir "..\screenshots"
copy /y screenshots\*.png "..\screenshots\"

echo All 6 screenshots captured successfully!

