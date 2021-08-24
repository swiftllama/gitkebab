echo "--- Checking Libgit2 1.1.1 build (Android/arm64/Debug) ---"

SENTINEL="./build/libgit2-1.1.1/android/arm64-debug/lib/libgit2.a"
echo "Sentinel: $SENTINEL"
echo "Working directory: ${PWD}"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


