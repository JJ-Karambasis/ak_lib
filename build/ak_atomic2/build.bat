@echo off

setlocal ENABLEDELAYEDEXPANSION
REM - Common global variables

set base_path=%~dp0..\..
set build_path=%base_path%\build\ak_atomic
set bin_path=%base_path%\bin\ak_atomic
set test_path=%base_path%\tests\ak_atomic

REM - Build inputs and validation
for %%a in (%*) do set "%%a=1" 

if "%android%"=="1" (
	call %build_path%\build_android.bat %* || exit /b 1
	exit /b 0
)

if not "%release%"=="1" set debug=1
if not "%intel%"=="1" (
	if not "%clang%"=="1" set msvc=1
)

set msvc_version=0
if "%msvc%"=="1" (
	if "%x64%"=="1" call %build_path%\..\setup_msvc.bat x64 || exit /b 1
	if "%x86%"=="1" call %build_path%\..\setup_msvc.bat x86 || exit /b 1
)

if "%intel%"=="1" (
	call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" || exit /b 1
)

REM - Build information
if "%msvc%"=="1" set compiler=MSVC %msvc_version%
if "%clang%"=="1" set compiler=Clang
if "%intel%"=="1" set compiler=Intel

if "%debug%"=="1" set build_mode=Debug
if "%release%"=="1" set build_mode=Release

REM - Get the proper binary directory based on build inputs (goes compiler -> platform -> mode -> executable)
if "%msvc%"=="1" set bin_path=%bin_path%\msvc
if "%clang%"=="1" set bin_path=%bin_path%\clang
if "%intel%"=="1" set bin_path=%bin_path%\intel

if "%x64%"=="1" set bin_path=%bin_path%\x64
if "%x86%"=="1" set bin_path=%bin_path%\x86

if "%debug%"=="1" set bin_path=%bin_path%\debug
if "%release%"=="1" set bin_path=%bin_path%\release

if not exist %bin_path% mkdir %bin_path%

REM - MSVC flags
set msvc_warnings=/WX /Wall /wd4710 /wd4711
set msvc_flags=/nologo /FC /Z7 -I%base_path% %msvc_warnings%

set msvc_debug_flags=/Od %msvc_flags%
set msvc_release_flags=/O2 /Oi- %msvc_flags%
set msvc_link=/link /incremental:no
set msvc_out=/out:
set msvc_compile=call cl

REM - Clang flags
set clang_warnings=-Werror -Wall
set clang_flags=-g -gcodeview -fdiagnostics-absolute-paths -I%base_path% %clang_warnings%

set clang_debug_flags=-O0 -DAK_ATOMIC_DEBUG_BUILD %clang_flags%
set clang_release_flags=-O2 %clang_flags%
set clang_link=
set clang_out=-o
set clang_compile=call clang

REM - Intel flags
set intel_warnings=-Werror -Wall
set intel_flags=-g -gcodeview -fdiagnostics-absolute-paths -I%base_path% %intel_warnings%

set intel_debug_flags=-Od %intel_flags%
set intel_release_flags=-O2 %intel_flags%
set intel_link=-link -incremental:no -nologo
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
set i=0

if "%msvc%"=="1" (
	set compile_tests[!i!]=/std:c11 /Tc %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c11_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=/std:c++14 /Zc:__cplusplus /D_HAS_EXCEPTIONS=0 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp14_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=/std:c++17 /Zc:__cplusplus /D_HAS_EXCEPTIONS=0 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp17_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=/std:c++20 /Z  c:__cplusplus /D_HAS_EXCEPTIONS=0 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp20_unit_test.exe
	set /a i=i+1
)

if "%clang%"=="1" (
	set compile_tests[!i!]=-std=gnu89 %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c89_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-std=c99 %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c99_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-std=c11 %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c11_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-std=c17 %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c17_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-std=c++14 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp14_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-std=c++17 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp17_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-std=c++20 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp20_unit_test.exe
	set /a i=i+1
)

if "%intel%"=="1" (
	set compile_tests[!i!]=-Qstd=c89 %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c89_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-Qstd=c99 %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c99_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-Qstd=c11 %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c11_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-Qstd=c17 %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c17_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-Qstd=c++14 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp14_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-Qstd=c++17 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp17_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=-Qstd=c++20 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp20_unit_test.exe
	set /a i=i+1
)

:build_step

REM - Build all the tests

echo Compiler: %compiler%
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