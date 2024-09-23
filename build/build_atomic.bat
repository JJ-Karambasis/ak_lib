@echo off

setlocal ENABLEDELAYEDEXPANSION
REM - Common global variables

set env32=0
set env64=0

set base_path=%~dp0..
set build_path=%base_path%\build
set bin_path=%base_path%\bin\ak_atomic
set test_path=%base_path%\tests\ak_atomic

REM - Build inputs and validation
for %%a in (%*) do set "%%a=1" 
if not "%release%"=="1" set debug=1
if not "%env32%"=="1" set env64=1

if not "%intel%"=="1" (
	if not "%clang%"=="1" set msvc=1
)

set target_version=0

set msvc_version=0
if "%msvc%"=="1" (
	if "%v_2022%"=="1" set target_version=2022
	if "%v_2019%"=="1" set target_version=2019
	if "%v_2017%"=="1" set target_version=2017
	if "%v_14%"=="1"   set target_version=14
	if "%v_12%"=="1"   set target_version=12
	if "%v_11%"=="1"   set target_version=11
	if "%v_11%"=="1"   set /a env32=1 & set /a env64=0

	if !env32! == 1 call %build_path%\setup_msvc.bat !target_version! x86 >nul || exit /b 1 
    if !env64! == 1 call %build_path%\setup_msvc.bat !target_version! x64 >nul || exit /b 1
)

if "%intel%"=="1" (
	if "%env32%"=="1" call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" ia32 >nul || exit /b 1 
	if "%env64%"=="1" call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" intel64 >nul || exit /b 1 
)

REM - Build information
if "%msvc%"=="1" set compiler=MSVC %msvc_version%
if "%clang%"=="1" set compiler=Clang
if "%intel%"=="1" set compiler=Intel

if %env32%==1 set bitness=32
if %env64%==1 set bitness=64

if "%debug%"=="1" set build_mode=Debug
if "%release%"=="1" set build_mode=Release

REM - Get the proper binary directory based on build inputs (goes compiler -> platform -> mode -> executable)
if "%msvc%"=="1" set bin_path=%bin_path%\msvc
if "%clang%"=="1" set bin_path=%bin_path%\clang
if "%intel%"=="1" set bin_path=%bin_path%\intel

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
set msvc_link=/link /incremental:no /nodefaultlib kernel32.lib
set msvc_out=/out:
set msvc_compile=call cl

REM - Clang flags
set clang_warnings=-Werror -Wall
set clang_flags=-g -gcodeview -fdiagnostics-absolute-paths -I%base_path% %clang_warnings%

if "%clang%"=="1" (
	if "%env32%"=="1" set clang_flags=-m32 %clang_flags%
)

set clang_debug_flags=-O0 %clang_flags%
set clang_release_flags=-O2 %clang_flags%
set clang_link=-Xlinker -nodefaultlib -lkernel32.lib
set clang_out=-o
set clang_compile=call clang

REM - Intel flags
set intel_warnings=-Werror -Wall
set intel_flags=-g -gcodeview -fdiagnostics-absolute-paths -I%base_path% %intel_warnings%

if "%intel%"=="1" (
	if "%env32%"=="1" set intel_flags=-m32 %intel_flags%
)

set intel_debug_flags=-Od %intel_flags%
set intel_release_flags=-O2 %intel_flags%
set intel_link=-link -incremental:no -nodefaultlib -nologo kernel32.lib
set intel_out=-out:
set intel_compile=call icx

REM - Unified compile flags
if "%msvc%"=="1" set compile_debug_flags=%msvc_debug_flags%
if "%msvc%"=="1" set compile_release_flags=%msvc_release_flags%
if "%msvc%"=="1" set compile_link=%msvc_link%
if "%msvc%"=="1" set compile_out=%msvc_out%
if "%msvc%"=="1" set compile=%msvc_compile%

if "%clang%"=="1" set compile_debug_flags=%clang_debug_flags%
if "%clang%"=="1" set compile_release_flags=%clang_release_flags%
if "%clang%"=="1" set compile_link=%clang_link%
if "%clang%"=="1" set compile_out=%clang_out%
if "%clang%"=="1" set compile=%clang_compile%

