echo "--- Checking Pcre2 10.37 build (Linux/Debug) ---"

echo "Working directory: ${PWD}"
if [ -f "./build/pcre2-10.37/linux/debug/lib/libpcre2-posix.a" ]
then
    echo "Build seems to exist"
    exit 0
fi

echo "Build does not exist"
exit 1


