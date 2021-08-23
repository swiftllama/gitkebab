set -e
echo "--- Deleting OpenSSL 1.1.1k Build for Linux / Debug ---"

BUILD_FOLDER=build/openssl-1.1.1k/linux/debug
TMP_BUILD_FOLDER=build/tmp/openssl-1.1.1k/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
