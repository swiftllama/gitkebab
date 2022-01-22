set -e
echo "--- Downloading Cmocka 1.1.5 Source ---"

set -x
rm -rf source/cmocka-1.1.5
rm -rf source/tmp/cmocka
mkdir -p source/tmp/cmocka
cd source/tmp/cmocka
curl -O https://cmocka.org/files/1.1/cmocka-1.1.5.tar.xz
tar xf cmocka-1.1.5.tar.xz
mv cmocka-1.1.5 ../../
cd ../../../
rm -rf source/tmp/cmocka

set +x
echo "--- DONE ---"
