@echo off
cd ..\..
if not exist bin mkdir bin
copy source\editor\output\Release\editor.exe bin
echo Installing editor.exe

