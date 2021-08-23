set -e
echo "--- Deleting Libgit2 1.1.1 Build for Linux / Debug ---"

BUILD_FOLDER=build/libgit2-1.1.1/linux/debug
TMP_BUILD_FOLDER=build/tmp/libgit2-1.1.1/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
