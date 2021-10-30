set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-head" "linux" "x86_64" "debug"
define_build_folders

echo "\n\n=== Generating dart bindings ==="

mkdir -p ${TMP_BUILD_FOLDER}/dart
cd ${TMP_BUILD_FOLDER}/dart
cp ../${RELATIVE_SOURCE}/src/dart/pubspec.yaml ./
cp ../${RELATIVE_SOURCE}/src/dart/conf-build-workspace.yaml ./
dart pub get --offline
dart run ffigen --config conf-build-workspace.yaml
mkdir -p ${ROOT}/${BUILD_FOLDER}/lib
cp gitkebab_lib.dart ${ROOT}/${BUILD_FOLDER}/lib

print_done
