@echo off
D:\MinGW\mingw64\bin\g++.exe -g .\cyi\main.cpp .\CodeAnalysis\Syntax\*.cpp .\CodeAnalysis\Binder\*.cpp .\CodeAnalysis\Text\*.cpp .\CodeAnalysis\Symbol\*.cpp .\CodeAnalysis\Lowerer\*.cpp .\Utils\*.cpp -I include -o compiler
if %errorlevel% equ 0 (
    echo Build succeeded. Running the program...
    compiler %*
) else (
    echo Build failed.
)