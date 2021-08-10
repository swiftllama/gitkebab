echo "--- Checking Libssh2 1.9.0 build (Linux/Debug) ---"

echo "Working directory: ${PWD}"
if [ -f "./build/libssh2-1.9.0/linux/debug/lib/libssh2.a" ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


