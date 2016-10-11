@echo off
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /format:list') do set datetime=%%I
set datetime=%datetime:~0,8%-%datetime:~8,6%

if not exist export mkdir export
pushd export

if not exist %datetime% mkdir %datetime%
pushd %datetime%

popd
popd

xcopy ..\assets export\%datetime%\assets\ /Y /Q
xcopy ..\lua\*.lua export\%datetime%\lua\ /Y /Q
xcopy *.dll export\%datetime%\bin\ /Y /Q
xcopy replay.exe export\%datetime%\bin\ /Y /Q
