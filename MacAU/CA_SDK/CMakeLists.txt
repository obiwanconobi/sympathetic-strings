# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.15)

project(CoreAudioUtilityClasses
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Apple CoreAudio Utility Classes"
)

if(NOT APPLE)
    message(FATAL_ERROR "CoreAudioUtilityClasses only supports macOS builds")
endif()

add_library(CoreAudioUtilityClasses STATIC
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

add_library(CoreAudio::UtilityClasses ALIAS CoreAudioUtilityClasses)

target_include_directories(CoreAudioUtilityClasses PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/CoreAudio/PublicUtility>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/CoreAudio/AudioUnits/AUPublic>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/CoreAudio/AudioUnits/AUPublic/AUBase>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/CoreAudio/AudioUnits/AUPublic/OtherBases>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/CoreAudio/AudioUnits/AUPublic/Utility>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/CoreAudio/AudioUnits/AUPublic/AUInstrumentBase>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/CoreAudio/AudioFile/AFPublic>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/CoreAudio/AudioCodecs/ACPublic>
)

target_link_libraries(CoreAudioUtilityClasses PUBLIC
    "-framework CoreAudio"
    "-framework AudioToolbox"
    "-framework CoreFoundation"
    "-framework AudioUnit"
)

target_compile_options(CoreAudioUtilityClasses PRIVATE
    -Wall
    -Wno-deprecated-declarations
    -Wno-unused-private-field
)

set_target_properties(CoreAudioUtilityClasses PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_STANDARD              14 # caveat: not compatible with newer c++ standard
)
