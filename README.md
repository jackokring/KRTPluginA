KRTPluginA
===

A (Tranparent Module - made with sexspex plastique)
===

Parameters
---
* Frequency (-12dB) `FRQ`
* Resonance `REZ`
* Frequency Modulation Gain `MOD`
* Relative Plate (-6 dB) `OFF`
* Plate Gain `AM`
* Plate Modulation Gain `MAM`

Ins
---
* -18 dB In (HP on `XP12`) `IN1`
* -18 dB Predictive (LP on `XP12`) `LP1`
* Frequency CV (Polyphonic master) `CV`
* -12 dB In (No Plate) `IN2`
* -6 dB HP In (Plate Ring) `HP1`
* Modulation `MOD`

Outs
---
* -6 dB LP Out `LP1`
* Tracking Out `CV`
* -12/-18 dB Out `LP12`
* -6 dB HP Out `HP1`
* Plate Out `AM`
* -12/-18 dB Inverse Extended Filter Out `XP12`

A generic filter with some nice fun additions for a characteristic sound. How much fun can you have with 4 poles and some other DSP functions? Yes, it is by design digital, although it might be possible to make an analogue version of most if not all of the implied circuit.

The filter poles are ZDF (Zero Delay Filter) arithmetic. The `LP1` predictive input creates a
ninth order estimate one sample into the future. This has the effect of a bit of phasing and given that it is not equally fed into the ring modulation too, makes it spectrally different. The 45 degree lead lag of the ring makes for a ninety on the AM, for even harmonic self AM power.

If some controls appear not to operate use `IN1` and `LP12` out, with `CV` for tracking, and then all the controls work. Some specific inserts are pre or post certain controls. The plate AM tracking stacks on the filter corner tracking.

μ (The Gold One)
===

Parameters
---
* Master Gain `dB`
* Slew Limit `Hz`
* Halflife `λ`
* Gain 1 `G1`
* Gain 2 `G2`
* Gain 3 `G3`

Ins
---
* 1V/6dB `CVdB`
* 1V/Oct (Polyphonic master) `CVHz`
* 1V/halflife  `CVλ`
* In 1 `IN1`
* In 2 `IN2`
* In 3 `IN3`

Outs
---
* Differential `Δ`
* Second Differential `ΔΔ`
* Third Differential `ΔΔΔ`
* Integral Linear `Σ`
* Integral Pole `1/Σ`
* Integral Log `LNΣ`

Some weird calculus module. It really depends what it can be made to do. It might become unstable if you feedback connections. It might not as well. The `λ` control only affect the integral outputs, as the 3 representations should converge to the same value and `λ` affects the effective sample timing offset which defaults to 1.

All the estimators are predictive one sample into the future (with `λ` kind of being a sophisticated constant of integration), and the slew filter is a 6 dB/Oct zero delay filter.

So the inputs `IN1`, `IN3` and `IN3` are controlled by the attenuverters for gain, amplified by an exponetial `dB`, slew limited and then processed for calculus estimates (9th order FIR).

The integrals are further processed by 3 point series accelerations (a tiny effect but interesting) to occur at a sample in the future. `λ` applies an effective modulation of this integral time step from a fraction of a sample to multiple samples. The halflife then is measured in exponetial samples.
