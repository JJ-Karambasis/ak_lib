@echo off

set BasePath=%~dp0..

for %%a in (%*) do set "%%a=1"
if not "%release%"=="1" set debug=1

set ndk_path=%ANDROID_HOME%\ndk\25.2.9519653
set ndk_clang_path=%ndk_path%\toolchains\llvm\prebuilt\windows-x86_64\bin
set ndk_native_app_blue_path=%ndk_path%\sources\android\native_app_glue
set lib_path=%BasePath%\build\android\ak_atomic_test\src\main\jniLibs\

set linker_flags=-landroid_support -llog -landroid
set sysroot=--sysroot=%ndk_path%\toolchains\llvm\prebuilt\windows-x86_64\sysroot

set compile_warnings=-Werror -Wall -Wno-comment
set compile_common=-Wl,--build-id=sha1 -I%BasePath% -I%ndk_native_app_blue_path% %sysroot% -DANDROID_BUILD -DANDROID -fdiagnostics-absolute-paths -fPIC -std=c89 %compile_warnings%
set compile_debug=-g -O0 -DDEBUG %compile_common%
set compile_release=-g -O2 %compile_common%
set compile_out=-o
set compile_shared_library=-shared

if "%release%"=="1" set compile_flags=%compile_release%
if "%debug%"=="1" set compile_flags=%compile_debug%

if not exist %lib_path%\armeabi-v7a ( 
    mkdir %lib_path%\armeabi-v7a\
)

pushd %lib_path%\armeabi-v7a
%ndk_clang_path%\armv7a-linux-androideabi31-clang %compile_flags% %compile_shared_library% %linker_flags% %BasePath%\build\android\ak_atomic_test\src\main\cpp\ak_android_atomic_test.c %ndk_native_app_blue_path%\android_native_app_glue.c -u ANativeActivity_onCreate %compile_out% libnative-activity.so
popd