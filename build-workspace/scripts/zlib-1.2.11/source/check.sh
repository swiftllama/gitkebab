echo "--- Checking Zlib 1.2.11 Source ---"

echo "Working directory: ${PWD}"
if [ -d "./source/zlib-1.2.11/.git" ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


