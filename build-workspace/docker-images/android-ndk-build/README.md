
# Android build NDK

A docker image that includes the NDK for android builds. It is based
on `rikorose/gcc-cmake`.

The build script `build-docker-image.sh` currently expects the android
ndk to be available in `resources/android-ndk-r23-linux.zip`. This
file is not included in the repository and must be downloaded
separately before building the docker image, for example from here:

  https://developer.android.com/ndk/downloads
  
  https://dl.google.com/android/repository/android-ndk-r23-linux.zip
  
It installs the android ndk to:

    /opt/android-ndk/android-ndk-r23

and has `cmake` in the path (currently version `3.21`).

Most tools are present at:

    /opt/android-ndk/android-ndk-r23/toolchains/llvm/prebuilt/linux-x86_64/bin/  

To build the docker image:

    ./build-docker-image.sh
    
To run a command using the docker image:

    docker run -it android-ndk-build <command>
    
To get a prompt in the docker container:

    docker run -it android-ndk-build /bin/bash

