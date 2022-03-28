echo "--- Checking Openssl 1.1.1n Source ---"

SENTINEL="./source/openssl-!1.1.1n/.git"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -d $SENTINEL ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


