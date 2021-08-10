echo "--- Checking Libssh2 1.9.0 Source ---"

echo "Working directory: ${PWD}"
if [ -d "./source/libssh2-1.9.0/.git" ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


