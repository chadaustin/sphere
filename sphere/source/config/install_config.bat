@echo off
cd ..\..
if not exist bin mkdir bin
copy source\config\Release\config.exe bin
echo Installing config.exe

