build_path=$(dirname "$(realpath "$0")")
base_path="$build_path/../.."

arch=""
compiler=""
build_mode=""
while [[ $# -gt 0 ]]; do
  case $1 in
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

echo "Sup"

if [ $build_mode != "release" ]; then
    if [ $build_mode != "debug" ]; then
        echo "Invalid build mode: $build_mode. Must be either 'debug' or 'release'"
        exit 1
    fi
fi

echo "Sup"

# Binary path
bin_path="$base_path/bin/ak_atomic/$compiler/$arch/$build_mode"
if [ ! -d $bin_path ]; then
    mkdir -p $bin_path
fi

echo "Sup"

paths=(`find "$bin_path" -perm -o=x,-g=x,-u=x -type f -name "*_unit_test"`)
for i in "${paths[@]}"; do
    echo "Running $(basename $i):"
    $i
    echo 
done