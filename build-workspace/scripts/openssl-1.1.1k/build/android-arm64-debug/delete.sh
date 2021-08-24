set -e
echo "--- Deleting OpenSSL 1.1.1k Build for Android/arm64/Debug ---"

BUILD_FOLDER=build/openssl-1.1.1k/android/arm64-debug
TMP_BUILD_FOLDER=build/tmp/openssl-1.1.1k/android/arm64-debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
