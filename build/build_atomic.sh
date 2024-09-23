# Common global variables
release=0
debug=0
env32=0
env64=0
armv6=0
armv7=0
thumb=0
clang=1 # Clang is the only supported compiler right now

build_path=$(dirname "$(realpath "$0")")
base_path="$build_path/.."
bin_path="$base_path/bin/ak_atomic"
test_path="$base_path/tests/ak_atomic"

# Get the architecture of the computer
aarch=$(uname -m)
echo $aarch

# Build inputs and validation
for var in "$@"
do
    if [ "$var" = "release" ]; then release=1; fi
    if [ "$var" = "debug" ]; then debug=1; fi
    if [ "$var" = "env32" ]; then env32=1; fi
    if [ "$var" = "env64" ]; then env64=1; fi
    if [ "$var" = "armv6" ]; then armv6=1; fi
    if [ "$var" = "armv7" ]; then armv7=1; fi
    if [ "$var" = "thumb" ]; then thumb=1; fi
done

if [ ! $release -eq 1 ]; then debug=1; fi
if [ ! $env32 -eq 1 ]; then env64=1; fi

# Build information
if [ $clang -eq 1 ]; then compiler="Clang"; fi

if [ $env32 -eq 1 ]; then bitness=32; fi
if [ $env64 -eq 1 ]; then bitness=64; fi

if [ $debug -eq 1 ]; then build_mode="Debug"; fi
if [ $release -eq 1 ]; then build_mode="Release"; fi

# Get the proper binary directory based on build inputs (goes compiler -> platform -> mode -> executable)
if [ $clang -eq 1 ]; then bin_path="$bin_path/clang"; fi

if [ $env32 -eq 1 ]; then bin_path="$bin_path/env32"; fi
if [ $env64 -eq 1 ]; then bin_path="$bin_path/env64"; fi

if [ $debug -eq 1 ]; then bin_path="$bin_path/debug"; fi
if [ $release -eq 1 ]; then bin_path="$bin_path/release"; fi

if [ ! -d $bin_path ]; then
    mkdir -p $bin_path
fi

# Clang flags
clang_warnings="-Werror -Wall"
clang_flags="-g -fdiagnostics-absolute-paths -I$base_path $clang_warnings"

if [ $clang -eq 1 ]; then
    if [ $env32 -eq 1 ]; then
        clang_flags="-m32 $clang_flags"
    fi

    # Architecture override flags for clang
    if [ $armv6 -eq 1 ]; then
        clang_flags="-arch armv6 $clang_flags"
    elif [ $armv7 -eq 1 ]; then
        clang_flags="-arch armv7 $clang_flags"
    else 
        clang_flags="-arch $aarch $clang_flags"
    fi
fi

clang_debug_flags="-O0 $clang_flags"
clang_release_flags="-O2 $clang_flags"
clang_link=
clang_out="-o"
clang_compile="clang"

# Unified compile flags

if [ $clang -eq 1 ]; then compile_debug_flags=$clang_debug_flags; fi
if [ $clang -eq 1 ]; then compile_release_flags=$clang_release_flags; fi
if [ $clang -eq 1 ]; then compile_link=$clang_link; fi
if [ $clang -eq 1 ]; then compile_out=$clang_out; fi
if [ $clang -eq 1 ]; then compile=$clang_compile; fi

# Choose between release and debug builds
if [ $debug -eq 1 ]; then compile_flags=$compile_debug_flags; fi
if [ $release -eq 1 ]; then compile_flags=$compile_release_flags; fi

# Get all the configuration tests based on compiler and platform
declare -a compile_tests
declare -a compile_output
declare -a compile_std

if [ $clang -eq 1 ]; then
    compile_std[0]="-std=c89 -ansi"
    compile_tests[0]="$test_path/ak_atomic_compile_test.c"
	compile_output[0]="ak_atomic_c89_compile_test"

    compile_std[1]="-std=c99"
	compile_tests[1]="$test_path/ak_atomic_compile_test.c"
	compile_output[1]="ak_atomic_c99_compile_test"

    compile_std[2]="-std=c11"
	compile_tests[2]="$test_path/ak_atomic_compile_test.c"
	compile_output[2]="ak_atomic_c11_compile_test"

    compile_std[3]="-std=c17"
	compile_tests[3]="$test_path/ak_atomic_compile_test.c"
	compile_output[3]="ak_atomic_c17_compile_test"

    compile_std[4]="-std=c++11"
	compile_tests[4]="$test_path/ak_atomic_compile_test.cpp"
	compile_output[4]="ak_atomic_cpp11_compile_test"

    compile_std[5]="-std=c++14"
	compile_tests[5]="$test_path/ak_atomic_compile_test.cpp"
	compile_output[5]="ak_atomic_cpp14_compile_test"

    compile_std[6]="-std=c++17"
	compile_tests[6]="$test_path/ak_atomic_compile_test.cpp"
	compile_output[6]="ak_atomic_cpp17_compile_test"

    compile_std[7]="-std=c++20"
	compile_tests[7]="$test_path/ak_atomic_compile_test.cpp"
	compile_output[7]="ak_atomic_cpp20_compile_test"

    compile_std[8]="-std=c++98"
	compile_tests[8]="$test_path/ak_atomic_compile_test.cpp"
	compile_output[8]="ak_atomic_cpp98_compile_test"


fi

# Build all the tests

echo "Compiler: $compiler $bitness-bit"
echo "Mode: $build_mode"
echo "Compile Flags: $compile_flags"
echo "Link Flags: $compile_link"
echo "Output Path: $bin_path"
echo

pushd $bin_path
    for i in "${!compile_tests[@]}"; do 
        ${compile} ${compile_flags} ${compile_std[$i]} "${compile_tests[$i]}" $compile_link "${compile_out}${compile_output[$i]}"
    done
popd