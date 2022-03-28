echo "--- Checking Libssh2 1.10.0 Source ---"

SENTINEL="./source/libssh2-1.10.0/.git"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -d $SENTINEL ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


