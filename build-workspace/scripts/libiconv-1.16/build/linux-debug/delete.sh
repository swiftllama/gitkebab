set -e
echo "--- Deleting Libiconv 1.16 Build for Linux / Debug ---"

BUILD_FOLDER=build/libiconv-1.16/linux/debug
TMP_BUILD_FOLDER=build/tmp/libiconv-1.16/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
