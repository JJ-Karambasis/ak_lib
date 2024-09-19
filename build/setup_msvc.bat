@echo off

set target=%1

SET "LIB="

set vc_path=
if %target%==11 SET vc_path=C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\vcvarsall.bat" %2))

if defined LIB set msvc_version=11
if defined LIB goto end

set vc_path=
if %target%==12 SET vc_path=C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\vcvarsall.bat" %2))

if defined LIB set msvc_version=12
if defined LIB goto end

set vc_path=
if %target%==14 SET vc_path=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\vcvarsall.bat" %2))

if defined LIB set msvc_version=14
if defined LIB goto end

set vc_path=
if %target%==2017 SET vc_path=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\VC\Auxiliary\Build\vcvarsall.bat" %2))

if defined LIB set msvc_version=2017
if defined LIB goto end

set vc_path=
if %target%==2019 SET vc_path=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\VC\Auxiliary\Build\vcvarsall.bat" %2))

if defined LIB set msvc_version=2019
if defined LIB goto end

set vc_path=
if %target%==2022 SET vc_path=C:\Program Files\Microsoft Visual Studio\2022\Professional
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\VC\Auxiliary\Build\vcvarsall.bat" %2))

if %target%==2022 SET vc_path=C:\Program Files\Microsoft Visual Studio\2022\Community
IF NOT DEFINED LIB (IF EXIST "%vc_path%" (call "%vc_path%\VC\Auxiliary\Build\vcvarsall.bat" %2))

if defined LIB set msvc_version=2022
if defined LIB goto end


:end