@echo off

set BasePath=%~dp0..

for %%a in (%*) do set "%%a=1"
if not "%release%"=="1" set debug=1

if "%all%"=="1" (
    set armv7a=1
    set aarch64=1
    set x86=1
    set x86_64=1
)

set ndk_path=%ANDROID_HOME%\ndk\25.2.9519653
set ndk_clang_path=%ndk_path%\toolchains\llvm\prebuilt\windows-x86_64\bin
set ndk_native_app_blue_path=%ndk_path%\sources\android\native_app_glue
set lib_path=%BasePath%\build\android\ak_atomic_test\src\main\jniLibs\

set linker_flags=-llog -landroid
set sysroot=--sysroot=%ndk_path%\toolchains\llvm\prebuilt\windows-x86_64\sysroot

set compile_warnings=-Werror -Wall
set compile_common=-I%BasePath% -I%ndk_native_app_blue_path% %sysroot% -DANDROID_BUILD -fdiagnostics-absolute-paths -fPIC -std=c89 %compile_warnings%
set compile_debug=-g -O0 %compile_common%
set compile_release=-g -O2 %compile_common%
set compile_out=-o
set compile_shared_library=-shared

if "%release%"=="1" set compile_flags=%compile_release%
if "%debug%"=="1" set compile_flags=%compile_debug%

set android_glue_flags=%compile_flags% -c -Wno-comment %ndk_native_app_blue_path%\android_native_app_glue.c %compile_out% android_native_app_glue.o
set android_app_flags=%compile_flags% %compile_shared_library% %linker_flags% %BasePath%\build\android\ak_atomic_test\src\main\cpp\ak_android_atomic_test.c android_native_app_glue.o -u ANativeActivity_onCreate %compile_out% libnative-activity.so

if "%armv7a%"=="1" (
    if not exist %lib_path%\armeabi-v7a ( 
        mkdir %lib_path%\armeabi-v7a\
    )

    pushd %lib_path%\armeabi-v7a
        call %ndk_clang_path%\armv7a-linux-androideabi19-clang %android_glue_flags%
        call %ndk_clang_path%\armv7a-linux-androideabi19-clang %android_app_flags%
    popd

    del %lib_path%\armeabi-v7a\*.o
)

if "%aarch64%"=="1" (
    if not exist %lib_path%\arm64-v8a (
        mkdir %lib_path%\arm64-v8a\
    )

    pushd %lib_path%\arm64-v8a
        call %ndk_clang_path%\aarch64-linux-android21-clang %android_glue_flags%
        call %ndk_clang_path%\aarch64-linux-android21-clang %android_app_flags%
    popd

    del %lib_path%\arm64-v8a\*.o
)

if "%x86%"=="1" (
    if not exist %lib_path%\x86 (
        mkdir %lib_path%\x86\
    )

    pushd %lib_path%\x86
        call %ndk_clang_path%\i686-linux-android19-clang %android_glue_flags%
        call %ndk_clang_path%\i686-linux-android19-clang %android_app_flags%
    popd

    del %lib_path%\x86\*.o
)

if "%x86_64%"=="1" (
    if not exist %lib_path%\x86_64 (
        mkdir %lib_path%\x86_64\
    )

    pushd %lib_path%\x86_64
        call %ndk_clang_path%\x86_64-linux-android21-clang %android_glue_flags%
        call %ndk_clang_path%\x86_64-linux-android21-clang %android_app_flags%
    popd

    del %lib_path%\x86_64\*.o
)