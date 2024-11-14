set -e
source "scripts/common/bash/common.sh"
set -x

define_basic_variables "gitkebab-0.8.0" "ios" "universal" "debug"
init_and_change_into_tmp_build_folder

rm -rf ${ROOT}/${BUILD_FOLDER}
cp -PR ${ROOT}/${BUILD_FOLDER}_device ${ROOT}/${BUILD_FOLDER}

lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libgitkebab_static.a ${ROOT}/${BUILD_FOLDER}_simulator/lib/libgitkebab_static.a -output ${ROOT}/${BUILD_FOLDER}/lib/libgitkebab_static.a
lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libgitkebab.dylib ${ROOT}/${BUILD_FOLDER}_simulator/lib/libgitkebab.dylib -output ${ROOT}/${BUILD_FOLDER}/lib/libgitkebab.dylib
mkdir -p ${ROOT}/${BUILD_FOLDER}/lib/device/GitKebab.framework
mkdir -p ${ROOT}/${BUILD_FOLDER}/lib/simulator/GitKebab.framework

FRAMEWORK_NAME=GitKebab
FRAMEWORK_VERSION=1.1.0
FRAMEWORK_BUILD_NUMBER=1
MINIMUM_OS_VERSION=11.0

cat <<EOF >  ${ROOT}/${BUILD_FOLDER}/lib/device/GitKebab.framework/Info.plist
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleExecutable</key>
	<string>${FRAMEWORK_NAME}</string>
	<key>CFBundleIdentifier</key>
	<string>net.swiftllama.gitkebab</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>${FRAMEWORK_NAME}</string>
	<key>CFBundlePackageType</key>
	<string>FMWK</string>
	<key>CFBundleShortVersionString</key>
	<string>${FRAMEWORK_VERSION}</string>
	<key>CFBundleSignature</key>
	<string>????</string>
	<key>CFBundleVersion</key>
	<string>${FRAMEWORK_BUILD_NUMBER}</string>
	<key>MinimumOSVersion</key>
	<string>${MINIMUM_OS_VERSION}</string>
</dict>
</plist>
EOF

cp ${ROOT}/${BUILD_FOLDER}/lib/device/GitKebab.framework/Info.plist ${ROOT}/${BUILD_FOLDER}/lib/simulator/GitKebab.framework/Info.plist

lipo -create ${ROOT}/${BUILD_FOLDER}_device/lib/libgitkebab.dylib -output ${ROOT}/${BUILD_FOLDER}/lib/device/GitKebab.framework/GitKebab
install_name_tool -id @rpath/Frameworks/GitKebab.framework/GitKebab ${ROOT}/${BUILD_FOLDER}/lib/device/GitKebab.framework/GitKebab

lipo -create ${ROOT}/${BUILD_FOLDER}_simulator/lib/libgitkebab.dylib -output ${ROOT}/${BUILD_FOLDER}/lib/simulator/GitKebab.framework/GitKebab
install_name_tool -id @rpath/Frameworks/GitKebab.framework/GitKebab ${ROOT}/${BUILD_FOLDER}/lib/simulator/GitKebab.framework/GitKebab

xcrun xcodebuild -create-xcframework -framework ${ROOT}/${BUILD_FOLDER}/lib/device/GitKebab.framework -framework ${ROOT}/${BUILD_FOLDER}/lib/simulator/GitKebab.framework -output ${ROOT}/${BUILD_FOLDER}/lib/GitKebab.xcframework

print_done
