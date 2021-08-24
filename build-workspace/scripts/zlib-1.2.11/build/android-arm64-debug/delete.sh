set -e
echo "--- Deleting Zlib 1.2.11 Build for android/arm64/Debug ---"

BUILD_FOLDER=build/zlib-1.2.11/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/zlib-1.2.11/android/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
