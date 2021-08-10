echo "--- Checking Libgit2 1.1.1 build (Linux/Debug) ---"

echo "Working directory: ${PWD}"
if [ -f "./build/libgit2-1.1.1/linux/debug/lib/libgit2.a" ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


