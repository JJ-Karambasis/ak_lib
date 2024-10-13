@echo off

set base_path=%~dp0..\..\..
set build_path=%base_path%\build\ak_atomic\internal
set test_path=%base_path%\tests\ak_atomic

set ndk_path=%1
set build_mode=%2
set arch=%3

if not "%build_mode%" == "release" (
	if not "%build_mode%" == "debug" (
		echo Invalid build mode: %build_mode%. Must be either 'debug' or 'release'
		exit /b 1
	)
)

if "%arch%" == "all" (
	set build_x86=1
	set build_x64=1
	set build_armv7=1
	set build_aarch64=1
)

if not "%arch%" == "all" (
	if "%arch%" == "x86" (
		set build_x86=1
		goto arch_done
	)

	if "%arch%" == "x64" (
		set build_x64=1
		goto arch_done
	)

	if "%arch%" == "armv7" (
		set build_armv7=1
		goto arch_done
	)

	if "%arch%" == "aarch64" (
		set build_aarch64=1
		goto arch_done
	)

	echo Invalid architecture: %arch%. Must be 'all', 'x86', 'x64', 'armv7', or 'aarch64'
	exit /b 1
)

:arch_done

set ndk_clang_path=%ndk_path%\toolchains\llvm\prebuilt\windows-x86_64\bin
set ndk_native_app_glue_path=%ndk_path%\sources\android\native_app_glue

if "%build_mode%" == "release" (
	set output_path_unit_test=%build_path%\android\ak_atomic_unit_test\src\main\jniLibs
)

if "%build_mode%" == "debug" (
	set output_path_unit_test=%build_path%\android\ak_atomic_unit_test\src\debug\jniLibs
)

set sysroot=--sysroot=%ndk_path%\toolchains\llvm\prebuilt\windows-x86_64\sysroot

set clang_warnings=-Werror -Wall
set clang_flags=-g -fdiagnostics-absolute-paths -fPIC -std=c99 %sysroot% -I%ndk_native_app_glue_path% -I%base_path% -I%test_path% %clang_warnings%

set clang_debug_flags=-O0 -DAK_ATOMIC_DEBUG_BUILD %clang_flags%
set clang_release_flags=-O3 %clang_flags%
set clang_link=-llog -landroid
set clang_out=-o

if "%build_mode%" == "release" (
	set compile_flags=%clang_release_flags%
)

if "%build_mode%" == "debug" (
	set compile_flags=%clang_debug_flags%
)

set compile_link=%clang_link%
set compile_out=%clang_out%

set android_glue_flags=%compile_flags% -c -Wno-comment %ndk_native_app_glue_path%\android_native_app_glue.c %compile_out% android_native_app_glue.o
set android_app_flags=%compile_flags% -shared %build_path%\android\ak_atomic_unit_test\src\main\cpp\ak_atomic_unit_test_android.c android_native_app_glue.o %compile_link% -u ANativeActivity_onCreate %compile_out% libnative-activity.so

echo Building Android Unit Tests
echo Mode: %build_mode%
echo Compile Flags: %compile_flags%
echo Link Flags: %compile_link%
echo Output Path: %output_path_unit_test%

if "%build_armv7%" == "1" (
	if not exist %output_path_unit_test%\armeabi-v7a (
		mkdir %output_path_unit_test%\armeabi-v7a
	)

    pushd %output_path_unit_test%\armeabi-v7a
        call %ndk_clang_path%\armv7a-linux-androideabi21-clang %android_glue_flags%
        call %ndk_clang_path%\armv7a-linux-androideabi21-clang %android_app_flags%
    popd

    del %output_path_unit_test%\armeabi-v7a\*.o
)

if "%build_aarch64%" == "1" (
	if not exist %output_path_unit_test%\arm64-v8a (
		mkdir %output_path_unit_test%\arm64-v8a
	)

    pushd %output_path_unit_test%\arm64-v8a
        call %ndk_clang_path%\aarch64-linux-android21-clang %android_glue_flags%
        call %ndk_clang_path%\aarch64-linux-android21-clang %android_app_flags%
    popd

    del %output_path_unit_test%\arm64-v8a\*.o
)

if "%build_x86%" == "1" (
	if not exist %output_path_unit_test%\x86 (
		mkdir %output_path_unit_test%\x86
	)

    pushd %output_path_unit_test%\x86
        call %ndk_clang_path%\i686-linux-android21-clang %android_glue_flags%
        call %ndk_clang_path%\i686-linux-android21-clang %android_app_flags%
    popd

    del %output_path_unit_test%\x86\*.o
)

if "%build_x64%" == "1" (
	if not exist %output_path_unit_test%\x86_64 (
		mkdir %output_path_unit_test%\x86_64
	)

    pushd %output_path_unit_test%\x86_64
        call %ndk_clang_path%\x86_64-linux-android21-clang %android_glue_flags%
        call %ndk_clang_path%\x86_64-linux-android21-clang %android_app_flags%
    popd

    del %output_path_unit_test%\x86_64\*.o
)