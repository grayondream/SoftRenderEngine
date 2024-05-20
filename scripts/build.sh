root_dir=$(git rev-parse --show-toplevel)

cd $root_dir

cmake -S . -B build

cd build

make -j32

cd -
