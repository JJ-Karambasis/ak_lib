
Script=$(realpath "$0")
ScriptPath=$(dirname "$Script")

BasePath="$ScriptPath/.."

if [ ! -d "$BasePath/tests/ak_fbx_test_bin/" ]; then
    mkdir "$BasePath/tests/ak_fbx_test_bin/"
fi

pushd "$BasePath/tests/ak_fbx_test_bin/"

clang -g -Werror -O0 -fPIC -std=c89 "$BasePath/tests/ak_fbx_test.c" -o ak_fbx_test

popd