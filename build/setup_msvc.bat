@echo off


SET "LIB="

SET vc_path=C:\Program Files\Microsoft Visual Studio\2022\Professional
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\VC\Auxiliary\Build\vcvarsall.bat" %1))

SET vc_path=C:\Program Files\Microsoft Visual Studio\2022\Community
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\VC\Auxiliary\Build\vcvarsall.bat" %1))

if defined LIB set msvc_version=2022
if defined LIB goto end

if not defined LIB exit /b 1

:end