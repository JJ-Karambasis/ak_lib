@echo off

set BasePath=%~dp0..

for %%a in (%*) do set "%%a=1"
if not "%release%"=="1" set debug=1
if not "%env32%"=="1" set env64=1

if not "%intel%"=="1" (
    if not "%clang%"=="1" set msvc=1
)

if "%msvc%"=="1" (
    if "%env32%"=="1" call %BasePath%\build\setup_compiler.bat x86
    if "%env64%"=="1" call %BasePath%\build\setup_compiler.bat x64
)

if "%intel%"=="1" (
    if "%env32%"=="1" call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" ia32
    if "%env64%"=="1" call "C:\Program Files (x86)\Intel\oneAPI\setvars.bat" intel64
)

set cl_warnings= /WX /Wall /wd4100 /wd4242 /wd4244 /wd5045
set cl_common=   /nologo /D_CRT_SECURE_NO_WARNINGS /FC /Z7 /I%BasePath%\ %cl_warnings% /Tc

set clang_warnings= -Werror -Wall 
set clang_common=   -g -gcodeview -fdiagnostics-absolute-paths -D_CRT_SECURE_NO_WARNINGS -I%BasePath%\ %clang_warnings% -std=c89

set intel_warnings= -Werror -Wall
set intel_common=   -g -gcodeview -fdiagnostics-absolute-paths -D_CRT_SECURE_NO_WARNINGS -I%BasePath%\ %intel_warnings% -Qstd=c89

if "%clang%"=="1" (
    if "%env32%"=="1" set clang_common=%clang_common% -m32
)

if "%intel%"=="1" (
    if "%env32%"=="1" set intel_common=%intel_common% -m32
)

set cl_debug=    call cl /Od %cl_common%
set cl_release=  call cl /O2 %cl_common%
set cl_link=     /link /incremental:no
set cl_out=      /out:

set clang_debug=    call clang -O0 %clang_common%
set clang_release=  call clang -O2 %clang_common%
set clang_link=     
set clang_out=      -o

set intel_debug=    call icx -Od %intel_common%
set intel_release=  call icx -O2 %intel_common%
set intel_link=
set intel_out=      -o

if "%msvc%"=="1" set compile_debug=   %cl_debug%
if "%msvc%"=="1" set compile_release= %cl_release%
if "%msvc%"=="1" set compile_link=    %cl_link%
if "%msvc%"=="1" set compile_out=     %cl_out%

if "%clang%"=="1" set compile_debug=   %clang_debug%
if "%clang%"=="1" set compile_release= %clang_release%
if "%clang%"=="1" set compile_link=    %clang_link%
if "%clang%"=="1" set compile_out=     %clang_out%

if "%intel%"=="1" set compile_debug=   %intel_debug%
if "%intel%"=="1" set compile_release= %intel_release%
if "%intel%"=="1" set compile_link=    %intel_link%
if "%intel%"=="1" set compile_out=     %intel_out%

if "%debug%"=="1"   set compile=%compile_debug%
if "%release%"=="1" set compile=%compile_release%

if not exist %BasePath%\tests\ak_atomic_test_bin\ ( mkdir %BasePath%\tests\ak_atomic_test_bin\ )
pushd %BasePath%\tests\ak_atomic_test_bin\
    %compile% %BasePath%\tests\ak_atomic_test.c %compile_link% %compile_out%ak_atomic_test.exe
popd