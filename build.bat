@echo off
set compiler=cl
setlocal enableextensions enabledelayedexpansion

if not exist bin mkdir bin

echo Using MSVC
set common_c=%CD%\main.c -nologo -D_WIN32_WINNT=0x0501 -TP -EHa- -MP -I..\ -I..\lib
set common_l=/link /OPT:REF user32.lib gdi32.lib winmm.lib main.res /SUBSYSTEM:windows /OUT:Transmog.exe


echo Building resources
rc /nologo /fo bin\main.res main.rc

pushd bin
if "%1"=="release" (
	echo Building release
	cl !common_c! -O2 -MTd -DRELEASE_BUILD /Gy /Oy !common_l!
	echo Stripping
	strip Transmog.exe
) else if "!1"=="released" (
	echo Building release with debug information
	cl !common_c! -O2 -MTd -DRELEASE_BUILD /Gy /Oy /Z7 !common_l!
) else (
	echo Building debug
	cl !common_c! -Od -MDd -Z7 !common_l!
)
popd
