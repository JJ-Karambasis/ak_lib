debug=0
release=0
gcc=0
intel=0
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
    if [ "$var" = "gcc" ]; then gcc=1; fi
    if [ "$var" = "intel" ]; then intel=1; fi
    if [ "$var" = "clang" ]; then clang=1; fi
    if [ "$var" = "env32" ]; then env32=1; fi
    if [ "$var" = "env64" ]; then env64=1; fi
done


if [ ! $release -eq 1 ]; then debug=1; fi
if [ ! $env32 -eq 1 ]; then env64=1; fi

if [ ! $intel -eq 1 ]; then
    if [! $clang -eq 1 ]; then
        gcc=1
    fi
fi

if [ $intel -eq 1 ]; then
    if [ $env32 -eq 1 ]; then source $HOME/intel/oneapi/setvars.sh ia32; fi
    if [ $env64 -eq 1 ]; then source $HOME/intel/oneapi/setvars.sh intel64; fi
fi

gcc_warnings="-Werror -Wall"
gcc_common="-g -I$BasePath -fPIC -ansi -std=c89 $gcc_warnings"

intel_warnings="-Werror -Wall"
intel_common="-g -gcodeview -fdiagnostics-absolute-paths -I$BasePath $intel_warnings -ansi -std=c89"

clang_warnings="-Werror -Wall"
clang_common="-g -gcodeview -fdiagnostics-absolute-paths -I$BasePath $intel_warnings -ansi -std=c89"

if [ $env32 -eq 1 ]; then
    gcc_common="$gcc_common -m32"
    intel_common="$intel_common -m32"
    clang_common="$clang_common -m32"
fi

gcc_debug="gcc -O0 $gcc_common"
gcc_release="gcc -O2 $gcc_common"
gcc_out="-o"

intel_debug="icx -O0 $intel_common"
intel_release="icx -O2 $intel_common"
intel_out="-o"

clang_debug="clang -O0 $clang_common"
clang_release="clang -O2 $clang_common"
clang_out="-o"

if [ $gcc -eq 1 ]; then compile_debug=${gcc_debug}; fi
if [ $gcc -eq 1 ]; then compile_release=${gcc_release}; fi
if [ $gcc -eq 1 ]; then compile_out=${gcc_out}; fi

if [ $intel -eq 1 ]; then compile_debug=${intel_debug}; fi
if [ $intel -eq 1 ]; then compile_release=${intel_release}; fi
if [ $intel -eq 1 ]; then compile_out=${intel_out}; fi

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