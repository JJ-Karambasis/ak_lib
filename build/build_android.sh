debug=0
release=0
Script=$(realpath "$0")
ScriptPath=$(dirname "$Script")

BasePath="$ScriptPath/.."

for var in "$@"
do
    if [ $var=release ]; then release=1; fi
    if [ $var=armv7a ]; then armv7a=1; fi
    if [ $var=aarch64 ]; then aarch64=1; fi
    if [ $var=x86 ]; then x86=1; fi
    if [ $var=x86_64 ]; then x86_64=1; fi
    if [ $var=all ]; then all=1; fi
done

if [ ! $release -eq 1 ]; then debug=1; fi

if [ "$(uname)" == "Darwin" ]; then
    osx=1        
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    linux=1
fi

if [ $all -eq 1 ]; then
    armv7a=1
    aarch64=1
    x86=1
    x86_64=1
fi

ANDROID_HOME="$HOME/Android/Sdk"
ndk_path="$ANDROID_HOME/ndk/25.2.9519653"
ndk_native_app_glue_path="$ndk_path/sources/android/native_app_glue"
lib_path="$BasePath/build/android/ak_atomic_test/src/main/jniLibs"

if [ $linux -eq 1 ]; then 
    ndk_clang_path="$ndk_path/toolchains/llvm/prebuilt/linux-x86_64/bin"
    ndk_sys_root="$ndk_path/toolchains/llvm/prebuilt/linux-x86_64/sysroot"
elif [ $osx -eq 1 ]; then
    echo Not Implemented
fi

linker_flags="-llog -landroid"

compile_warnings="-Werror -Wall"
compile_common=-"I$BasePath -I$ndk_native_app_glue_path --sysroot=$ndk_sys_root -DANDROID_BUILD -fdiagnostics-absolute-paths -fPIC -std=c89 $compile_warnings"
compile_debug="-g -O0 $compile_common"
compile_release="-g -O2 $compile_common"
compile_out="-o"
compile_shared_library="-shared"

if [ $release -eq 1 ]; then compile_flags=$compile_release; fi
if [ $debug -eq 1 ]; then compile_flags=$compile_debug; fi

android_glue_flags="$compile_flags -c -Wno-comment $ndk_native_app_glue_path/android_native_app_glue.c $compile_out android_native_app_glue.o"
android_app_flags="$compile_flags $compile_shared_library $linker_flags $BasePath/build/android/ak_atomic_test/src/main/cpp/ak_android_atomic_test.c android_native_app_glue.o -u ANativeActivity_onCreate $compile_out libnative-activity.so"

if [ $armv7a -eq 1 ]; then
    if [ ! -d "$lib_path/armeabi-v7a" ]; then mkdir -p "$lib_path/armeabi-v7a/"; fi 

    pushd "$lib_path/armeabi-v7a"
        $ndk_clang_path/armv7a-linux-androideabi19-clang $android_glue_flags
        $ndk_clang_path/armv7a-linux-androideabi19-clang $android_app_flags
    popd

    rm $lib_path/armeabi-v7a/*.o
fi

if [ $aarch64 -eq 1 ]; then
    if [ ! -d "$lib_path/arm64-v8a" ]; then mkdir -p "$lib_path/arm64-v8a/"; fi

    pushd "$lib_path/arm64-v8a"
        $ndk_clang_path/aarch64-linux-android21-clang $android_glue_flags
        $ndk_clang_path/aarch64-linux-android21-clang $android_app_flags
    popd

    rm $lib_path/arm64-v8a/*.o
fi

if [ $x86 -eq 1 ]; then
    if [ ! -d "$lib_path/x86" ]; then mkdir -p "$lib_path/x86/"; fi

    pushd "$lib_path/x86"
        $ndk_clang_path/aarch64-linux-android21-clang $android_glue_flags
        $ndk_clang_path/aarch64-linux-android21-clang $android_app_flags
    popd

    rm $lib_path/x86/*.o
fi

if [ $x86_64 -eq 1 ]; then
    if [ ! -d "$lib_path/x86_64" ]; then mkdir -p "$lib_path/x86_64/"; fi

    pushd "$lib_path/x86_64"
        $ndk_clang_path/aarch64-linux-android21-clang $android_glue_flags
        $ndk_clang_path/aarch64-linux-android21-clang $android_app_flags
    popd

    rm $lib_path/x86_64/*.o
fi