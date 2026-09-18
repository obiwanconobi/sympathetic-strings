# Sympathetic Strings (guitar) — Audio Plugin Plan

## 1. Concept & target sound
Sympathetic strings are unplayed strings that ring when the guitar's energy coincides with their resonant frequencies (fundamental + harmonics). The plugin adds a sustained, shimmering "tarab/12-string/hardanger" resonance tail to a dry guitar signal, without pitch-tracking — it's a **fixed tuning bank** of resonators that the input excites naturally.

Signal flow:
```
dry guitar ──┬──────────────────────────────► mix
             │
             └─► [string 1: comb @ f1] ─┐
                 [string 2: comb @ f2] ─┤
                 ...                     ├─ sum ─► gain ─► wet
                 [string N: comb @ fN] ─┘
```

## 2. DSP design (the core)
Each sympathetic string = a bank of **2-pole bandpass resonators** at the string's harmonic frequencies `f, 2f, 3f…` (a "modal" model of a string). A high-Q bandpass rings when the input has energy near its center frequency, which *is* the sympathetic behavior; the sharp resonance also gives the selectivity a real string has.

> Design note (revised during Phase 1 testing): the original "feedback comb" idea was dropped. A single feedback comb's anti-resonant valleys are only −6 dB, so it can't discriminate between matching and non-matching notes (measured 8.5× vs. the 33× of a 2-pole bandpass). The bandpass resonator also has exactly zero DC gain (no DC-peak runaway).

Per-string structure:
- **Fundamental resonator** at `f` (2-pole bandpass, RBJ coefficients, 0 dB peak gain).
- **Optional harmonic resonators** at `2f, 3f…` with lower gain / lower Q (damping) for richness.
- **Q** (from Sustain) sets both ring duration and selectivity.

Parameter map (A–F, matching AirWindows conventions):
| Param | Name | Meaning |
|-------|------|---------|
| A | Resonance | wet level / string volume |
| B | Sustain | feedback gain (decay length) |
| C | Damping | lowpass cutoff in feedback loop (brightness of ring) |
| D | Detune | small per-string frequency spread (chorus/realism) |
| E | Bank | selects tuning bank (guitar open strings / octaves+fifths / D-modal / chromatic) |
| F | Dry/Wet | output blend |

Tuning banks (presets of `f` arrays, ~10–16 strings each): guitar open strings E2–E4, a "12-string" octave+fifth set, a DADGAD-friendly modal set, etc.

## 3. File & project structure (AirWindows pattern)
Mirror the repo layout:
```
plugins/LinuxVST/src/Sympathetic/Sympathetic.cpp   (the plugin)
plugins/LinuxVST/src/Sympathetic/Sympathetic.h
```
- Single `.cpp` per plugin: `createEffectInstance`, `AudioEffectX` ctor (A–F defaults + member init), `getChunk/setChunk` (preset save/load), `setParameter/getParameter/*Name/*Display/*Label`, `canDo`, `getEffectName`, `processReplacing` + `processDoubleReplacing`.
- Fixed `kUniqueId`, `kNumInputs=2`, `kNumOutputs=2`, `x2in2out`, `canDoubleReplacing`.
- Denormal hygiene: `fpdL/fpdR` seed pattern + `while` guards, as in `ToTape5`.

## 4. Implementation phases
1. **Phase 0 — toolchain & skeleton.** Install VST2 SDK 2.4 (AirWindows expects it); copy a template plugin, rename to `Sympathetic`, get an empty `processDoubleReplacing` (dry passthrough) compiling + loading in a Linux host (e.g. Carla, ardour, reaper). Lock the build script.
2. **Phase 1 — single string.** Implement one comb resonator at a fixed frequency; verify it rings when fed a matching sine, and is silent-ish on non-matching content. Validate fractional-delay tuning.
3. **Phase 2 — full bank.** Add N strings, the 4 DSP params (A–D), and the tuning-bank selector (E). Wire excitation = input into every comb; sum tails; apply wet gain.
4. **Phase 3 — polish.** Dry/Wet (F), stereo (independent L/R buffers or mono-summed resonance), level matching vs dry (so bypass is fair), denormal protection, chunk/preset correctness.
5. **Phase 4 — AU + packaging.** Generate AU via the AirWindows AU template (Mac build; deferred until a Mac is available), finalize parameter ranges/defaults.

## 5. Verification
- Build warning-free; load in host; confirm preset save/load.
- A/B bypass for volume parity.
- Feed synthesized tests: matching sine → long ring; swept/mismatched → minimal bleed.
- Frequency check: confirm each string's peaks land on the intended note.

## 6. Risks / decisions to revisit
- ~~**Integer delay precision**~~ — moot: switched to 2-pole bandpass resonators (no delay line).
- **Feedback stability** near `g→1` → bandpass poles stay well inside the unit circle (Q clamped); still keep a soft limiter on the resonance bus in Phase 3.
- **"Every note excites everything"** — resolved by the 2-pole bandpass (33× selectivity measured); higher Q or harmonic modes can sharpen further if needed.
- **Bank-count vs CPU** — each string is 1–3 cheap biquads; 16 strings × a few modes is negligible.

## 7. Status
- [x] Phase 0 — toolchain & skeleton
- [x] Phase 1 — single string resonator
- [x] Phase 2 — full bank + params
- [x] Phase 3 — polish (stereo, dry/wet, denormals)
- [ ] Phase 4 — AU + packaging