echo "--- Checking Libssh2 1.9.0 build (Linux/Debug) ---"

SENTINEL="./build/libssh2-1.9.0/linux/debug/lib/libssh2.a"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


