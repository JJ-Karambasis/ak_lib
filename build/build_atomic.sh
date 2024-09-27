# Common global variables
release=0
debug=0
clang=1 # Clang is the only supported compiler right now

build_path=$(dirname "$(realpath "$0")")
base_path="$build_path/.."
bin_path="$base_path/bin/ak_atomic"
test_path="$base_path/tests/ak_atomic"

# Get the architecture of the computer
aarch=$(uname -m)

# Build inputs and validation
for var in "$@"
do
    if [ "$var" = "release" ]; then release=1; fi
    if [ "$var" = "debug" ]; then debug=1; fi
done

if [ ! $release -eq 1 ]; then debug=1; fi

# Build information
if [ $clang -eq 1 ]; then compiler="Clang"; fi

if [ $debug -eq 1 ]; then build_mode="Debug"; fi
if [ $release -eq 1 ]; then build_mode="Release"; fi

# Get the proper binary directory based on build inputs (goes compiler -> platform -> mode -> executable)
if [ $clang -eq 1 ]; then bin_path="$bin_path/clang"; fi

bin_path="$bin_path/$aarch"

if [ $debug -eq 1 ]; then bin_path="$bin_path/debug"; fi
if [ $release -eq 1 ]; then bin_path="$bin_path/release"; fi

if [ ! -d $bin_path ]; then
    mkdir -p $bin_path
fi

# Clang flags
clang_warnings="-Werror -Wall -Wno-implicit-function-declaration"
clang_flags="-g -fdiagnostics-absolute-paths -arch $aarch -I$base_path $clang_warnings"

clang_debug_flags="-O0 $clang_flags"
clang_release_flags="-O3 $clang_flags"
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
    # compile_std+=("-std=c89 -ansi")
    # compile_tests+=("$test_path/ak_atomic_compile_test.c")
	# compile_output+=("ak_atomic_c89_compile_test")

    # compile_std+=("-std=c99")
    # compile_tests+=("$test_path/ak_atomic_compile_test.c")
	# compile_output+=("ak_atomic_c99_compile_test")

    # compile_std+=("-std=c11")
    # compile_tests+=("$test_path/ak_atomic_compile_test.c")
	# compile_output+=("ak_atomic_c11_compile_test")

    # compile_std+=("-std=c17")
    # compile_tests+=("$test_path/ak_atomic_compile_test.c")
	# compile_output+=("ak_atomic_c17_compile_test")

    # compile_std+=("-std=c++98 -lc++")
    # compile_tests+=("$test_path/ak_atomic_compile_test.cpp")
	# compile_output+=("ak_atomic_cpp98_compile_test")

    # compile_std+=("-std=c++11 -lc++")
    # compile_tests+=("$test_path/ak_atomic_compile_test.cpp")
	# compile_output+=("ak_atomic_cpp11_compile_test")

    # compile_std+=("-std=c++14 -lc++")
    # compile_tests+=("$test_path/ak_atomic_compile_test.cpp")
	# compile_output+=("ak_atomic_cpp14_compile_test")

    # compile_std+=("-std=c++17 -lc++")
    # compile_tests+=("$test_path/ak_atomic_compile_test.cpp")
	# compile_output+=("ak_atomic_cpp17_compile_test")

    # compile_std+=("-std=c++20 -lc++")
    # compile_tests+=("$test_path/ak_atomic_compile_test.cpp")
	# compile_output+=("ak_atomic_cpp20_compile_test")
    
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

    compile_std+=("-std=c++98 -lc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp98_unit_test")

    compile_std+=("-std=c++11 -lc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp11_unit_test")

    compile_std+=("-std=c++14 -lc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp14_unit_test")

    compile_std+=("-std=c++17 -lc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp17_unit_test")

    compile_std+=("-std=c++20 -lc++")
    compile_tests+=("$test_path/ak_atomic_unit_test.cpp")
	compile_output+=("ak_atomic_cpp20_unit_test")
fi

# Build all the tests

echo "Compiler: $compiler $aarch"
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