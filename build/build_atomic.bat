@echo off
setlocal ENABLEDELAYEDEXPANSION

REM - Common global variables
set base_path=%~dp0..
set build_path=%base_path%\build
set bin_path=%base_path%\bin\ak_atomic
set test_path=%base_path%\tests\ak_atomic

REM - Build inputs and validation
for %%a in (%*) do set "%%a=1" 
if not "%release%"=="1" set debug=1
if not "%env32%"=="1" set env64=1

set target_version=0
if "%msvc%"=="1" (
	if "%v_2022%"=="1" set target_version=2022
	if "%v_2019%"=="1" set target_version=2019
	if "%v_2017%"=="1" set target_version=2017
	if "%v_14%"=="1"   set target_version=14
	if "%v_12%"=="1"   set target_version=12
	if "%v_11%"=="1"   set target_version=11

	if "%env32%"=="1" call %build_path%\setup_msvc.bat !target_version! x86 || exit /b 1
    if "%env64%"=="1" call %build_path%\setup_msvc.bat !target_version! x64 || exit /b 1
)

REM - Build information
if "%msvc%"=="1" set compiler=MSVC %msvc_version%
if "%env32%"=="1" set bitness=32
if "%env64%"=="1" set bitness=64

if "%debug%"=="1" set build_mode=Debug
if "%release%"=="1" set build_mode=Release


REM - Get the proper binary directory based on build inputs (goes compiler -> platform -> mode -> executable)
if "%msvc%"=="1" set bin_path=%bin_path%\msvc

if "%env32%"=="1" set bin_path=%bin_path%\env32
if "%env64%"=="1" set bin_path=%bin_path%\env64

if "%debug%"=="1" set bin_path=%bin_path%\debug
if "%release%"=="1" set bin_path=%bin_path%\release

if not exist %bin_path% mkdir %bin_path%

REM - MSVC flags
set msvc_warnings=/WX /Wall
set msvc_flags=/nologo /FC /Z7 -I%base_path% %msvc_warnings%

set msvc_debug_flags=/Od %msvc_flags%
set msvc_release_flags=/O2 %msvc_flags%
set msvc_link=/link /incremental:no
set msvc_out=/out:
set msvc_compile=call cl

REM - Unified compile flags
if "%msvc%"=="1" set compile_debug_flags=%msvc_debug_flags%
if "%msvc%"=="1" set compile_release_flags=%msvc_release_flags%
if "%msvc%"=="1" set compile_link=%msvc_link%
if "%msvc%"=="1" set compile_out=%msvc_out%
if "%msvc%"=="1" set compile=%msvc_compile%

REM - Choose between release and debug builds
if "%debug%"=="1" set compile_flags=%compile_debug_flags%
if "%release%"=="1" set compile_flags=%compile_release_flags%

set compile_tests=
set compile_output=

REM - Get all the configuration tests based on compiler and platform
if "%msvc%"=="1" (
	set compile_tests[0]=/Za /Tc %test_path%\ak_atomic_compile_test.c
	set compile_output[0]=ak_atomic_ansii_compile_test.exe

	if %msvc_version% lss 14 (
		set compile_tests[1]=%test_path%\ak_atomic_compile_test.cpp
		set compile_output[1]=ak_atomic_cpp_compile_test.exe
		goto build_step
	)

	set compile_tests[1]=/std:c++14 %test_path%\ak_atomic_compile_test.cpp
	set compile_output[1]=ak_atomic_cpp14_compile_test.exe

	if %msvc_version% gtr 14 (
		set compile_tests[2]=/std:c++17 %test_path%\ak_atomic_compile_test.cpp
		set compile_output[2]=ak_atomic_cpp17_compile_test.exe

		if %msvc_version% geq 2019 (
			set compile_tests[3]=/std:c11 /Tc %test_path%\ak_atomic_compile_test.c
			set compile_output[3]=ak_atomic_c11_compile_test.exe

			set compile_tests[4]=/std:c17 /Tc %test_path%\ak_atomic_compile_test.c
			set compile_output[4]=ak_atomic_c17_compile_test.exe

			set compile_tests[5]=/std:c++20 %test_path%\ak_atomic_compile_test.cpp
			set compile_output[5]=ak_atomic_cpp20_compile_test.exe
		)
	)
)

:build_step

REM - Build all the tests

echo Compiler: %compiler% %bitness%-bit
echo Mode: %build_mode%
echo Flags: %compile_flags%
echo Output Path: %bin_path%
echo:

set build_failed=0

pushd %bin_path%	
	del *.* /F /Q

	set compile_index=0
	echo Building Atomic Tests
	:compile_loop
	if defined compile_tests[%compile_index%] (
		%compile% %compile_flags% %%compile_tests[%compile_index%]%% %compile_link% %compile_out%%%compile_output[%compile_index%]%% || set build_failed=1
		set /a compile_index=%compile_index%+1
		GOTO :compile_loop
	)

	del *.obj /F /Q
popd

if %build_failed%==1 exit /b 1