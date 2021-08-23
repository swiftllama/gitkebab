echo "--- Checking Pcre 8.45 Source ---"

echo "Working directory: ${PWD}"
set -x
if [ -f "./source/pcre-8.45/pcre.h.in" ] 
then
    echo "Source code seems to exist"
    exit 0
fi

set +x
echo "Source code does not exist"
exit 1


