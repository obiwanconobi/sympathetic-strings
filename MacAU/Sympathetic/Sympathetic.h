/*
 *	File:		Sympathetic.h
 *
 *	Version:	1.0
 *
 *	Copyright:  Copyright (c) 2026 Panaudio, Panaudio uses the MIT license
 *
 *	Audio Unit wrapper for the Sympathetic string-resonator effect.
 *	The DSP is shared with the VST2 build (Resonator.h /
 *	SympatheticString.h / SympatheticDSP.h); only the host glue differs.
 */

#include "AUEffectBase.h"
#include "SympatheticVersion.h"

#if AU_DEBUG_DISPATCHER
	#include "AUDebugDispatcher.h"
#endif

#ifndef __Sympathetic_h__
#define __Sympathetic_h__

#include "SympatheticDSP.h"
#include "SympatheticString.h"

#pragma mark ____Sympathetic Parameters

// parameters
static const float kDefaultValue_Resonance = 0.5;
static const float kDefaultValue_Sustain   = 0.5;
static const float kDefaultValue_Damping   = 0.5;
static const float kDefaultValue_Detune    = 0.4;
static const float kDefaultValue_Bank      = 0.0;
static const float kDefaultValue_DryWet    = 1.0;

static CFStringRef kParameterResonanceName = CFSTR("Resonance");
static CFStringRef kParameterSustainName   = CFSTR("Sustain");
static CFStringRef kParameterDampingName   = CFSTR("Damping");
static CFStringRef kParameterDetuneName    = CFSTR("Detune");
static CFStringRef kParameterBankName      = CFSTR("Bank");
static CFStringRef kParameterDryWetName    = CFSTR("Dry/Wet");

enum {
	kParam_Resonance = 0,
	kParam_Sustain   = 1,
	kParam_Damping   = 2,
	kParam_Detune    = 3,
	kParam_Bank      = 4,
	kParam_DryWet    = 5,
	kNumberOfParameters = 6
};

#pragma mark ____Sympathetic

class Sympathetic : public AUEffectBase
{
public:
	Sympathetic(AudioUnit component);
#if AU_DEBUG_DISPATCHER
	virtual ~Sympathetic () { delete mDebugDispatcher; }
#endif

	virtual AUKernelBase *		NewKernel() { return new SympatheticKernel(this); }

	virtual	ComponentResult		GetParameterValueStrings(AudioUnitScope			inScope,
														 AudioUnitParameterID	inParameterID,
														 CFArrayRef *			outStrings);

	virtual	ComponentResult		GetParameterInfo(AudioUnitScope			inScope,
												 AudioUnitParameterID	inParameterID,
												 AudioUnitParameterInfo	&outParameterInfo);

	virtual ComponentResult		GetPropertyInfo(AudioUnitPropertyID		inID,
												AudioUnitScope			inScope,
												AudioUnitElement		inElement,
												UInt32 &			outDataSize,
												Boolean	&			outWritable );

	virtual ComponentResult		GetProperty(AudioUnitPropertyID inID,
											AudioUnitScope 		inScope,
											AudioUnitElement 		inElement,
											void *			outData);

	virtual ComponentResult		Initialize();
	virtual bool				SupportsTail () { return true; }
	virtual Float64				GetTailTime() {return 2.0;}   // in SECONDS
	virtual Float64				GetLatency() {return 0.0;}    // in SECONDS

	/*! @method Version */
	virtual ComponentResult		Version() { return kSympatheticVersion; }

protected:
	class SympatheticKernel : public AUKernelBase		// most of the real work happens here
	{
public:
		SympatheticKernel(AUEffectBase *inAudioUnit )
		: AUKernelBase(inAudioUnit)
	{
	}

		// *Required* override for the process method for this effect
		// processes one channel of interleaved samples
		virtual void		Process(	const Float32 	*inSourceP,
										Float32		 	*inDestP,
										UInt32 			inFramesToProcess,
										UInt32			inNumChannels,
										bool			&ioSilence);

		virtual void		Reset();

		private:
		SympatheticString strings[kMaxStrings];
		int stringCount;
		uint32_t fpd;
	};
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
