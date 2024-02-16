
Script=$(realpath "$0")
ScriptPath=$(dirname "$Script")

BasePath="$ScriptPath/.."

if [ ! -d "$BasePath/tests/ak_fbx_test_bin/" ]; then
    mkdir "$BasePath/tests/ak_fbx_test_bin/"
fi

if [ ! -d "$BasePath/tests/ak_atomic_test_bin/" ]; then
    mkdir "$BasePath/tests/ak_atomic_test_bin/"
fi

pushd "$BasePath/tests/ak_fbx_test_bin/"

# clang -g -Werror -O0 -fPIC -std=c89 "$BasePath/tests/ak_fbx_test.c" -o ak_fbx_test

popd

pushd "$BasePath/tests/ak_atomic_test_bin/"

clang -g -Werror -O2 -fPIC -std=c89 -I"$BasePath" "$BasePath/tests/ak_atomic_test.c" -o ak_atomic_test

popd 