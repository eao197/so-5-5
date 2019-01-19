@echo off

:loop

if "%1" == "" goto :finalize
if /i "%1" == "msvc14" goto :msvc14
if /i "%1" == "msvc15" goto :msvc15
if /i "%1" == "x86" goto :x86
if /i "%1" == "i386" goto :x86
if /i "%1" == "amd64" goto :amd64
if /i "%1" == "x86_64" goto :amd64
if /i "%1" == "x64" goto :amd64

echo Invalid argument: '%1'
exit -1

:msvc14
set TOOLCHAIN=msvc14
set VCVARS_PATH="C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/vcvarsall.bat"
shift
goto :loop

:msvc15
set TOOLCHAIN=msvc15
set VCVARS_PATH="C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Auxiliary/Build/vcvarsall.bat"
shift
goto :loop

:x86
set TARGET_CPU=x86
shift
goto :loop

:amd64
set TARGET_CPU=amd64
shift
goto :loop

:finalize

if "%TOOLCHAIN%" == "" goto :msvc14
if "%TARGET_CPU%" == "" goto :amd64
if "%CONFIGURATION%" == "" (set CONFIGURATION=Release)

echo VCVARS_PATH %VCVARS_PATH%
call %VCVARS_PATH% %TARGET_CPU% || exit /b
