@echo off

set base_path=%~dp0..
set build_path=%base_path%\build
set bin_path=%base_path%\bin

if not exist %bin_path% mkdir %bin_path%

call %build_path%\build_atomic.bat %*