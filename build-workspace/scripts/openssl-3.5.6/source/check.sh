echo "--- Checking Openssl 3.5.6 Source ---"

SENTINEL="./source/openssl-3.5.6/.git"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -d $SENTINEL ]
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1
