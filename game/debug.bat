@echo off
cd /D "%~dp0"

pushd bin
start devenv /debugexe .\game.exe
popd
