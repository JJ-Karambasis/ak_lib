build_path=$(dirname "$(realpath "$0")")
base_path="$build_path/../../.."
test_path="$base_path/tests/ak_atomic"

compiler=$1
build_mode=$2
arch=$3

if [ $build_mode != "release" ]; then
    if [ $build_mode != "debug" ]; then
        echo "Invalid build mode: $build_mode. Must be either 'debug' or 'release'"
        exit 1
    fi
fi

actual_arch="none"
if [ $arch == "x64" ]; then actual_arch="x86_64"; fi
if [ $arch == "x86" ]; then actual_arch="i686"; fi
if [ $arch == "aarch64" ]; then actual_arch="aarch64"; fi

if [ $actual_arch == "none" ]; then
    echo "Invalid architecture $arch not supported"
    exit 1
fi

# Binary path
bin_path="$base_path/bin/ak_atomic/$compiler/$arch/$build_mode"
if [ ! -d $bin_path ]; then
    mkdir -p $bin_path
fi

machine="none"
if [ "$(uname)" == "Darwin" ]; then
    # Do something under Mac OS X platform   
    machine="apple-darwin"     
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    machine="linux-gnu"
fi

# Clang flags
clang_warnings="-Werror -Wall -Wno-implicit-function-declaration"
clang_flags="-g -fdiagnostics-absolute-paths -I$base_path $clang_warnings -target $arch-$machine"

clang_debug_flags="-O0 $clang_flags"
clang_release_flags="-O3 $clang_flags"
clang_link=
clang_out="-o"
clang_compile="clang"

# Unified compile flags
if [ $compiler == "clang" ]; then compile_debug_flags=$clang_debug_flags; fi
if [ $compiler == "clang" ]; then compile_release_flags=$clang_release_flags; fi
if [ $compiler == "clang" ]; then compile_link=$clang_link; fi
if [ $compiler == "clang" ]; then compile_out=$clang_out; fi
if [ $compiler == "clang" ]; then compile=$clang_compile; fi

# Choose between release and debug builds
if [ $build_mode == "debug" ]; then compile_flags=$compile_debug_flags; fi
if [ $build_mode == "release" ]; then compile_flags=$compile_release_flags; fi

# Get all the configuration tests based on compiler and platform
declare -a compile_tests
declare -a compile_output
declare -a compile_std

echo $compile_flags

if [ $compiler == "clang" ]; then
    compile_std+=("-std=c89 -ansi")
    compile_tests+=("$test_path/ak_atomic_unit_test.c")
	compile_output+=("ak_atomic_c89_unit_test")

    compile_std+=("-std=c99")
    compile_tests+=("$test_path/ak_atomic_unit_test.c")
	compile_output+=("ak_atomic_c99_unit_test")

    compile_std+=("-std=c11")
    compile_tests+=("$test_path/ak_atomic_unit_test.c")
	compile_output+=("ak_atomic_c11_unit_test")

    compile_std+=("-std=c17")
    compile_tests+=("$test_path/ak_atomic_unit_test.c")
	compile_output+=("ak_atomic_c17_unit_test")

    compile_std+=("-std=c++98 -lstdc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp98_unit_test")

    compile_std+=("-std=c++11 -lstdc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp11_unit_test")

    compile_std+=("-std=c++14 -lstdc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp14_unit_test")

    compile_std+=("-std=c++17 -lstdc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp17_unit_test")

    compile_std+=("-std=c++20 -lstdc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp20_unit_test")
fi

pushd $bin_path
    for i in "${!compile_tests[@]}"; do 
        ${compile} ${compile_flags} ${compile_std[$i]} "${compile_tests[$i]}" $compile_link "${compile_out}${compile_output[$i]}"
    done
popd