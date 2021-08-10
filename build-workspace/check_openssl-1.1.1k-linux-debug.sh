echo "--- Checking Openssl 1.1.1k build (Linux/Debug) ---"

echo "Working directory: ${PWD}"
if [ -f "./build/openssl-1.1.1k/linux/debug/lib/libssl.a" ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


