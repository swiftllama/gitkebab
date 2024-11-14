set -e
echo "--- Downloading Libiconv 1.16 Source ---"

set -x
rm -rf source/libiconv-1.16
rm -rf source/tmp/libiconv
mkdir -p source/tmp/libiconv
cd source/tmp/libiconv
curl -O https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.16.tar.gz
tar xzvf libiconv-1.16.tar.gz
mv libiconv-1.16 ../../
cd ../../../
rm -rf source/tmp/libiconv

set +x
echo "--- DONE ---"

