@echo off

:loop
if not "%1"=="" (
    if "%1"=="-ndk" (
        set ndk_version=%2
        shift
    )
    shift
    goto :loop
)

echo %ndk_version%