@echo off

set BasePath=%~dp0..

call %BasePath%\build\setup_compiler.bat x64

:: AK FBX Tests
if not exist %BasePath%\tests\ak_fbx_test_bin\ ( mkdir %BasePath%\tests\ak_fbx_test_bin\ )
pushd %BasePath%\tests\ak_fbx_test_bin\
cl -nologo -Od -FC -Z7 -WX -Wall -wd5045 -I%BasePath%\ -Za -Tc %BasePath%\tests\ak_fbx_test.c -link -out:ak_fbx_test.exe
popd