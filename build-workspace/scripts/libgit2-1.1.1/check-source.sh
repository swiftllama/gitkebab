echo "--- Checking Libgit2 1.1.1 Source ---"

echo "Working directory: ${PWD}"
if [ -d "./source/libgit2-1.1.1/.git" ] 
then
    echo "Repo seems to exist"
    exit 0
fi

echo "Repo does not exist"
exit 1


