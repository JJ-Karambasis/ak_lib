build_path=$(dirname "$(realpath "$0")")
base_path="$build_path/.."
bin_path="$base_path/bin"

if [ ! -d $bin_path ]; then
    mkdir $bin_path
fi

${build_path}/build_atomic.sh "$@"