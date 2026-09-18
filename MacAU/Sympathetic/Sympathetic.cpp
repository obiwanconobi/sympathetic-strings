/*
 *	File:		Sympathetic.cpp
 *
 *	Version:	1.0
 *
 *	Copyright:  Copyright (c) 2026 Panaudio, Panaudio uses the MIT license
 *
 *	Audio Unit wrapper for the Sympathetic string-resonator effect.
 */

#include "Sympathetic.h"

#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AUDIOCOMPONENT_ENTRY(AUBaseFactory, Sympathetic)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Sympathetic::Sympathetic
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sympathetic::Sympathetic(AudioUnit component)
	: AUEffectBase(component)
{
	CreateElements();
	Globals()->UseIndexedParameters(kNumberOfParameters);
	SetParameter(kParam_Resonance, kDefaultValue_Resonance);
	SetParameter(kParam_Sustain,   kDefaultValue_Sustain);
	SetParameter(kParam_Damping,   kDefaultValue_Damping);
	SetParameter(kParam_Detune,    kDefaultValue_Detune);
	SetParameter(kParam_Bank,      kDefaultValue_Bank);
	SetParameter(kParam_DryWet,    kDefaultValue_DryWet);

#if AU_DEBUG_DISPATCHER
	mDebugDispatcher = new AUDebugDispatcher (this);
#endif
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Bank display names
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static CFStringRef SympatheticBankName(int bank)
{
	switch (bank) {
		case 0: return CFSTR("Standard");
		case 1: return CFSTR("Twelve");
		case 2: return CFSTR("DADGAD");
		case 3: return CFSTR("Dm drone");
		default: return CFSTR("");
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Bundle URL of this component, used to expose the Cocoa view class.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
static CFURLRef SympatheticBundleURL()
{
	Dl_info info;
	if (dladdr((const void *)&SympatheticBundleURL, &info) == 0 || info.dli_fname == NULL)
		return NULL;

	CFStringRef exePath = CFStringCreateWithCString(kCFAllocatorDefault, info.dli_fname, kCFStringEncodingUTF8);
	if (exePath == NULL) return NULL;
	CFURLRef exeURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, exePath, kCFURLPOSIXPathStyle, false);
	CFRelease(exePath);
	if (exeURL == NULL) return NULL;

	// .../Sympathetic.component/Contents/MacOS/Sympathetic -> .../Sympathetic.component
	CFURLRef macosURL = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, exeURL);
	CFURLRef contentsURL = macosURL ? CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, macosURL) : NULL;
	CFURLRef bundleURL = contentsURL ? CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, contentsURL) : NULL;

	CFRelease(exeURL);
	if (macosURL) CFRelease(macosURL);
	if (contentsURL) CFRelease(contentsURL);
	return bundleURL;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Sympathetic::GetParameterValueStrings
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult			Sympathetic::GetParameterValueStrings(AudioUnitScope		inScope,
																AudioUnitParameterID	inParameterID,
																CFArrayRef *			outStrings)
{
	// AUBase calls this twice: once with a NULL outStrings to discover the
	// property's size (GetPropertyInfo), and once with a real buffer.
	if (inScope != kAudioUnitScope_Global)
		return kAudioUnitErr_InvalidScope;

	if (inParameterID != kParam_Bank)
		return kAudioUnitErr_InvalidProperty;

	if (outStrings != NULL) {
		const void* valueStrings[kNumBanks];
		for (int i = 0; i < kNumBanks; i++)
			valueStrings[i] = SympatheticBankName(i);
		*outStrings = CFArrayCreate(NULL, valueStrings, kNumBanks, &kCFTypeArrayCallBacks);
	}

	return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Sympathetic::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult			Sympathetic::GetParameterInfo(AudioUnitScope		inScope,
														AudioUnitParameterID	inParameterID,
														AudioUnitParameterInfo	&outParameterInfo )
{
	ComponentResult result = noErr;

	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
						|		kAudioUnitParameterFlag_IsReadable;

	if (inScope == kAudioUnitScope_Global) {
		switch(inParameterID)
		{
			case kParam_Resonance:
				AUBase::FillInParameterName (outParameterInfo, kParameterResonanceName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = 1.0;
				outParameterInfo.defaultValue = kDefaultValue_Resonance;
				break;
			case kParam_Sustain:
				AUBase::FillInParameterName (outParameterInfo, kParameterSustainName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = 1.0;
				outParameterInfo.defaultValue = kDefaultValue_Sustain;
				break;
			case kParam_Damping:
				AUBase::FillInParameterName (outParameterInfo, kParameterDampingName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = 1.0;
				outParameterInfo.defaultValue = kDefaultValue_Damping;
				break;
			case kParam_Detune:
				AUBase::FillInParameterName (outParameterInfo, kParameterDetuneName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = 1.0;
				outParameterInfo.defaultValue = kDefaultValue_Detune;
				break;
			case kParam_Bank:
				AUBase::FillInParameterName (outParameterInfo, kParameterBankName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
				outParameterInfo.flags |= kAudioUnitParameterFlag_ValuesHaveStrings;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = kNumBanks - 1;
				outParameterInfo.defaultValue = kDefaultValue_Bank;
				break;
			case kParam_DryWet:
				AUBase::FillInParameterName (outParameterInfo, kParameterDryWetName, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = 1.0;
				outParameterInfo.defaultValue = kDefaultValue_DryWet;
				break;
			default:
				result = kAudioUnitErr_InvalidParameter;
				break;
		}
	} else {
		result = kAudioUnitErr_InvalidParameter;
	}

	return result;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Sympathetic::GetPropertyInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult			Sympathetic::GetPropertyInfo (AudioUnitPropertyID	inID,
														AudioUnitScope		inScope,
														AudioUnitElement	inElement,
														UInt32 &		outDataSize,
														Boolean &		outWritable)
{
	if (inID == kAudioUnitProperty_CocoaUI && inScope == kAudioUnitScope_Global) {
		outDataSize = sizeof(AudioUnitCocoaViewInfo);
		outWritable = false;
		return noErr;
	}
	if (inID == kAudioUnitProperty_ParameterStringFromValue && inScope == kAudioUnitScope_Global) {
		outDataSize = sizeof(AudioUnitParameterStringFromValue);
		outWritable = false;
		return noErr;
	}
	if (inID == kAudioUnitProperty_ParameterValueFromString && inScope == kAudioUnitScope_Global) {
		outDataSize = sizeof(AudioUnitParameterValueFromString);
		outWritable = false;
		return noErr;
	}
	return AUEffectBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Sympathetic::GetProperty
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult			Sympathetic::GetProperty(	AudioUnitPropertyID inID,
														AudioUnitScope 		inScope,
														AudioUnitElement 	inElement,
														void *			outData )
{
	if (inID == kAudioUnitProperty_CocoaUI && inScope == kAudioUnitScope_Global) {
		AudioUnitCocoaViewInfo* info = (AudioUnitCocoaViewInfo*)outData;
		info->mCocoaAUViewBundleLocation = SympatheticBundleURL();
		info->mCocoaAUViewClass[0] = (CFStringRef)CFRetain(CFSTR("SympatheticViewFactory"));
		return noErr;
	}
	if (inID == kAudioUnitProperty_ParameterStringFromValue && inScope == kAudioUnitScope_Global) {
		AudioUnitParameterStringFromValue* info = (AudioUnitParameterStringFromValue*)outData;
		if (info->inParamID == kParam_Bank && info->inValue != NULL) {
			int bank = (int)(*info->inValue + 0.5);
			if (bank < 0) bank = 0;
			if (bank >= kNumBanks) bank = kNumBanks - 1;
			info->outString = SympatheticBankName(bank);
			return noErr;
		}
		return kAudioUnitErr_InvalidParameter;
	}
	if (inID == kAudioUnitProperty_ParameterValueFromString && inScope == kAudioUnitScope_Global) {
		AudioUnitParameterValueFromString* info = (AudioUnitParameterValueFromString*)outData;
		if (info->inParamID == kParam_Bank && info->inString != NULL) {
			for (int i = 0; i < kNumBanks; i++) {
				if (CFStringCompare(info->inString, SympatheticBankName(i), 0) == kCFCompareEqualTo) {
					info->outValue = (AudioUnitParameterValue)i;
					return noErr;
				}
			}
			return kAudioUnitErr_InvalidParameter;
		}
		return kAudioUnitErr_InvalidParameter;
	}
	return AUEffectBase::GetProperty (inID, inScope, inElement, outData);
}

//	Sympathetic::Initialize
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ComponentResult Sympathetic::Initialize()
{
	ComponentResult result = AUEffectBase::Initialize();
	if (result == noErr)
		Reset(kAudioUnitScope_Global, 0);
	return result;
}

#pragma mark ____SympatheticEffectKernel

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Sympathetic::SympatheticKernel::Reset()
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void		Sympathetic::SympatheticKernel::Reset()
{
	for (int i = 0; i < kMaxStrings; i++) {
		strings[i] = SympatheticString();
	}
	stringCount = 0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Sympathetic::SympatheticKernel::Process
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void		Sympathetic::SympatheticKernel::Process(	const Float32 	*inSourceP,
													Float32		 	*inDestP,
													UInt32 			inFramesToProcess,
													UInt32			inNumChannels,
													bool			&ioSilence )
{
	UInt32 nSampleFrames = inFramesToProcess;
	const Float32 *sourceP = inSourceP;
	Float32 *destP = inDestP;

	double sr = GetSampleRate();
	double A = GetParameter(kParam_Resonance);
	double B = GetParameter(kParam_Sustain);
	double C = GetParameter(kParam_Damping);
	double D = GetParameter(kParam_Detune);
	double E = GetParameter(kParam_Bank);
	double F = GetParameter(kParam_DryWet);

	int bank = (int)(E + 0.5); // E is an indexed parameter: 0..kNumBanks-1
	if (bank < 0) bank = 0;
	if (bank >= kNumBanks) bank = kNumBanks - 1;
	double q = 10.0 + 490.0 * B * B; // B -> quality factor (ring duration)
	double damp = C; // C -> harmonic damping (brightness)
	double detuneScale = D; // D -> per-string detune spread
	double resGain = A; // A -> resonance level
	double wet = F; // F -> dry/wet

	stringCount = bankCounts[bank];
	for (int i = 0; i < stringCount; i++) {
		double cents = detuneScale * detuneCents[i];
		double f0 = bankFreqs[bank][i] * pow(2.0, cents / 1200.0);
		strings[i].configure(f0, sr, q, damp);
	}

	while (nSampleFrames-- > 0) {
		double inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23) inputSample = fpd * 1.18e-17;

		double ring = 0.0;
		for (int i = 0; i < stringCount; i++) {
			ring += strings[i].process(inputSample);
		}
		ring = softclip(ring);

		inputSample += ring * resGain * wet;

		//begin 32 bit floating point dither
		int expon; frexpf((float)inputSample, &expon);
		fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
		inputSample += ((double(fpd)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
		//end 32 bit stereo floating point dither

		*destP = (Float32)inputSample;

		sourceP += inNumChannels; destP += inNumChannels;
	}

	// This effect has a long ringing tail, so never advertise the
	// output as silent while there may still be energy in the strings.
	ioSilence = false;
}
