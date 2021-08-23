set -e
echo "--- Building Pcre 8.45 for Linux / Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/pcre-8.45/linux/debug
TMP_BUILD_FOLDER=build/tmp/pcre-8.45/linux/debug

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

../../../../../source/pcre-8.45/configure --prefix=${ROOT}/${BUILD_FOLDER} CFLAGS=-fPIC CXXFLAGS="-fPIC"

make
make install

set +x
echo "--- DONE ---"
