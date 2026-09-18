/* ========================================
 *  Sympathetic - SympatheticString.h
 *  One sympathetic string: a small bank of
 *  2-pole bandpass resonators at its fundamental
 *  and low harmonics (a "modal" model).
 *  Copyright (c) 2026, Panaudio uses the MIT license.
 * ======================================== */

#ifndef __SympatheticString_H
#define __SympatheticString_H

#include "Resonator.h"

class SympatheticString {
public:
    enum { kModes = 3 };

    SympatheticString() {
        g[0] = 0.0; g[1] = 0.0; g[2] = 0.0;
    }

    // tune to f0 (Hz) at sr; q = base quality factor, damp = 0..1 brightness
    void configure(double f0, double sr, double q, double damp) {
        mode[0].setQ(q);
        mode[0].setFrequency(f0, sr);
        g[0] = 1.0;

        double q2 = q * (1.0 - 0.35 * damp);
        if (q2 < 0.5) q2 = 0.5;
        mode[1].setQ(q2);
        mode[1].setFrequency(2.0 * f0, sr);
        g[1] = 0.5 * (1.0 - 0.5 * damp);

        double q3 = q * (1.0 - 0.55 * damp);
        if (q3 < 0.5) q3 = 0.5;
        mode[2].setQ(q3);
        mode[2].setFrequency(3.0 * f0, sr);
        g[2] = 0.25 * (1.0 - 0.7 * damp);
    }

    double process(double x) {
        return mode[0].process(x) * g[0]
             + mode[1].process(x) * g[1]
             + mode[2].process(x) * g[2];
    }

private:
    Resonator mode[kModes];
    double g[kModes];
};

#endif
