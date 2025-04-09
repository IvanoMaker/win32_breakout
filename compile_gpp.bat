@echo off
g++ main.cpp -o main.exe -lgdi32 -mwindows
if %errorlevel% neq 0 (
    echo Compilation failed.
    pause
    exit /b
)
echo Compilation successful.