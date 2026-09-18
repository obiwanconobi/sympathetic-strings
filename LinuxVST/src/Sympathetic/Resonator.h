/* ========================================
 *  Sympathetic - Resonator.h
 *  A single mode of a sympathetic string: a 2-pole
 *  bandpass resonator (RBJ-style, 0 dB peak gain).
 *  A sympathetic string is a bank of these at its
 *  fundamental + harmonics. Copyright (c) 2026,
 *  Panaudio uses the MIT license.
 * ======================================== */

#ifndef __Resonator_H
#define __Resonator_H

#include <math.h>

class Resonator {
public:
    Resonator() {
        x1 = 0.0; x2 = 0.0;
        y1 = 0.0; y2 = 0.0;
        B0 = 0.0; A1 = 0.0; A2 = 0.0;
        Q = 50.0;
    }

    void setQ(double q) {
        if (q < 0.5) q = 0.5;
        if (q > 1000.0) q = 1000.0;
        Q = q;
    }

    // tune to freq (Hz) at sample rate sr; call after setQ
    void setFrequency(double freq, double sr) {
        if (freq < 1.0) freq = 1.0;
        if (freq > sr * 0.45) freq = sr * 0.45;
        double w0 = 2.0 * M_PI * freq / sr;
        double alpha = sin(w0) / (2.0 * Q);
        double a0 = 1.0 + alpha;
        B0 = alpha / a0;                 // b0/a0 (and b2/a0 = -B0)
        A1 = (-2.0 * cos(w0)) / a0;      // a1/a0
        A2 = (1.0 - alpha) / a0;         // a2/a0
    }

    // process one sample; returns the resonant (ringing) output
    double process(double x) {
        double y = B0 * (x - x2) - A1 * y1 - A2 * y2;
        x2 = x1; x1 = x;
        y2 = y1; y1 = y;
        return y;
    }

private:
    double x1, x2, y1, y2;
    double B0, A1, A2;
    double Q;
};

#endif