if "%intel%"=="1" set compile_debug_flags=%intel_debug_flags%
if "%intel%"=="1" set compile_release_flags=%intel_release_flags%
if "%intel%"=="1" set compile_link=%intel_link%
if "%intel%"=="1" set compile_out=%intel_out%
if "%intel%"=="1" set compile=%intel_compile%

REM - Choose between release and debug builds
if "%debug%"=="1" set compile_flags=%compile_debug_flags%
if "%release%"=="1" set compile_flags=%compile_release_flags%

REM - Get all the configuration tests based on compiler and platform
set compile_tests=
set compile_output=

if "%msvc%"=="1" (
	set compile_tests[0]=/Tc %test_path%\ak_atomic_compile_test.c
	set compile_output[0]=ak_atomic_ansii_compile_test.exe

	if %msvc_version% lss 14 (
		set compile_tests[1]=-wd4514 %test_path%\ak_atomic_compile_test.cpp
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

if "%clang%"=="1" (
	set compile_tests[0]=-std=gnu89 %test_path%\ak_atomic_compile_test.c
	set compile_output[0]=ak_atomic_c89_compile_test.exe

	set compile_tests[1]=-std=c99 %test_path%\ak_atomic_compile_test.c
	set compile_output[1]=ak_atomic_c99_compile_test.exe

	set compile_tests[2]=-std=c11 %test_path%\ak_atomic_compile_test.c
	set compile_output[2]=ak_atomic_c11_compile_test.exe

	set compile_tests[3]=-std=c17 %test_path%\ak_atomic_compile_test.c
	set compile_output[3]=ak_atomic_c17_compile_test.exe

	set compile_tests[4]=-std=c++11 %test_path%\ak_atomic_compile_test.cpp
	set compile_output[4]=ak_atomic_cpp11_compile_test.exe

	set compile_tests[5]=-std=c++14 %test_path%\ak_atomic_compile_test.cpp
	set compile_output[5]=ak_atomic_cpp14_compile_test.exe

	set compile_tests[6]=-std=c++17 %test_path%\ak_atomic_compile_test.cpp
	set compile_output[6]=ak_atomic_cpp17_compile_test.exe

	set compile_tests[7]=-std=c++20 %test_path%\ak_atomic_compile_test.cpp
	set compile_output[7]=ak_atomic_cpp20_compile_test.exe
)

if "%intel%"=="1" (
	set compile_tests[0]=-Qstd=c89 %test_path%\ak_atomic_compile_test.c
	set compile_output[0]=ak_atomic_c89_compile_test.exe

	set compile_tests[1]=-Qstd=c99 %test_path%\ak_atomic_compile_test.c
	set compile_output[1]=ak_atomic_c99_compile_test.exe

	set compile_tests[2]=-Qstd=c11 %test_path%\ak_atomic_compile_test.c
	set compile_output[2]=ak_atomic_c11_compile_test.exe

	set compile_tests[3]=-Qstd=c17 %test_path%\ak_atomic_compile_test.c
	set compile_output[3]=ak_atomic_c17_compile_test.exe

	set compile_tests[6]=-Qstd=c++14 %test_path%\ak_atomic_compile_test.cpp
	set compile_output[6]=ak_atomic_cpp14_compile_test.exe

	set compile_tests[7]=-Qstd=c++17 %test_path%\ak_atomic_compile_test.cpp
	set compile_output[7]=ak_atomic_cpp17_compile_test.exe

	set compile_tests[8]=-Qstd=c++20 %test_path%\ak_atomic_compile_test.cpp
	set compile_output[8]=ak_atomic_cpp20_compile_test.exe
)

:build_step

REM - Build all the tests

echo Compiler: %compiler% %bitness%-bit
echo Mode: %build_mode%
echo Compile Flags: %compile_flags%
echo Link Flags: %compile_link%
echo Output Path: %bin_path%
echo:

set build_failed=0

pushd %bin_path%	
	del *.* /F /Q 2>nul

	set compile_index=0
	echo Building Atomic Tests
	:compile_loop
	if defined compile_tests[%compile_index%] (
		%compile% %compile_flags% %%compile_tests[%compile_index%]%% %compile_link% %compile_out%%%compile_output[%compile_index%]%% || set build_failed=1
		set /a compile_index=%compile_index%+1
		GOTO :compile_loop
	)

	del *.obj /F /Q 2>nul
	del *.ilk /F /Q 2>nul
popd

if %build_failed%==1 exit /b 1