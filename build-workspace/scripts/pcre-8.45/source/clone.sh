set -e
echo "--- Downloading Pcre 8.45 Source ---"

set -x
rm -rf source/pcre-8.45
rm -rf source/tmp/pcre
mkdir -p source/tmp/pcre
cd source/tmp/pcre
#curl -O https://ftp.pcre.org/pub/pcre/pcre-8.45.zip
curl -L https://sourceforge.net/projects/pcre/files/pcre/8.45/pcre-8.45.zip/download -o pcre-8.45.zip
unzip pcre-8.45.zip
mv pcre-8.45 ../../
cd ../../../
rm -rf source/tmp/pcre

set +x
echo "--- DONE ---"
