# Building "Sympathetic" on macOS

This is the runbook for building the `Sympathetic` plugin (VST2 and/or Audio Unit)
on a Mac. The DSP is written in the AirWindows style and is self-contained, so the
port is mostly mechanical. VST2 is nearly a copy-paste; the AU needs a thin wrapper
around the same DSP.

> **Licensing note:** the VST2 SDK and Apple's Core Audio SDK must be obtained by
> you under their respective licenses. They are not committed to this repo.

---

## 0. Prerequisites

| Requirement | Notes |
|---|---|
| Xcode + Command Line Tools | Any recent Xcode works for VST; the AirWindows AU template was written for older Xcode, so see Path B for caveats |
| VST 2.4 SDK | Needed for **Path A (VST)**. Structure: `pluginterfaces/vst2.x/` and `public.sdk/source/vst2.x/` |
| Apple Core Audio SDK (a.k.a. `CA_SDK`) | Needed for **Path B (AU)**. The AirWindows AU template references a `CA_SDK` folder |
| (Optional) Apple Developer ID | Only needed for code-signing/notarization of distributed builds |

---

## 1. Get the source onto the Mac

Copy this repo to the Mac (git clone, `scp -r`, a thumb drive, whatever):

```
git clone <your-remote> sympathetic-strings
```

The only files the Mac build needs are under:

```
LinuxVST/src/Sympathetic/
├── Sympathetic.h          # VST2 class (AudioEffectX)
├── Sympathetic.cpp        # VST2 wrapper (params, chunk, host glue)
├── SympatheticProc.cpp    # the DSP loop (processReplacing / processDoubleReplacing)
├── SympatheticString.h    # one string = 3 bandpass modes  (PORTABLE)
└── Resonator.h            # 2-pole bandpass resonator        (PORTABLE)
```

`SympatheticString.h` and `Resonator.h` are pure C++ with zero VST/AU dependencies —
they drop into either build unchanged. `SympatheticProc.cpp` holds the audio loop
(everything inside `while (--sampleFrames >= 0)`), which is format-agnostic apart
from the `float**/double**` buffer access and `getSampleRate()`.

The AirWindows Mac project templates live in the AirWindows repo
(`https://github.com/airwindows/airwindows`, under `plugins/MacVST` and
`plugins/MacAU`). Use `ToTape5` there as the reference skeleton.

---

## 2. Path A — VST2 (do this first; our code is already VST2)

1. **Clone the AirWindows repo** (for the Xcode project skeleton):
   ```
   git clone https://github.com/airwindows/airwindows
   ```

2. **Copy and rename the template:**
   ```
   cp -R airwindows/plugins/MacVST/ToTape5 MacVST/Sympathetic
   ```
   Rename all `ToTape5` files to `Sympathetic` (`ToTape5.cpp`, `.h`, `Proc.cpp`,
   and the `.xcodeproj`).

3. **Replace the plugin source** in `MacVST/Sympathetic/source/` with ours:
   - `Sympathetic.h`, `Sympathetic.cpp`, `SympatheticProc.cpp`
   - `SympatheticString.h`, `Resonator.h`

4. **Point Xcode at the VST2.4 SDK:**
   - Delete the red `vstsdk2.4` folder from the project.
   - Drag in your `vstsdk2.4` folder (only `pluginterfaces/vst2.x` and
     `public.sdk/source/vst2.x`). Choose **"Added folders: Create groups"**,
     **do not copy items**.
   - Set **Target → Build Settings → Header Search Paths** to your SDK, e.g.
     `/Users/you/Desktop/vstsdk2.4/**`.

5. **Project settings** (match AirWindows):
   - Scheme: "Any Mac", Build Configuration = **Release**.
   - Project → Info: macOS Deployment Target **11.1**.
   - Build Settings (All): Architectures = **Standard (Apple Silicon, Intel)**,
     Base SDK = **macOS**, Build Active Architecture Only = **Yes**,
     Always Search User Paths = **No**.
   - Target → Packaging: Product Name = `Sympathetic`.
   - Target → Deployment: Installation Directory = `$(HOME)/Library/Audio/Plug-Ins/VST`,
     Strip Style = Debugging Symbols.

6. **Build** (Product → Clean Build Folder, then Build). The `.vst` bundle lands in
   `build/Release/`.

7. **Install & test:** copy `Sympathetic.vst` to `~/Library/Audio/Plug-Ins/VST/` and
   load it in Reaper/Logic (wrapped)/Carla/etc. Confirm it's listed as
   **"Sympathetic"**, and that the six params (Resonance, Sustain, Damping, Detune,
   Bank, Dry/Wet) are present.

---

## 3. Path B — Audio Unit

The AU needs a wrapper because Apple's AU API (`AUEffectBase`) is a different base
class from VST's `AudioEffectX`. The DSP itself is unchanged.

### 3.1 Porting the code

