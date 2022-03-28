set -e
echo "--- Cloning Openssl 1.1.1n Source ---"

set -x
rm -rf source/openssl-1.1.1n
mkdir -p source
git clone --branch OpenSSL_1_1_1n --depth 1 git@github.com:openssl/openssl.git source/openssl-1.1.1n

#echo "Applying patch"
#cd source/openssl-1.1.1n

set +x
echo "--- DONE ---"
