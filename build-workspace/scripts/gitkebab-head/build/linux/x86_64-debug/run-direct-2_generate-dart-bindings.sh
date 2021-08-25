set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-head" "linux" "x86_64" "debug"
init_and_change_into_tmp_build_folder

echo "\n\n=== Generating dart bindings ==="

mkdir -p dart
cd dart
cp ../${RELATIVE_SOURCE}/src/dart/pubspec.yaml ./
cp ../${RELATIVE_SOURCE}/src/dart/conf-build-workspace.yaml ./
PATH=/usr/bin/ dart pub get --offline
PATH=/usr/bin/ dart run ffigen --config conf-build-workspace.yaml
mkdir -p ${ROOT}/${BUILD_FOLDER}/lib
cp gitkebab_lib.dart ${ROOT}/${BUILD_FOLDER}/lib

print_done
