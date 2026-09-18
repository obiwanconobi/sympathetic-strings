#!/bin/bash
#
# Build Sympathetic.component without Xcode's build system.
#
# The BUILD_MAC.md runbook targets the AirWindows Xcode project. That project
# cannot be driven from the command line on a machine where the Xcode license
# has not been accepted (xcodebuild is gated). This script performs the same
# build steps with clang directly:
#   1. compile the Apple CoreAudio Utility Classes (the "CA_SDK") into a
#      static library,
#   2. compile the Sympathetic AU wrapper and link it against that library,
#   3. assemble the .component bundle and copy it into place.
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CA_SDK="$SCRIPT_DIR/CA_SDK"
SRC="$SCRIPT_DIR/Sympathetic"
BUILD="$SCRIPT_DIR/build"
OBJ="$BUILD/obj"
PRODUCT="Sympathetic"
BUNDLE="$BUILD/$PRODUCT.component"

DEV="$(xcode-select -p)"
SDK="$DEV/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
CXX="$DEV/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
AR="$DEV/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar"

if [ ! -d "$SDK" ]; then
	echo "macOS SDK not found at $SDK" >&2
	exit 1
fi

ARCH_FLAGS="-arch arm64"
COMMON_FLAGS=(-std=c++14 -O2 -fPIC -isysroot "$SDK" -mmacosx-version-min=11.0
	$ARCH_FLAGS -Wno-deprecated-declarations -Wno-unused-private-field
	-Wno-unused-parameter)

CAU_INCLUDES=(
	-I"$CA_SDK/CoreAudio/PublicUtility"
	-I"$CA_SDK/CoreAudio/AudioUnits/AUPublic"
	-I"$CA_SDK/CoreAudio/AudioUnits/AUPublic/AUBase"
	-I"$CA_SDK/CoreAudio/AudioUnits/AUPublic/OtherBases"
	-I"$CA_SDK/CoreAudio/AudioUnits/AUPublic/Utility"
	-I"$CA_SDK/CoreAudio/AudioUnits/AUPublic/AUInstrumentBase"
	-I"$CA_SDK/CoreAudio/AudioFile/AFPublic"
	-I"$CA_SDK/CoreAudio/AudioCodecs/ACPublic"
)

