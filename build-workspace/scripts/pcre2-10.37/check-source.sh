echo "--- Checking pcre 10.37 Source ---"

echo "Working directory: ${PWD}"
set -x
if [ -f "./source/pcre2-10.37/src/pcre2.h.in" ] 
then
    echo "Source code seems to exist"
    exit 0
fi

set +x
echo "Source code does not exist"
exit 1


