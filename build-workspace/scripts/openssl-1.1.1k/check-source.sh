echo "--- Checking Openssl 1.1.1k Source ---"

echo "Working directory: ${PWD}"
if [ -d "./source/openssl-1.1.1k/.git" ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


