debug=0
release=0

Script=$(realpath "$0")
ScriptPath=$(dirname "$Script")
BasePath="$ScriptPath/.."

for var in "$@"
do
    if [ $var=release ]; then release=1; fi
done

if [ ! $release -eq 1 ]; then debug=1; fi

clang_warnings="-Werror -Wall"
clang_common="-I$BasePath -fPIC -ansi $clang_warnings"
clang_debug="gcc -g -O0 -DDEBUG $clang_common"
clang_release="gcc -g -O2 $clang_common"
clang_out="-o"

if [ $debug -eq 1 ]; then compile=${clang_debug}; fi
if [ $release -eq 1 ]; then compile=${clang_release}; fi

pushd "$BasePath/tests/ak_atomic_test_bin/"

    if [ ! -d "$BasePath/tests/ak_atomic_test_bin/" ]; then
        mkdir "$BasePath/tests/ak_atomic_test_bin/"
    fi

    ${compile} $BasePath/tests/ak_atomic_test.c ${clang_out} ak_atomic_test
popd