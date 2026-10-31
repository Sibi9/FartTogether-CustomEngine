@echo off
for /f "usebackq tokens=*" %%i in (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
    if exist "%%i\VC\Auxiliary\Build\vcvars64.bat" call "%%i\VC\Auxiliary\Build\vcvars64.bat" > nul
)
cl.exe /nologo /O2 /MD /utf-8 /I CustomEngine\vendor\raylib-6.2_win64_msvc16\include CustomEngine\test_ar_render.cpp /Fe:CustomEngine\test_ar_render.exe /link /LIBPATH:CustomEngine\vendor\raylib-6.2_win64_msvc16\lib raylib.lib winmm.lib gdi32.lib user32.lib shell32.lib opengl32.lib
CustomEngine\test_ar_render.exe
