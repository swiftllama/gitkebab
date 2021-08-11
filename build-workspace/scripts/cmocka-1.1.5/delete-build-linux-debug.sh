set -e
echo "--- Deleting Cmocka 1.1.5 Build for Linux / Debug ---"

BUILD_FOLDER=build/cmocka-1.1.5/linux/debug
TMP_BUILD_FOLDER=build/tmp/cmocka-1.1.5/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
