# Common global variables
release=0
debug=0
clang=1 # Clang is the only supported compiler right now

build_path=$(dirname "$(realpath "$0")")
base_path="$build_path/.."
bin_path="$base_path/bin/ak_atomic"

# Get the architecture of the computer
aarch=$(uname -m)

# Build inputs and validation
for var in "$@"
do
    if [ "$var" = "release" ]; then release=1; fi
    if [ "$var" = "debug" ]; then debug=1; fi
done

if [ ! $release -eq 1 ]; then debug=1; fi

# Get the proper binary directory based on build inputs (goes compiler -> platform -> mode -> executable)
if [ $clang -eq 1 ]; then bin_path="$bin_path/clang"; fi

bin_path="$bin_path/$aarch"

if [ $debug -eq 1 ]; then bin_path="$bin_path/debug"; fi
if [ $release -eq 1 ]; then bin_path="$bin_path/release"; fi

paths=(`find "$bin_path" -perm -o=x,-g=x,-u=x -type f -name "*_unit_test"`)
for i in "${paths[@]}"; do
    echo "Running $(basename $i):"
    $i
    echo 
done