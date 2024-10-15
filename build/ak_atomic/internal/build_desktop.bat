setlocal ENABLEDELAYEDEXPANSION

set base_path=%~dp0..\..\..
set build_path=%base_path%\build\ak_atomic\internal
set test_path=%base_path%\tests\ak_atomic

set compiler=%1
set build_mode=%2
set arch=%3

if not "%build_mode%" == "release" (
	if not "%build_mode%" == "debug" (
		echo Invalid build mode: %build_mode%. Must be either 'debug' or 'release'
		exit /b 1
	)
)

if "%compiler%"=="msvc" (
	call %build_path%\..\..\setup_msvc.bat %arch% || exit /b 1
)

if "%compiler%"=="intel" (
	if "%arch%"=="x86" call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" ia32 || exit /b 1
	if "%arch%"=="x64" call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" intel64 || exit /b 1
)

REM - Binary path
set bin_path=%base_path%\bin\ak_atomic\%compiler%\%arch%\%build_mode%
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

if "%compiler%"=="clang" (
	if "%arch%"=="x86" (
		set clang_flags=%clang_flags% -target i686-windows-unknown
	)

	if "%arch%"=="x64" (
		set clang_flags=%clang_flags% -target x86_64-windows-unknown
	)
)

set clang_debug_flags=-O0 -DAK_ATOMIC_DEBUG_BUILD %clang_flags%
set clang_release_flags=-O2 %clang_flags%
set clang_link=
set clang_out=-o
set clang_compile=call clang

REM - Intel flags
set intel_warnings=-Werror -Wall
set intel_flags=-g -gcodeview -fdiagnostics-absolute-paths -I%base_path% %intel_warnings%

if "%compiler%"=="intel" (
	if "%arch%"=="x86" (
		set intel_flags=%intel_flags% -target i686-windows-unknown
	)

	if "%arch%"=="x64" (
		set intel_flags=%intel_flags% -target x86_64-windows-unknown
	)
)

set intel_debug_flags=-Od %intel_flags%
set intel_release_flags=-O2 %intel_flags%
set intel_link=-link -incremental:no -nologo
set intel_out=-out:
set intel_compile=call icx

REM - Unified compile flags
if "%compiler%"=="msvc" set compile_debug_flags=%msvc_debug_flags%
if "%compiler%"=="msvc" set compile_release_flags=%msvc_release_flags%
if "%compiler%"=="msvc" set compile_link=%msvc_link%
if "%compiler%"=="msvc" set compile_out=%msvc_out%
if "%compiler%"=="msvc" set compile=%msvc_compile%

if "%compiler%"=="clang" set compile_debug_flags=%clang_debug_flags%
if "%compiler%"=="clang" set compile_release_flags=%clang_release_flags%
if "%compiler%"=="clang" set compile_link=%clang_link%
if "%compiler%"=="clang" set compile_out=%clang_out%
if "%compiler%"=="clang" set compile=%clang_compile%

if "%compiler%"=="intel" set compile_debug_flags=%intel_debug_flags%
if "%compiler%"=="intel" set compile_release_flags=%intel_release_flags%
if "%compiler%"=="intel" set compile_link=%intel_link%
if "%compiler%"=="intel" set compile_out=%intel_out%
if "%compiler%"=="intel" set compile=%intel_compile%

if not "%build_mode%" == "release" set compile_flags=%compile_debug_flags%
if not "%build_mode%" == "debug" set compile_flags=%compile_release_flags%

REM - Get all the configuration tests based on compiler and platform
set compile_tests=
set compile_output=
set i=0

if "%compiler%"=="msvc" (
	set compile_tests[!i!]=/std:c11 /Tc %test_path%\ak_atomic_unit_test.c
	set compile_output[!i!]=ak_atomic_c11_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=/std:c++14 /Zc:__cplusplus /D_HAS_EXCEPTIONS=0 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp14_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=/std:c++17 /Zc:__cplusplus /D_HAS_EXCEPTIONS=0 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp17_unit_test.exe
	set /a i=i+1

	set compile_tests[!i!]=/std:c++20 /Zc:__cplusplus /D_HAS_EXCEPTIONS=0 %test_path%\ak_atomic_unit_test.cpp
	set compile_output[!i!]=ak_atomic_cpp20_unit_test.exe
	set /a i=i+1
)

if "%compiler%"=="clang" (
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

if "%compiler%"=="intel" (
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

echo %compile_flags%

pushd %bin_path%	
	del *.* /F /Q 2>nul

	set compile_index=0
	echo Building Atomic Tests
	:compile_loop
	if defined compile_tests[%compile_index%] (
		%compile% %compile_flags% %%compile_tests[%compile_index%]%% %compile_link% %compile_out%%%compile_output[%compile_index%]%%
		set /a compile_index=%compile_index%+1
		GOTO :compile_loop
	)

	del *.obj /F /Q 2>nul
	del *.ilk /F /Q 2>nul
popd