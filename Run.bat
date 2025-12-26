@echo off
chcp 65001 > nul

call Build.bat

echo Starting editor in 5 seconds...
timeout /t 5 > nul
cd .\build\exe\
start "" GenHotkeys.exe
