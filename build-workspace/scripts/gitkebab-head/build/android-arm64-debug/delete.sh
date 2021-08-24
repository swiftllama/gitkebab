set -e
echo "--- Deleting GitKebab Head Build for Android/arm64/Debug ---"

BUILD_FOLDER=build/gitkebab-head/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/gitkebab-head/android/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
