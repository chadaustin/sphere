@echo off
cd ..\..\..
if not exist system\video mkdir system\video
copy source\video\%1\output\release\%1.dll system\video
echo Installing %1

