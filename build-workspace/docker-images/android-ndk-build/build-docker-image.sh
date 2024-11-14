echo "=== Building Docker Image ==="
set -x
NDK_VERSION=r23
DOCKER_IMAGE_VERSION=1
docker build . --tag "android-ndk-build:${DOCKER_IMAGE_VERSION}.${NDK_VERSION}" --tag "android-ndk-build:latest" 
echo "=== Done ==="
