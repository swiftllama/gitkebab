set -e
echo "--- Deleting Pcre 8.45 Build for Linux / Debug ---"

BUILD_FOLDER=build/pcre-8.45/linux/debug
TMP_BUILD_FOLDER=build/tmp/pcre-8.45/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
