echo "--- Checking Pcre 8.45 build (Linux/Debug) ---"

echo "Working directory: ${PWD}"
if [ -f "./build/pcre-8.45/linux/debug/lib/libpcre.a" ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


