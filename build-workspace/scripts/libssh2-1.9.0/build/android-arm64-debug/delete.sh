set -e
echo "--- Deleting Libssh2 1.9.0 Build for Android/arm64/Debug ---"

BUILD_FOLDER=build/libssh2-1.9.0/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/libssh2-1.9.0/android/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
