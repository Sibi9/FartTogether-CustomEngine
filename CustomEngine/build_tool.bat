@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

if exist "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" > nul
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" > nul
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%i in (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
        if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" call "%%i\VC\Auxiliary\Build\vcvars64.bat" > nul
    )
)

set "INC_DIR=%~dp0vendor\raylib-6.2_win64_msvc16\include"
set "LIB_DIR=%~dp0vendor\raylib-6.2_win64_msvc16\lib"
set "NATIVE_DIR=%~dp0native"

cl.exe /nologo /std:c++17 /utf-8 /EHsc /O2 /W3 /MD /I"%INC_DIR%" /I"%NATIVE_DIR%" "%NATIVE_DIR%\GameEngine.cpp" "%NATIVE_DIR%\LanNetwork.cpp" /Fe"%~dp0FartTogetherNative.exe" /link /LIBPATH:"%LIB_DIR%" raylib.lib gdi32.lib user32.lib winmm.lib shell32.lib opengl32.lib ws2_32.lib /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup
