build_path=$(dirname "$(realpath "$0")")
base_path="$build_path/../../.."
test_path="$base_path/tests/ak_atomic"

echo "$test_path"

ndk_path=$1
build_mode=$2
arch=$3

if [ $build_mode != "release" ]; then
    if [ $build_mode != "debug" ]; then
        echo "Invalid build mode: $build_mode. Must be either 'debug' or 'release'"
        exit 1
    fi
fi

if [ $arch == "all" ]; then
	build_x86=1
	build_x64=1
	build_armv7=1
	build_aarch64=1
fi

if [ $arch != "all" ]; then
    is_valid=0
    if [ $arch == "x86" ]; then 
        build_x86=1
        is_valid=1
    fi

    if [ $arch == "x64" ]; then 
        build_x64=1
        is_valid=1
    fi

    if [ $arch == "armv7" ]; then 
        build_armv7=1
        is_valid=1
    fi

    if [ $arch == "aarch64" ]; then
        build_aarch64=1
        is_valid=1
    fi

    if [ $is_valid -eq 0 ]; then
	    echo "Invalid architecture: $arch. Must be 'all', 'x86', 'x64', 'armv7', or 'aarch64'"
	    exit 1
    fi
fi

if [ $build_mode == "release" ]; then
	output_path_unit_test="$build_path/android/ak_atomic_unit_test/src/main/jniLibs"
fi

if [ $build_mode == "debug" ]; then
	output_path_unit_test="$build_path/android/ak_atomic_unit_test/src/debug/jniLibs"
fi

ndk_native_app_glue_path="$ndk_path/sources/android/native_app_glue"
ndk_clang_path=""
sysroot=""

machine="none"
if [ "$(uname)" == "Darwin" ]; then
    ndk_clang_path="$ndk_path/toolchains/llvm/prebuilt/darwin-x86_64/bin"
    sysroot="$ndk_path/toolchains/llvm/prebuilt/darwin-x86_64/sysroot"  
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    ndk_clang_path="$ndk_path/toolchains/llvm/prebuilt/linux-x86_64/bin"
    sysroot="$ndk_path/toolchains/llvm/prebuilt/linux-x86_64/sysroot"
else 
    echo "Invalid platform for android!"
    exit 1
fi

clang_warnings="-Werror -Wall"
clang_flags="-g -fdiagnostics-absolute-paths -fPIC -std=c99 --sysroot=$sysroot -I$ndk_native_app_glue_path -I$base_path -I$test_path $clang_warnings"

clang_debug_flags="-O0 -DAK_ATOMIC_DEBUG_BUILD $clang_flags"
clang_release_flags="-O3 $clang_flags"
clang_link="-llog -landroid"
clang_out="-o"

if [ $build_mode == "release" ]; then
	compile_flags=$clang_release_flags
fi

if [ $build_mode == "debug" ]; then
	output_path_unit_test=$clang_debug_flags
fi

compile_link=$clang_link
compile_out=$clang_out

android_glue_flags="$compile_flags -c -Wno-comment $ndk_native_app_glue_path/android_native_app_glue.c $compile_out android_native_app_glue.o"
android_app_flags="$compile_flags -shared $build_path/android/ak_atomic_unit_test/src/main/cpp/ak_atomic_unit_test_android.c android_native_app_glue.o $compile_link -u ANativeActivity_onCreate $compile_out libnative-activity.so"

echo "Building Android Unit Tests"
echo "Mode: $build_mode"
echo "Compile Flags: $compile_flags"
echo "Link Flags: $compile_link"
echo "Output Path: $output_path_unit_test"

if [ $build_armv7 -eq 1 ]; then
	if [ ! -d "$output_path_unit_test/armeabi-v7a" ]; then
		mkdir -p "$output_path_unit_test/armeabi-v7a"
	fi

    pushd "$output_path_unit_test/armeabi-v7a"
        $ndk_clang_path/armv7a-linux-androideabi21-clang $android_glue_flags
        $ndk_clang_path/armv7a-linux-androideabi21-clang $android_app_flags
    popd

    rm $output_path_unit_test/armeabi-v7a/*.o
fi

if [ $build_aarch64 -eq 1 ]; then
	if [ ! -d "$output_path_unit_test/arm64-v8a" ]; then
		mkdir -p "$output_path_unit_test/arm64-v8a"
	fi

    pushd "$output_path_unit_test/arm64-v8a"
        $ndk_clang_path/aarch64-linux-android21-clang $android_glue_flags
        $ndk_clang_path/aarch64-linux-android21-clang $android_app_flags
    popd

    rm $output_path_unit_test/arm64-v8a/*.o
fi

if [ $build_x86 -eq 1 ]; then
	if [ ! -d "$output_path_unit_test/x86" ]; then
		mkdir -p "$output_path_unit_test/x86"
	fi

    pushd "$output_path_unit_test/x86"
        $ndk_clang_path/i686-linux-android21-clang $android_glue_flags
        $ndk_clang_path/i686-linux-android21-clang $android_app_flags
    popd

    rm $output_path_unit_test/x86/*.o
fi

if [ $build_x64 -eq 1 ]; then
	if [ ! -d "$output_path_unit_test/x86_64" ]; then
		mkdir -p "$output_path_unit_test/x86_64"
	fi

    pushd "$output_path_unit_test/x86_64"
        $ndk_clang_path/x86_64-linux-android21-clang $android_glue_flags
        $ndk_clang_path/x86_64-linux-android21-clang $android_app_flags
    popd

    rm $output_path_unit_test/x86_64/*.o
fi