echo "--- Checking Libgit2 1.9.4 Source ---"

SENTINEL="./source/libgit2-1.9.4/.git"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -d $SENTINEL ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


