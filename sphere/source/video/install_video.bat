@echo off
cd ..\..\..
if not exist system\video mkdir system\video
copy source\video\%1\Release\%1.dll system\video
echo Installing %1

