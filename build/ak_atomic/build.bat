@echo off

set base_path=%~dp0..\..
set build_path=%base_path%\build\ak_atomic

set ndk_path=
set is_android_build=0
set arch=
set compiler=

:loop
if not "%1"=="" (
	if "%1"=="-android" (
		set is_android_build=1
	)

    if "%1"=="-ndk" (
        set ndk_path=%2
        shift
    )

	if "%1"=="-arch" (
		set arch=%2
	)

	if "%1"=="-mode" (
		set build_mode=%2
	)

	if "%1"=="-compiler" (
		set compiler=%2
	)

    shift
    goto :loop
)

if %is_android_build% == 1 (
	call %build_path%/internal/build_android.bat %ndk_path% %build_mode% %arch% || exit /b 1
	exit /b 0
)

call %build_path%/internal/build_desktop.bat %compiler% %build_mode% %arch% 
