set -e
echo "\n\n=== Generating dart bindings ==="

ROOT=${PWD}
BUILD_FOLDER=build/gitkebab-head/linux/debug
TMP_BUILD_FOLDER=build/tmp/gitkebab-head/linux/debug

set -x
mkdir -p ${TMP_BUILD_FOLDER}/dart
cd ${TMP_BUILD_FOLDER}/dart
cp ../../../../../../source/gitkebab-head/src/dart/pubspec.yaml ./
cp ../../../../../../source/gitkebab-head/src/dart/conf-build-workspace.yaml ./
PATH=/usr/bin/ dart pub get --offline
PATH=/usr/bin/ dart run ffigen --config conf-build-workspace.yaml
mkdir -p ${ROOT}/${BUILD_FOLDER}/lib
cp gitkebab_lib.dart ${ROOT}/${BUILD_FOLDER}/lib
