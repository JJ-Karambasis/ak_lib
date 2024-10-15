build_path=$(dirname "$(realpath "$0")")

arch=""
compiler=""
build_mode=""
ndk_path=""
is_android_build=0
while [[ $# -gt 0 ]]; do
  case $1 in
    -android)
        is_android_build=1
        shift # past argument
        ;;
    -ndk)
      ndk_path="$2"
      shift # past argument
      shift # past value
      ;;
    -arch)
      arch="$2"
      shift # past argument
      shift # past value
      ;;
    -mode)
      build_mode="$2"
      shift # past argument
      shift # past value
      ;;
    -compiler)
      compiler="$2"
      shift # past argument
      shift # past value
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      shift # past argument
      ;;
  esac
done

if [ $is_android_build -eq 1 ]; then
    /bin/bash $build_path/internal/build_android.sh $ndk_path $build_mode $arch
    exit 0
fi

/bin/bash $build_path/internal/build_desktop.sh $compiler $build_mode $arch