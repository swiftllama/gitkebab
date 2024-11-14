FROM rikorose/gcc-cmake

RUN mkdir -p /opt/android-ndk
WORKDIR /opt/android-ndk

ENV ANDROID_NDK /opt/android-ndk
ENV ANDROID_NDK_HOME /opt/android-ndk

ADD resources/android-ndk-r23-linux.zip ./android-ndk-r23-linux.zip
RUN unzip android-ndk-r23-linux.zip && rm -rf android-ndk-r23-linux.zip
