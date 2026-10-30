@echo off
title Fart Together - GitHub Project Uploader
chcp 65001 > nul
cd /d "%~dp0"
cls
if exist "C:\Users\ghenemi\AppData\Local\Programs\Python\Python312\python.exe" (
    "C:\Users\ghenemi\AppData\Local\Programs\Python\Python312\python.exe" "%~dp0upload_to_github.py"
) else (
    py -3.12 "%~dp0upload_to_github.py"
)
pause
