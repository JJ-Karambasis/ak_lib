@echo off

set base_path=%~dp0..\..
set build_path=%base_path%\build\ak_atomic

set arch=
set compiler=
set build_mode=

:loop
if not "%1"=="" (
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

set bin_path=%base_path%\bin\ak_atomic\%compiler%\%arch%\%build_mode%

for /r "%bin_path%" %%a in (*unit_test.exe) do (
	echo Running %%a 
	%%~fa
)