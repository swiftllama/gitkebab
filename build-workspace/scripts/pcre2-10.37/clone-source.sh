set -e
echo "--- Downloading Pcre 10.37 Source ---"

set -x
rm -rf source/tmp/pcre
mkdir -p source/tmp/pcre
cd source/tmp/pcre
curl -O https://ftp.pcre.org/pub/pcre/pcre2-10.37.zip
unzip pcre2-10.37.zip
mv pcre2-10.37 ../../
cd ../../../
rm -rf source/tmp/pcre

set +x
echo "--- DONE ---"
