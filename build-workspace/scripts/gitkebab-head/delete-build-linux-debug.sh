set -e
echo "--- Deleting Pcre 8.45 Build for Linux / Debug ---"

BUILD_FOLDER=build/gitkebab-head/linux/debug
TMP_BUILD_FOLDER=build/tmp/gitkebab-head/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
