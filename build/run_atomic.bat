@echo off

setlocal ENABLEDELAYEDEXPANSION

set base_path=%~dp0..
set build_path=%base_path%\build
set bin_path=%base_path%\bin\ak_atomic

REM - Build inputs and validation
for %%a in (%*) do set "%%a=1" 
if not "%release%"=="1" set debug=1

if not "%intel%"=="1" (
	if not "%clang%"=="1" set msvc=1
)

REM - Get the proper binary directory based on build inputs (goes compiler -> platform -> mode -> executable)
if "%msvc%"=="1" set bin_path=%bin_path%\msvc
if "%clang%"=="1" set bin_path=%bin_path%\clang
if "%intel%"=="1" set bin_path=%bin_path%\intel

if "%x64%"=="1" set bin_path=%bin_path%\x64
if "%x86%"=="1" set bin_path=%bin_path%\x86

if "%debug%"=="1" set bin_path=%bin_path%\debug
if "%release%"=="1" set bin_path=%bin_path%\release

echo %bin_path%

for /r "%bin_path%" %%a in (*unit_test.exe) do (
	echo Running %%a 
	%%~fa
)