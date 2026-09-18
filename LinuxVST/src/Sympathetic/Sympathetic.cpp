/* ========================================
 *  Sympathetic - Sympathetic.cpp
 *  Copyright (c) 2026, Panaudio uses the MIT license
 * ======================================== */

#ifndef __Sympathetic_H
#include "Sympathetic.h"
#endif

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {return new Sympathetic(audioMaster);}

Sympathetic::Sympathetic(audioMasterCallback audioMaster) :
    AudioEffectX(audioMaster, kNumPrograms, kNumParameters)
{
	A = 0.5;
	B = 0.5;
	C = 0.5;
	D = 0.4;
	E = 0.0;
	F = 1.0;

	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	//this is reset: values being initialized only once. Startup values, whatever they are.

	stringCount = 0;

    _canDo.insert("plugAsChannelInsert"); // plug-in can be used as a channel insert effect.
    _canDo.insert("plugAsSend"); // plug-in can be used as a send effect.
    _canDo.insert("x2in2out");
    setNumInputs(kNumInputs);
    setNumOutputs(kNumOutputs);
    setUniqueID(kUniqueId);
    canProcessReplacing();     // supports output replacing
    canDoubleReplacing();      // supports double precision processing
	programsAreChunks(true);
    vst_strncpy (_programName, "Default", kVstMaxProgNameLen); // default program name
}

Sympathetic::~Sympathetic() {}
VstInt32 Sympathetic::getVendorVersion () {return 1000;}
void Sympathetic::setProgramName(char *name) {vst_strncpy (_programName, name, kVstMaxProgNameLen);}
void Sympathetic::getProgramName(char *name) {vst_strncpy (name, _programName, kVstMaxProgNameLen);}
//Panaudio likes to ignore this stuff. Make your own programs, and make a different plugin rather than
//trying to do versioning and preventing people from using older versions. Maybe they like the old one!

static float pinParameter(float data)
{
	if (data < 0.0f) return 0.0f;
	if (data > 1.0f) return 1.0f;
	return data;
}

VstInt32 Sympathetic::getChunk (void** data, bool isPreset)
{
	float *chunkData = (float *)calloc(kNumParameters, sizeof(float));
	chunkData[0] = A;
	chunkData[1] = B;
	chunkData[2] = C;
	chunkData[3] = D;
	chunkData[4] = E;
	chunkData[5] = F;
	/* Note: The way this is set up, it will break if you manage to save settings on an Intel
	 machine and load them on a PPC Mac. However, it's fine if you stick to the machine you
	 started with. */

	*data = chunkData;
	return kNumParameters * sizeof(float);
}

VstInt32 Sympathetic::setChunk (void* data, VstInt32 byteSize, bool isPreset)
{
	float *chunkData = (float *)data;
	A = pinParameter(chunkData[0]);
	B = pinParameter(chunkData[1]);
	C = pinParameter(chunkData[2]);
	D = pinParameter(chunkData[3]);
	E = pinParameter(chunkData[4]);
	F = pinParameter(chunkData[5]);
	/* We're ignoring byteSize as we found it to be a filthy liar */

	/* calculate any other fields you need here - you could copy in
	 code from setParameter() here. */
	return 0;
}

void Sympathetic::setParameter(VstInt32 index, float value) {
    switch (index) {
        case kParamA: A = value; break;
        case kParamB: B = value; break;
        case kParamC: C = value; break;
        case kParamD: D = value; break;
        case kParamE: E = value; break;
        case kParamF: F = value; break;
        default: throw; // unknown parameter, shouldn't happen!
    }
}

float Sympathetic::getParameter(VstInt32 index) {
    switch (index) {
        case kParamA: return A; break;
        case kParamB: return B; break;
        case kParamC: return C; break;
        case kParamD: return D; break;
        case kParamE: return E; break;
        case kParamF: return F; break;
        default: break; // unknown parameter, shouldn't happen!
    } return 0.0; //we only need to update the relevant name, this is simple to manage
}

void Sympathetic::getParameterName(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "Resonance", kVstMaxParamStrLen); break;
		case kParamB: vst_strncpy (text, "Sustain", kVstMaxParamStrLen); break;
		case kParamC: vst_strncpy (text, "Damping", kVstMaxParamStrLen); break;
		case kParamD: vst_strncpy (text, "Detune", kVstMaxParamStrLen); break;
		case kParamE: vst_strncpy (text, "Bank", kVstMaxParamStrLen); break;
		case kParamF: vst_strncpy (text, "Dry/Wet", kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
    } //this is our labels for displaying in the VST host
}

void Sympathetic::getParameterDisplay(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: float2string (A, text, kVstMaxParamStrLen); break;
        case kParamB: float2string (B, text, kVstMaxParamStrLen); break;
        case kParamC: float2string (C, text, kVstMaxParamStrLen); break;
        case kParamD: float2string (D, text, kVstMaxParamStrLen); break;
        case kParamE: {
			int bank = (int)(E * kNumBanks);
			if (bank < 0) bank = 0;
			if (bank >= kNumBanks) bank = kNumBanks - 1;
			switch (bank) {
				case 0: vst_strncpy (text, "Standard", kVstMaxParamStrLen); break;
				case 1: vst_strncpy (text, "Twelve", kVstMaxParamStrLen); break;
				case 2: vst_strncpy (text, "DADGAD", kVstMaxParamStrLen); break;
				case 3: vst_strncpy (text, "Dm drone", kVstMaxParamStrLen); break;
				default: vst_strncpy (text, "", kVstMaxParamStrLen); break;
			}
			break;
		}
        case kParamF: float2string (F, text, kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
	}
}

void Sympathetic::getParameterLabel(VstInt32 index, char *text) {
    switch (index) {
        case kParamA: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamB: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamC: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamD: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamE: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        case kParamF: vst_strncpy (text, "", kVstMaxParamStrLen); break;
        default: break; // unknown parameter, shouldn't happen!
    }
}

VstInt32 Sympathetic::canDo(char *text)
{ return (_canDo.find(text) == _canDo.end()) ? -1: 1; } // 1 = yes, -1 = no, 0 = don't know

bool Sympathetic::getEffectName(char* name) {
    vst_strncpy(name, "Sympathetic", kVstMaxProductStrLen); return true;
}

VstPlugCategory Sympathetic::getPlugCategory() {return kPlugCategEffect;}

bool Sympathetic::getProductString(char* text) {
  	vst_strncpy (text, "Panaudio Sympathetic", kVstMaxProductStrLen); return true;
}

bool Sympathetic::getVendorString(char* text) {
  	vst_strncpy (text, "Panaudio", kVstMaxVendorStrLen); return true;
}
