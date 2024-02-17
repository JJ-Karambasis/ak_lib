@echo off

set BasePath=%~dp0..

call %BasePath%\build\setup_compiler.bat x64

set Flags=-nologo -Od -FC -Z7 -WX -Wall -wd4100 -wd4242 -wd4244 -wd5045  -I%BasePath%\ -Tc

:: AK FBX Tests
if not exist %BasePath%\tests\ak_fbx_test_bin\ ( mkdir %BasePath%\tests\ak_fbx_test_bin\ )
pushd %BasePath%\tests\ak_fbx_test_bin\
cl %Flags% %BasePath%\tests\ak_fbx_test.c -link -out:ak_fbx_test.exe
popd

:: AK Sim Tests
if not exist %BasePath%\tests\ak_atomic_test_bin\ ( mkdir %BasePath%\tests\ak_atomic_test_bin\ )
pushd %BasePath%\tests\ak_atomic_test_bin\
cl %Flags% %BasePath%\tests\ak_atomic_test.c -link -out:ak_atomic_test.exe
popd