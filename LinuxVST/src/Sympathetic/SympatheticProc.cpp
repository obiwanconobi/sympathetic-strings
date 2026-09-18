/* ========================================
 *  Sympathetic - SympatheticProc.cpp
 *  Copyright (c) 2026, Panaudio uses the MIT license
 * ======================================== */

#ifndef __Sympathetic_H
#include "Sympathetic.h"
#endif

static const double bankFreqs[kNumBanks][kMaxStrings] = {
	// Bank 0: Standard (guitar open strings, E)
	{ 82.4069, 110.0000, 146.8324, 195.9977, 246.9417, 329.6276,
	  0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	// Bank 1: Twelve (open strings + octaves)
	{ 82.4069, 110.0000, 146.8324, 195.9977, 246.9417, 329.6276,
	  164.8138, 220.0000, 293.6648, 391.9954, 493.8833, 659.2551,
	  0.0, 0.0, 0.0, 0.0 },
	// Bank 2: DADGAD
	{ 73.4162, 110.0000, 146.8324, 195.9977, 220.0000, 293.6648,
	  391.9954, 440.0000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	// Bank 3: D minor drone
	{ 73.4162, 110.0000, 146.8324, 174.6141, 220.0000, 293.6648,
	  349.2282, 440.0000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }
};

static const int bankCounts[kNumBanks] = { 6, 12, 8, 8 };

static const double detuneCents[kMaxStrings] = {
	0.0, 1.8, -1.6, 2.4, -2.0, 1.2, -1.0, 2.8,
	-2.2, 0.8, -1.4, 2.0, -1.8, 1.5, -0.9, 2.5
};

static inline double softclip(double x) {
	double a = fabs(x);
	if (a > 1.57079633) a = 1.57079633;
	a = sin(a);
	if (x < 0.0) return -a;
	return a;
}

void Sympathetic::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
    float* in1  =  inputs[0];
    float* in2  =  inputs[1];
    float* out1 = outputs[0];
    float* out2 = outputs[1];

	double sr = getSampleRate();
	int bank = (int)(E * kNumBanks);
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
		stringsL[i].configure(f0, sr, q, damp);
		stringsR[i].configure(f0, sr, q, damp);
	}

	double inputSampleL;
	double inputSampleR;
	double ringL;
	double ringR;

    while (--sampleFrames >= 0)
    {
		inputSampleL = *in1;
		inputSampleR = *in2;
		if (fabs(inputSampleL)<1.18e-23) inputSampleL = fpdL * 1.18e-17;
		if (fabs(inputSampleR)<1.18e-23) inputSampleR = fpdR * 1.18e-17;

		ringL = 0.0;
		ringR = 0.0;
		for (int i = 0; i < stringCount; i++) {
			ringL += stringsL[i].process(inputSampleL);
			ringR += stringsR[i].process(inputSampleR);
		}
		ringL = softclip(ringL);
		ringR = softclip(ringR);

		inputSampleL += ringL * resGain * wet;
		inputSampleR += ringR * resGain * wet;

		//begin 32 bit stereo floating point dither
		int expon; frexpf((float)inputSampleL, &expon);
		fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
		inputSampleL += ((double(fpdL)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
		frexpf((float)inputSampleR, &expon);
		fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
		inputSampleR += ((double(fpdR)-uint32_t(0x7fffffff)) * 5.5e-36l * pow(2,expon+62));
		//end 32 bit stereo floating point dither

		*out1 = inputSampleL;
		*out2 = inputSampleR;

		*in1++;
		*in2++;
		*out1++;
		*out2++;
    }
}

void Sympathetic::processDoubleReplacing(double **inputs, double **outputs, VstInt32 sampleFrames)
{
    double* in1  =  inputs[0];
    double* in2  =  inputs[1];
    double* out1 = outputs[0];
    double* out2 = outputs[1];

	double sr = getSampleRate();
	int bank = (int)(E * kNumBanks);
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
		stringsL[i].configure(f0, sr, q, damp);
		stringsR[i].configure(f0, sr, q, damp);
	}

	double inputSampleL;
	double inputSampleR;
	double ringL;
	double ringR;

    while (--sampleFrames >= 0)
    {
		inputSampleL = *in1;
		inputSampleR = *in2;
		if (fabs(inputSampleL)<1.18e-23) inputSampleL = fpdL * 1.18e-17;
		if (fabs(inputSampleR)<1.18e-23) inputSampleR = fpdR * 1.18e-17;

		ringL = 0.0;
		ringR = 0.0;
		for (int i = 0; i < stringCount; i++) {
			ringL += stringsL[i].process(inputSampleL);
			ringR += stringsR[i].process(inputSampleR);
		}
		ringL = softclip(ringL);
		ringR = softclip(ringR);

		inputSampleL += ringL * resGain * wet;
		inputSampleR += ringR * resGain * wet;

		//begin 64 bit stereo floating point dither
		//int expon; frexp((double)inputSampleL, &expon);
		fpdL ^= fpdL << 13; fpdL ^= fpdL >> 17; fpdL ^= fpdL << 5;
		//inputSampleL += ((double(fpdL)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
		//frexp((double)inputSampleR, &expon);
		fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
		//inputSampleR += ((double(fpdR)-uint32_t(0x7fffffff)) * 1.1e-44l * pow(2,expon+62));
		//end 64 bit stereo floating point dither

		*out1 = inputSampleL;
		*out2 = inputSampleR;

		*in1++;
		*in2++;
		*out1++;
		*out2++;
    }
}
