@echo off
cd /d "%~dp0"
if not exist "screenshots" mkdir screenshots

echo Taking screenshot 1/6: Start basin and hippos...
FartTogetherNative.exe --test-cam 15.55 3.20 96.50 15.55 1.00 84.00 --shot-frames 30 --shot-name screenshots/01_start_basin_and_hippos.png

echo Taking screenshot 2/6: First canal bumpers...
FartTogetherNative.exe --test-cam 16.50 4.50 68.00 22.00 0.80 48.00 --shot-frames 30 --shot-name screenshots/02_first_canal_bumpers.png

echo Taking screenshot 3/6: Canyon turn and croc...
FartTogetherNative.exe --test-cam 48.00 4.50 32.00 38.00 0.80 15.00 --shot-frames 30 --shot-name screenshots/03_canyon_turn_and_croc.png

echo Taking screenshot 4/6: Spiked drums F11...
FartTogetherNative.exe --test-cam -42.00 13.50 -88.00 -52.00 9.80 -100.00 --shot-frames 30 --shot-name screenshots/04_spiked_drums_f11.png

echo Taking screenshot 5/6: Checkpoints on water...
FartTogetherNative.exe --test-cam -115.00 13.00 -85.00 -135.00 9.80 -80.00 --shot-frames 30 --shot-name screenshots/05_checkpoints_on_water.png

echo Taking screenshot 6/6: Waterfall shark and finish...
FartTogetherNative.exe --test-cam -415.00 12.00 -103.10 -440.00 8.00 -103.12 --shot-frames 30 --shot-name screenshots/06_waterfall_shark_and_finish.png

echo Copying screenshots to project root...
if not exist "..\screenshots" mkdir "..\screenshots"
copy /y screenshots\*.png "..\screenshots\"

echo All 6 screenshots successfully captured!

