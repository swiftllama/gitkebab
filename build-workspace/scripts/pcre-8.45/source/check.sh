echo "--- Checking Pcre 8.45 Source ---"

SENTINEL="./source/pcre-8.45/pcre.h.in"
echo "Working directory: ${PWD}"
echo "Sentinel: $SENTINEL"

if [ -f $SENTINEL ] 
then
    echo "Source code seems to exist"
    exit 0
fi

echo "Source code does not exist"
exit 1


