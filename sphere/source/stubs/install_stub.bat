@echo off
cd ..\..\..
copy source\stubs\%1\Release\stub_%1.exe .\%1.exe
echo Installing stub %1

