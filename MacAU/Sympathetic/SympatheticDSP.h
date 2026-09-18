/* ========================================
 *  Sympathetic - SympatheticDSP.h
 *  Format-agnostic DSP constants and helpers shared by the
 *  VST2 and Audio Unit wrappers. The per-string resonator
 *  model itself lives in Resonator.h / SympatheticString.h.
 *  Copyright (c) 2026, Panaudio uses the MIT license
 * ======================================== */

#ifndef __SympatheticDSP_H
#define __SympatheticDSP_H

#include <math.h>

const int kNumBanks = 4;
const int kMaxStrings = 16;

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

#endif
