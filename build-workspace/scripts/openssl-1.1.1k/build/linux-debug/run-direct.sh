set -e
echo "--- Building OpenSSL 1.1.1k for Linux / Debug ---"

ROOT=${PWD}
BUILD_FOLDER=build/openssl-1.1.1k/linux/debug
TMP_BUILD_FOLDER=build/tmp/openssl-1.1.1k/linux/debug

set -x
rm -rf ${BUILD_FOLDER}
rm -rf ${TMP_BUILD_FOLDER}

mkdir -p ${BUILD_FOLDER}
mkdir -p ${TMP_BUILD_FOLDER}
cd ${TMP_BUILD_FOLDER}

## NOTES
##  - allow linking resulting static library against shared library later on
##    - "-fPIC"
##

../../../../../source/openssl-1.1.1k/Configure linux-x86_64 --prefix=${ROOT}/${BUILD_FOLDER} --openssldir=${ROOT}/${BUILD_FOLDER} -fPIC
make
make install

set +x
echo "--- DONE ---"
