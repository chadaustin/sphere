@echo off
cd ..\..
if not exist bin mkdir bin
copy source\engine\output\Release\engine.exe bin
echo Installing engine.exe

