set -e
echo "--- Building Zlib 1.2.11 for Linux / Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/zlib-1.2.11/linux/debug
TMP_BUILD_FOLDER=build/tmp/zlib-1.2.11/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

## NOTES
##  - allow linking resulting static library against shared library later on
##    - CFLAGS=-fPIC
##

CFLAGS="-fPIC" ../../../../../source/zlib-1.2.11/configure --prefix=${ROOT}/${BUILD_FOLDER}
make
make install

set +x
echo "--- DONE ---"
