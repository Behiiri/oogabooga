@echo off
cd /D "%~dp0"

if exist bin (
  rmdir /s /q bin
)

mkdir bin
pushd bin

SET USE_JUMBO_BUILD=1

set COMPILE_FILES= ../build.c
set LINK_FILES= build.o 

IF "%USE_JUMBO_BUILD%"=="0" (
    set COMPILE_FILES=%COMPILE_FILES% ../src/entity.c ../src/world.c ../src/monster.c ../src/weapon.c ../src/character.c
REM ../src/render.c ../src/menu.c
    set LINK_FILES=%LINK_FILES% entity.o world.o monster.o weapon.o character.o
REM render.o  menu.o
)

set COMPILER_FLAGS=-g -c -std=c11 -D_CRT_SECURE_NO_WARNINGS -DJUMBO_BUILD=%USE_JUMBO_BUILD% -Wextra -Wno-incompatible-library-redeclaration -Wno-sign-compare -Wno-unused-parameter -Wno-builtin-requires-header -femit-all-decls -fdiagnostics-absolute-paths
set LINKER_FLAGS=-g -o game.exe -O0 -lkernel32 -lgdi32 -luser32 -lruntimeobject -lwinmm -ld3d11 -ldxguid -ld3dcompiler -lshlwapi -lole32 -lavrt -lksuser -ldbghelp 

echo ^>^>^> Compiling...
set START_TIME=%time%
set /A START_MSEC=100*(%START_TIME:~6,2%)+%START_TIME:~9,2%
clang %COMPILE_FILES% %COMPILER_FLAGS%
set END_TIME=%time%
set /A END_MSEC=100*(%END_TIME:~6,2%)+%END_TIME:~9,2%
set /A ELAPSED_MSEC=END_MSEC-START_MSEC, ELAPSED_SEC=ELAPSED_MSEC/100
set /A ELAPSED_MSEC=ELAPSED_MSEC%%100
echo Compile time: %ELAPSED_SEC%.%ELAPSED_MSEC% seconds

echo ^>^>^> Linking...
set START_TIME=%time%
set /A START_MSEC=100*(%START_TIME:~6,2%)+%START_TIME:~9,2%
clang %LINK_FILES% %LINKER_FLAGS%
set END_TIME=%time%
set /A END_MSEC=100*(%END_TIME:~6,2%)+%END_TIME:~9,2%
set /A ELAPSED_MSEC=END_MSEC-START_MSEC, ELAPSED_SEC=ELAPSED_MSEC/100
set /A ELAPSED_MSEC=ELAPSED_MSEC%%100
echo Link time: %ELAPSED_SEC%.%ELAPSED_MSEC% seconds

echo ^>^>^> Running...
.\game.exe

popd
