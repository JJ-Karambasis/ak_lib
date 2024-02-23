debug=0
release=0
clang=0
env32=0
env64=0

Script=$(realpath "$0")
ScriptPath=$(dirname "$Script")

BasePath="$ScriptPath/.."

for var in "$@"
do
    if [ "$var" = "release" ]; then release=1; fi
    if [ "$var" = "debug" ]; then debug=1; fi
    if [ "$var" = "clang" ]; then clang=1; fi
    if [ "$var" = "env32" ]; then env32=1; fi
    if [ "$var" = "env64" ]; then env64=1; fi
done

if [ ! $release -eq 1 ]; then debug=1; fi
if [ ! $env32 -eq 1 ]; then env64=1; fi

#OSX only supports clang right now
clang=1

clang_warnings="-Werror -Wall"
clang_common="-g -fdiagnostics-absolute-paths -I$BasePath $clang_warnings -arch arm64 -ansi -std=c89"

if [ $env32 -eq 1 ]; then
    clang_common="$clang_common -m32"
fi

clang_debug="clang -O0 $clang_common"
clang_release="clang -O2 $clang_common"
clang_out="-o"

if [ $clang -eq 1 ]; then compile_debug=${clang_debug}; fi
if [ $clang -eq 1 ]; then compile_release=${clang_release}; fi
if [ $clang -eq 1 ]; then compile_out=${clang_out}; fi

if [ $debug -eq 1 ]; then compile=${compile_debug}; fi
if [ $release -eq 1 ]; then compile=${compile_release}; fi

if [ ! -d "$BasePath/tests/ak_atomic_test_bin/" ]; then
    mkdir "$BasePath/tests/ak_atomic_test_bin/"
fi

pushd "$BasePath/tests/ak_atomic_test_bin/"
    ${compile} $BasePath/tests/ak_atomic_test.c ${compile_out} ak_atomic_test
popd