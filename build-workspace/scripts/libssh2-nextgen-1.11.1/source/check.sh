echo "--- Checking Libssh2 1.11.1 Source ---"

SENTINEL="./source/libssh2-nextgen-1.11.1/.git"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -d $SENTINEL ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


