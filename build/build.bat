@echo off

set all=0

set base_path=%~dp0..
set build_path=%base_path%\build
set bin_path=%base_path%\bin

if not exist %bin_path% mkdir %bin_path%

for %%a in (%*) do set "%%a=1" 

if %all% == 1 (
	call %build_path%\build_atomic.bat msvc v_11 %*
	call %build_path%\build_atomic.bat msvc v_14 %*
	call %build_path%\build_atomic.bat msvc v_2017 %*
	call %build_path%\build_atomic.bat msvc v_2019 %*
	call %build_path%\build_atomic.bat msvc v_2022 %*
	call %build_path%\build_atomic.bat clang %*
	call %build_path%\build_atomic.bat intel %*
	goto end
)
call %build_path%\build_atomic.bat %*

:end