set -e
echo "--- Copying GitKebab 0.8.0 Source ---"

VERSION=0.8.0
GK="gitkebab-$VERSION"

set -x
rm -rf source/$GK ./source/tmp/$GK
git clone ../ ./source/tmp/$GK
cd ./source/tmp/$GK
git checkout $VERSION
cd ../../../

mkdir source/$GK
mv ./source/tmp/$GK/src source/$GK/src
mv ./source/tmp/$GK/CMakeLists.txt source/$GK/
rm -rf ./source/tmp/$GK
sed -i'' s/"develop"/"$VERSION"/ source/$GK/src/lib/gk_init.c

set +x
echo "--- DONE ---"
