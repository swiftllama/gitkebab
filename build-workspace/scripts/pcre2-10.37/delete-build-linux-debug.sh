set -e
echo "--- Deleting Pcre2 10.37 Build for Linux / Debug ---"

BUILD_FOLDER=build/pcre2-10.37/linux/debug
TMP_BUILD_FOLDER=build/tmp/pcre2-10.37/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

set +x
echo "--- DONE ---"