1. **Copy the AU template:**
   ```
   cp -R airwindows/plugins/MacAU/ToTape5 MacAU/Sympathetic
   ```
   Rename `ToTape5.cpp/.h/.r/.exp/Info.plist/ToTape5Version.h` and the `.xcodeproj`
   to `Sympathetic`.

2. **Carry over the portable DSP files unchanged:**
   - `Resonator.h`, `SympatheticString.h`

3. **Port the class** (`Sympathetic.h`): keep the same members the VST version has:
   ```cpp
   SympatheticString stringsL[16];
   SympatheticString stringsR[16];
   int stringCount;
   uint32_t fpdL, fpdR;
   float A, B, C, D, E, F;   // + the kParamX / kNumParameters enum
   ```
   but derive from `AUEffectBase` instead of `AudioEffectX`, and port
   `getParameter`/`setParameter`/`getParameterName`/`getParameterDisplay`/
   `getParameterLabel` to the AU equivalents (the AirWindows AU template already has
   these methods; just swap in our six names/values).

4. **Port the DSP loop** (`SympatheticProc.cpp`): move the body of our
   `processReplacing` into the AU's render/`Process` method. The per-sample core is
   unchanged:
   ```
   mono-free: process L through stringsL[i], R through stringsR[i]
   ring = sum; softclip; output = input + ring * A * F
   ```
   Keep the per-block setup (bank selection from `E`, `Q = 10 + 490*B*B`,
   `damp = C`, detune from `D`, `resGain = A`, `wet = F`) and the tuning-bank table.

5. **Four-char IDs** — edit `SympatheticVersion.h`:
   ```c
   #define Sympathetic_COMP_MANF    'Dthr'   // your own Apple-registered ID
   #define Sympathetic_COMP_SUBTYPE 'ssym'   // unique to this plugin
   ```
   In `Sympathetic.r`, set `NAME = "Airwindows: Sympathetic"` and
   `ENTRY_POINT = "SympatheticEntry"`.

### 3.2 Entry point (Apple Silicon / modern AUs)

For ARM (Apple Silicon) AUs you must use the modern factory entry point:

- In `Sympathetic.cpp`, change:
  ```
  COMPONENT_ENTRY(Sympathetic)
  ```
  to:
  ```
  AUDIOCOMPONENT_ENTRY(AUBaseFactory, Sympathetic)
  ```
- In `Sympathetic.exp`, add the factory export so it reads:
  ```
  _SympatheticEntry
  _SympatheticFactory
  ```
- In `Info.plist`, add the `AudioComponents` block (see the AirWindows
  `plugins/AirwindowsAUToSignedAUProcess.txt`); set `type` = `aufx`,
  `subtype` = `ssym`, `manufacturer` = your ID, `factoryFunction` =
  `SympatheticFactory`.

### 3.3 Xcode settings (AU)

- Delete the red `PublicUtility` and `AUPublic` folders; drag in `CA_SDK`
  (create groups, don't copy), Header Search Paths → `/Users/you/Desktop/CA_SDK/**`.
- Scheme: Any Mac, Release.
- Project Info: macOS Deployment Target **11.1**.
- Build Settings: Standard Architectures (Apple Silicon, Intel), Base SDK macOS,
  Build Active Architecture Only = Yes, Always Search User Paths = No.
- Target: delete "Build Carbon Resources" build phase; Rez flags cleared.
- Clean Build Folder, then Build.

The `.component` bundle lands in `build/Release/`. Copy to
`~/Library/Audio/Plug-Ins/Components/`.

### 3.4 Validate the AU

```
auval -v aufx ssym Dthr
```
and load it in Logic/GarageBand/Reaper. It should appear as
**"Airwindows: Sympathetic"**.

---

## 4. Signing & notarization (distribution only)

For local dev you can skip this. For distribution:

- Use your Developer ID (not "Christopher Johnson" as in the AirWindows notes).
- Sign the bundle with a **Developer ID Application** certificate (Xcode's
  "Manual" signing → your Team + Developer ID cert, or re-sign via `codesign`).
- Notarize + staple (either via `xcrun notarytool` or a GUI wrapper like SD Notary).

Full reference workflow: `plugins/AirwindowsAUToSignedAUProcess.txt` and
`plugins/AirwindowsVSTToSignedVSTProcess.txt` in the AirWindows repo.

---

## 5. Verification checklist

- [ ] Plugin appears in host as "Sympathetic".
- [ ] Six parameters visible, Bank shows named values (Standard / Twelve / DADGAD / Dm drone).
- [ ] Playing a note (or feeding a matching sine) produces a ringing sympathetic tail after the note stops.
- [ ] Bypass A/B: no level jump (dry passthrough when Resonance = 0).
- [ ] No clicks/pops when switching Bank or sweeping Detune.
- [ ] Apple Silicon build runs (not just Intel), if applicable.
