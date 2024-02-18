debug=0
release=0

for var in "$@"
do
    if [ $var=release ]; then release=1; fi
done

if [ ! $release -eq 1 ]; then debug=1; fi

Script=$(realpath "$0")
ScriptPath=$(dirname "$Script")

BasePath="$ScriptPath/.."

if [ ! -d "$BasePath/tests/ak_fbx_test_bin/" ]; then
    mkdir "$BasePath/tests/ak_fbx_test_bin/"
fi

if [ ! -d "$BasePath/tests/ak_atomic_test_bin/" ]; then
    mkdir "$BasePath/tests/ak_atomic_test_bin/"
fi

gcc_warnings="-Werror -Wall"
gcc_common="-I$BasePath -fPIC -ansi -m32 -pthread $gcc_warnings"
gcc_debug="gcc -g -O0 -DDEBUG $gcc_common"
gcc_release="gcc -g -O2 $gcc_common"
gcc_out="-o"

if [ $debug -eq 1 ]; then compile=${gcc_debug}; fi
if [ $release -eq 1 ]; then compile=${gcc_release}; fi

pushd "$BasePath/tests/ak_atomic_test_bin/"
${compile} $BasePath/tests/ak_atomic_test.c ${gcc_out} ak_atomic_test
popd