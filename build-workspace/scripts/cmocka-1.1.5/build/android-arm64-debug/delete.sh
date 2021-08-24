set -e
echo "--- Deleting Cmocka 1.1.5 Build for Android/arm64/Debug ---"

BUILD_FOLDER=build/cmocka-1.1.5/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/cmocka-1.1.5/android/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