CAU_SOURCES=(
	CoreAudio/PublicUtility/AUOutputBL.cpp
	CoreAudio/PublicUtility/AUParamInfo.cpp
	CoreAudio/PublicUtility/CAAudioBufferList.cpp
	CoreAudio/PublicUtility/CAAudioChannelLayout.cpp
	CoreAudio/PublicUtility/CAAudioChannelLayoutObject.cpp
	CoreAudio/PublicUtility/CAAudioFileFormats.cpp
	CoreAudio/PublicUtility/CAAudioTimeStamp.cpp
	CoreAudio/PublicUtility/CAAudioUnit.cpp
	CoreAudio/PublicUtility/CAAudioValueRange.cpp
	CoreAudio/PublicUtility/CAAUMIDIMap.cpp
	CoreAudio/PublicUtility/CAAUMIDIMapManager.cpp
	CoreAudio/PublicUtility/CAAUParameter.cpp
	CoreAudio/PublicUtility/CAAUProcessor.cpp
	CoreAudio/PublicUtility/CABufferList.cpp
	CoreAudio/PublicUtility/CABundleLocker.cpp
	CoreAudio/PublicUtility/CACFArray.cpp
	CoreAudio/PublicUtility/CACFDictionary.cpp
	CoreAudio/PublicUtility/CACFDistributedNotification.cpp
	CoreAudio/PublicUtility/CACFMachPort.cpp
	CoreAudio/PublicUtility/CACFMessagePort.cpp
	CoreAudio/PublicUtility/CACFNumber.cpp
	CoreAudio/PublicUtility/CACFPreferences.cpp
	CoreAudio/PublicUtility/CACFString.cpp
	CoreAudio/PublicUtility/CAComponent.cpp
	CoreAudio/PublicUtility/CAComponentDescription.cpp
	CoreAudio/PublicUtility/CADebugger.cpp
	CoreAudio/PublicUtility/CADebugMacros.cpp
	CoreAudio/PublicUtility/CADebugPrintf.cpp
	CoreAudio/PublicUtility/CAFilePathUtils.cpp
	CoreAudio/PublicUtility/CAGuard.cpp
	CoreAudio/PublicUtility/CAHALAudioDevice.cpp
	CoreAudio/PublicUtility/CAHALAudioObject.cpp
	CoreAudio/PublicUtility/CAHALAudioStream.cpp
	CoreAudio/PublicUtility/CAHALAudioSystemObject.cpp
	CoreAudio/PublicUtility/CAHostTimeBase.cpp
	CoreAudio/PublicUtility/CAMutex.cpp
	CoreAudio/PublicUtility/CAPersistence.cpp
	CoreAudio/PublicUtility/CAProcess.cpp
	CoreAudio/PublicUtility/CAPThread.cpp
	CoreAudio/PublicUtility/CARingBuffer.cpp
	CoreAudio/PublicUtility/CASettingsStorage.cpp
	CoreAudio/PublicUtility/CASharedLibrary.cpp
	CoreAudio/PublicUtility/CASpectralProcessor.cpp
	CoreAudio/PublicUtility/CAStreamBasicDescription.cpp
	CoreAudio/PublicUtility/CAStreamRangedDescription.cpp
	CoreAudio/PublicUtility/CAVectorUnit.cpp
	CoreAudio/PublicUtility/CAVolumeCurve.cpp
	CoreAudio/PublicUtility/CAXException.cpp
	CoreAudio/PublicUtility/MatrixMixerVolumes.cpp
	CoreAudio/AudioUnits/AUPublic/AUBase/AUBase.cpp
	CoreAudio/AudioUnits/AUPublic/AUBase/AUDispatch.cpp
	CoreAudio/AudioUnits/AUPublic/AUBase/AUInputElement.cpp
	CoreAudio/AudioUnits/AUPublic/AUBase/AUOutputElement.cpp
	CoreAudio/AudioUnits/AUPublic/AUBase/AUPlugInDispatch.cpp
	CoreAudio/AudioUnits/AUPublic/AUBase/AUScopeElement.cpp
	CoreAudio/AudioUnits/AUPublic/AUBase/ComponentBase.cpp
	CoreAudio/AudioUnits/AUPublic/AUInstrumentBase/AUInstrumentBase.cpp
	CoreAudio/AudioUnits/AUPublic/AUInstrumentBase/SynthElement.cpp
	CoreAudio/AudioUnits/AUPublic/AUInstrumentBase/SynthNote.cpp
	CoreAudio/AudioUnits/AUPublic/AUInstrumentBase/SynthNoteList.cpp
	CoreAudio/AudioUnits/AUPublic/OtherBases/AUEffectBase.cpp
	CoreAudio/AudioUnits/AUPublic/OtherBases/AUMIDIBase.cpp
	CoreAudio/AudioUnits/AUPublic/OtherBases/AUMIDIEffectBase.cpp
	CoreAudio/AudioUnits/AUPublic/OtherBases/AUOutputBase.cpp
	CoreAudio/AudioUnits/AUPublic/OtherBases/AUPannerBase.cpp
	CoreAudio/AudioUnits/AUPublic/OtherBases/MusicDeviceBase.cpp
	CoreAudio/AudioUnits/AUPublic/Utility/AUBaseHelper.cpp
	CoreAudio/AudioUnits/AUPublic/Utility/AUBuffer.cpp
	CoreAudio/AudioUnits/AUPublic/Utility/AUTimestampGenerator.cpp
	CoreAudio/AudioFile/AFPublic/AudioFileComponentBase.cpp
	CoreAudio/AudioFile/AFPublic/AudioFileFormat.cpp
	CoreAudio/AudioFile/AFPublic/AudioFileObject.cpp
	CoreAudio/AudioFile/AFPublic/CompressedPacketTable.cpp
	CoreAudio/AudioFile/AFPublic/DataSource.cpp
	CoreAudio/AudioCodecs/ACPublic/ACBaseCodec.cpp
	CoreAudio/AudioCodecs/ACPublic/ACCodec.cpp
	CoreAudio/AudioCodecs/ACPublic/ACPlugInDispatch.cpp
	CoreAudio/AudioCodecs/ACPublic/ACSimpleCodec.cpp
	CoreAudio/AudioCodecs/ACPublic/GetCodecBundle.cpp
)

mkdir -p "$OBJ"
rm -f "$BUILD/libCoreAudioUtilityClasses.a"

echo "==> Compiling CoreAudio Utility Classes"
for rel in "${CAU_SOURCES[@]}"; do
	src="$CA_SDK/$rel"
	obj="$OBJ/$(echo "$rel" | tr '/' '_').o"
	if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
		"$CXX" "${COMMON_FLAGS[@]}" "${CAU_INCLUDES[@]}" -c "$src" -o "$obj"
	fi
done
"$AR" rcs "$BUILD/libCoreAudioUtilityClasses.a" "$OBJ"/*.o

echo "==> Compiling Sympathetic AU"
"$CXX" "${COMMON_FLAGS[@]}" "${CAU_INCLUDES[@]}" -I"$SRC" -c "$SRC/Sympathetic.cpp" -o "$OBJ/Sympathetic.o"

echo "==> Compiling Sympathetic Cocoa view"
"$CXX" "${COMMON_FLAGS[@]}" -fobjc-arc -I"$SRC" -c "$SRC/SympatheticView.mm" -o "$OBJ/SympatheticView.o"

echo "==> Linking bundle"
rm -rf "$BUNDLE"
mkdir -p "$BUNDLE/Contents/MacOS"
"$CXX" -O2 -isysroot "$SDK" -mmacosx-version-min=11.0 $ARCH_FLAGS \
	-bundle -Wl,-exported_symbols_list,"$SRC/Sympathetic.exp" \
	-o "$BUNDLE/Contents/MacOS/$PRODUCT" \
	"$OBJ/Sympathetic.o" "$OBJ/SympatheticView.o" "$BUILD/libCoreAudioUtilityClasses.a" \
	-framework AudioUnit -framework AudioToolbox -framework CoreAudio \
	-framework CoreFoundation -framework CoreServices -framework CoreMIDI \
	-framework Accelerate -framework Cocoa

cp "$SRC/Info.plist" "$BUNDLE/Contents/Info.plist"
printf 'BNDL????' > "$BUNDLE/Contents/PkgInfo"

echo "==> Built $BUNDLE"
