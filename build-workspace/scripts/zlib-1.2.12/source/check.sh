echo "--- Checking Zlib 1.2.12 Source ---"

SENTINEL="./source/zlib-1.2.12/.git"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -d $SENTINEL ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


