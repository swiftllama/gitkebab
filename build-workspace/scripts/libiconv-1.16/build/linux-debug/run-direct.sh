set -e
echo "--- Building Libiconv 1.16 for Linux / Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/libiconv-1.16/linux/debug
TMP_BUILD_FOLDER=build/tmp/libiconv-1.16/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

../../../../../source/libiconv-1.16/configure --enable-static --prefix=${ROOT}/${BUILD_FOLDER} 
make
make install

set +x
echo "--- DONE ---"
