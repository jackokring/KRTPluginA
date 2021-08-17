KRTPluginA 
===

Version: `RACK_VERSION.WORKING_MACHINES_COUNT.FIX_INCREMENT` supplied in tags. Any commit not tagged is not a version for compile, although it may work it could also be quite dangerous. The index is numbered so that `WORKING_MACHINES_COUNT` makes sense given the development ordering of the index.

Current public version: [here](https://library.vcvrack.com/KRTPluginA) achives for older versions of the demos maybe available [here](https://github.com/jackokring/KRTPluginADemo).

Compiled with: `gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0` or later. Any issues due to edge case optimizations of `float` may be corrected if they are problems.

Less technical: [Captain's log](https://jackokring.github.io/KRTPluginADemo/) for less detail and more of an overview.

Index - Demo
===

1. [A](#a) - [DA](https://github.com/jackokring/KRTPluginADemo/blob/master/DA.vcv)
2. [μ](#μ) - [crisp](https://github.com/jackokring/KRTPluginADemo/blob/master/crisp.vcv)
3. [T](#t) - [I shat the she riff](https://github.com/jackokring/KRTPluginADemo/blob/master/I%20shat%20the%20she%20riff..vcv)
4. [L](#l) - [boring](https://github.com/jackokring/KRTPluginADemo/blob/master/boring.vcv)
5. [;D](#d) - [DA](https://github.com/jackokring/KRTPluginADemo/blob/master/DA.vcv)
6. [R](#r) - No demo required
7. [Ω](#Ω) - [Omicron](https://github.com/jackokring/KRTPluginADemo/blob/master/Omicron.vcv)
8. [V](#v) - [Omicron](https://github.com/jackokring/KRTPluginADemo/blob/master/Omicron.vcv)
9. [F](#f) - [morph](https://github.com/jackokring/KRTPluginADemo/blob/master/morph.vcv)
10. [W](#w) - [W](https://github.com/jackokring/KRTPluginADemo/blob/master/W.vcv)
11. [Y](#y) - [Y](https://github.com/jackokring/KRTPluginADemo/blob/master/Y.vcv)
12. [O](#o) - [parabolic](https://github.com/jackokring/KRTPluginADemo/blob/master/parabolic.vcv)
13. [S](#s) - [SE](https://github.com/jackokring/KRTPluginADemo/blob/master/SE.vcv)
14. [E](#e) - [SE](https://github.com/jackokring/KRTPluginADemo/blob/master/SE.vcv)
15. [X](#x) - [wave](https://github.com/jackokring/KRTPluginADemo/blob/master/wave.vcv)
16. [M](#m) - [M](https://github.com/jackokring/KRTPluginADemo/blob/master/M.vcv)

[Easy Access Demo Archive](https://github.com/jackokring/KRTPluginADemo/archive/refs/heads/master.zip) includes all the demos above as a **.zip** file for easy downloading and none of that cut, paste, save and rename business. 

A
===

*49% Tranparent Module - Made with sexspex plastique*

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
* Frequency CV `CV`
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

μ
===

*The Gold One - Summation generatve additive sythesis*

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
* 1V/Oct `CVHz`
* 1V/halflife  `CVλ`
* In 1 `IN1`
* In 2 `IN2`
* In 3 `IN3`

Outs
---
* Differential `Δ`
* Signal Pole `1/Δ`
* Signal Log `LNΔ`
* Integral Linear `Σ`
* Integral Pole `1/Σ`
* Integral Log `LNΣ`

Some weird calculus module. It really depends what it can be made to do. It might become unstable if you feedback connections. It might not as well. The `λ` control only affects the last 5 outputs, as the 3 integral representations should converge to the same value and `λ` affects the effective sample timing offset next to a potential singular solution.

All the estimators are predictive one sample into the future (with `λ` kind of being a sophisticated constant of integration), a pre slew filter is set as a 6 dB/Oct zero delay filter. This filter may work opposite to how you might expect for some outputs.

So the inputs `IN1`, `IN3` and `IN3` are controlled by **centred dB scaling** gainuverters for gain, amplified by an exponetial `dB`, slew limited and then processed for calculus estimates (9th order FIR).

The integrals are further processed by 3 point series accelerations (a tiny effect but interesting) to occur at a sample in the future. `λ` applies an effective modulation of this integral time step from a fraction of a sample to multiple samples. The halflife then is measured in exponetial inverse octaves.

I mean why a 9th order FIR, and the number 27 of the sporadic simple groups connection? From 2 comes 2 to the power of all N. From 3 comes all odd numbers? This likely explains the choice of cube root to control the range of output voltage.

Blerb (not essential to understanding the module)
---

He's irisponsible :D https://en.wikipedia.org/wiki/Jacques_Tits ... roll on (Monster Moonshine)[https://en.wikipedia.org/wiki/Monstrous_moonshine] and let's not confuse maths with physics "speculated" on maths. Hope I got all the integration by parts and collection of approximants in limited terms right? (Some 2's, some 3's, some hopeful sporadics on the elliptic). Did I mention the congruants div 3 and the determinats (count 44 (18n+26 (Groupies)[https://en.wikipedia.org/wiki/Sporadic_group])) with Z(p) and alternating groups to a 42 which the circle and ellipse are the alternating or not on the major minor radius?

Initialize the big bang with differential estimates of the future captain, so obivating the definitional requirements at the singularity Captain. Well Gamma Bob, well gamma. A form of precision series truncation crusher, nice! None of that chucking away an abstract precision and getting right in there with truncation of summation generatve additive sythesis, for that timbral union.

But isn't it about the singularity context of integration in a set of limits x, x+y where x is unkown but gestimated in a relative context Captain? Yes Bob all error in the calculus should be reported as this module is not about error, and others maybe built.

But shouldn't a sine converge to sin(x)? And and truncation of terms provide a controlled deviation from sin(x) for Fauxrier harminics? Exactly Boz, the sweet low power law alias summands.

"Proxy off a dirty hole of unknown gain? How dare you? Must fix the incomming to be at least plesant." -- Captain of the Exponential.

T
===

*The White One - A harmonic pitch down and up delay*

Parameters
---
* Course Tune `NOTE`
* Fine Tune `FINE`

Ins
---
* In `IN`
* Trigger`TRIG`

Outs
---
* High Trigger `HI`
* Out `OUT`

The trigger `TRIG` resets the delay and produces a down shifted `OUT` until the up shift starts causing `HI` to output a rising edge and the delay switches an octave up until the expected retrigger point. The delay plays regular speed until a retrigger. The delay will reset itself if the delay buffer becomes full.

Going down is always a prerequisite before rising to the top! And normal is very overdue. The maximal delay length is divided by polyphony. What's the cache flush GB/s on this thing? I'm thinking 4 bars and a bit or so?

Polyphonic triggers occur at the optimal octave intercept per trigger, and so the outputs become phasic to the triggers. `HI` can off trigger a trigger to sync with the up speed clearing of the precision down beat zero delay. No trigger equals monophony and no delay after the "blip" as there is a trigger need on `TRIG`.

Try `NOTE` at 5 semitones, and alter the trigger rate for a sync chord arpy sound. In this sense other semitone values are better at lower `TRIG` rates. Try some drums on the `HI` out for an interesting timing effect too.

L
===

*A White One - A synchronized delay with pre-trigger end*

Parameters
---
* Pre-trigger Delay `PRE`
* Fine Tune `FINE`

Ins
---
* In `IN`
* Trigger `TRIG`

Outs
---
* Pre-trigger `PRE`
* Out `OUT`

A synchronized delay with a `PRE` control to remove a time dependant on sample latency so as to match timing one synchronization trigger later. So suppose you wish to sychronize to some live play and are prepared to time up one `TRIG` later (say a bar for arguments sake), then you can increase `PRE` to achive that less than the bar delay with latency compensating the rest of the bar delay which `PRE` removed on `OUT`. The output `PRE` gates high before the bar end to allow down module to trigger other things with the need to be triggered before the bar start. 

;D
===

*Another White One - A future estimator VCA with error*

Parameters
---
* Level `dB`
* Level Modulation `CVdB`
* Frequency `FRQ`

Ins
---
* In `IN`
* Level Modulation `CVdB`

Outs
---
* Out `OUT`

Exponetial amplitude modulation with a future projection estimate 2 samples into the future for phase critical all pass when used with modules that have a 1 sample delay. The `CVdB` control is a **centred dB scaling** gainuverter.

Also useful for pointing out transients in `IN` as the estimator is excited by the parts of the signal with aliasing potential.

R
===

*The Final White One - A simple mixer split*

Parameters
---
* On Level `ON`
* As Level `AS`

Ins
---
* In `IN`
* Mix Some On `ON`

Outs
---
* Get As `AS`
* Out `OUT`

Mix some `ON` into the `IN` making an `OUT` with control of it `AS` for easy modulation mixing. The controls are **centred dB scaling** gainuverters.

Ω
===

*The 70% Grey One (With A Display) - Clock distribution randomizer*

Parameters
---
* Bird `BIRD`
* Seed `SEED`
* Randomize `RAND`

Ins
---
* Clock (Polyphony Master) `CLK`
* Reset (Polyphonic OR) `RST`

Outs (Bi-phase on clock given current letter)
---
* `AN`, `BO`, `CP`, `DQ`, `ER`, `FS`, `GT`, `HU`, `IV`, `JW`, `KX`, `LY`, `MZ`

`RST` starts a sequence of gate activations, from the beginning, on the outputs `AN` to `MZ` advanced by the `CLK` trigger. The `SEED` controls the initial symbol and some randomness of the following symbols. `RAND` increaes the chaos of randomization from the initial `SEED`. The magic of `BIRD` (the word) applies some modal operator lambda calculus to modify the sequence with the **very** discrete occasional stall in the `MM` Omega bird. This is not the differential calculus of Newton fame, but the one with names such as [Godel](https://en.wikipedia.org/wiki/Kurt_G%C3%B6del) and [Church](https://en.wikipedia.org/wiki/Alonzo_Church).

There is a great book on modal logic called [To Mock a Mocking Bird](https://en.wikipedia.org/wiki/To_Mock_a_Mockingbird), itself a follow up to [Forever Undecided](https://en.wikipedia.org/wiki/Raymond_Smullyan). A lay person introduction to the subject.

Higher polyphony using `CLK` copies the mono output one from the lowest channel to the highest given a `CLK` on a channel. For example when clocked the second channel takes the output of the first channel.

V
===

*The 70% Grey One - Envelope VCA and CV producer*

Parameters
---
* Base Frequency `Hz`
* Envelope Attack `ATK`
* Envelope Decay `DCY`

Ins
---
* Frequency Modulation `CVHz`
* Amplitude Modulation `CVdB`
* Envelope Time Modulation `CVs`
* 3 of
    * Oscillator In `IN`
    * Up 5 Semitones Trigger `T5`
    * Up 7 Semitones Trigger `T7`

Outs
---
* 3 of
    * Frequency CV to Oscillator`CV`
    * Envelope Shaped Output `OUT`

Takes in triggers on `T5` and `T7` to trigger one of the three vertical envelopes. `IN` is from the oscillator source, and `OUT` is that signal modified by the envelope. `CV` can be tuned dependant on the trigger pair combination. I thought it would help with using Ω. Although it might be quite useful on its own. The two outer `OUT` signals are normalized to the center `OUT` channel for less cables.

F
===

*The Blue One - Morph filter*

Parameters
---
* Pole Spread `SPD`
* Pole Skew `SKW`
* Frequency `FRQ`
* Low All High `LAH`
* Invert `INV`
* Drive `DRV`

Ins
---
* Frequency Modulation `FRQ`
* Spread Modulation `SPD`
* Skew Modulation `SKW`
* Low All High Modulation `LAH`
* Invert Modulation `INV`
* Drive Modulation `DRV`
* Input `IN`

Outs
---
* Output `OUT`

A filter with some internal gain processing to handle filter k values into self-oscillation. The four corners set by spread and skew form Linkwitz to Butterwork on spread when skew is full off, and Legendre to Bessel when skew is full on. The concept is stabilized by soft clipping.

W
===

*The Red One - Gate transposer*

Parameters
---
* None

Ins
---
* Input `IN`
* Gates Transpose `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`, `10`, `11`

Outs
---
* Output Add `+`
* Output Subtract `-`

To assist in turning patterned gates into CV chromatic sequences. Simple.

Y
===
[Y extra details](Y.md)

*The White Buttons One - Gate sequencer*

Parameters
---
* Tempo `TEMPO`
* Gate Length `G.LEN`
* Four Mode Buttons `PAT`, `SEQ`, `MUTE`, `NOW`
* The Main Buttons (28 for notes and triples)
* Run and Stop `RUN`
* Reset `RST`
* Copy Pattern `CPY` (makes note of active channel for `PAT` paste channel copies)
* Paste Pattern `PST` (only pastes into current channel in `PAT` mode)
* Shuffle Jazz `JAZZ` (three altering micro timings)

Ins
---
* CV `CV`
* Gate `GATE`
* Position Jazz `JAZZ` (later versions)

Outs
---
* Clock `CLK`
* Reset `RST`

A gate pattern sequencer. An obvious layout, but the modes might be complicated.
* `PAT` a pattern edit mode similar to a standard drum machine.
* `SEQ` triggers and selects various sequences. For performance play. Alters assigned pattern on pattern change controls of triples.
* `MUTE` allows channel mutes while maintaining some pattern change control on triples.
* `NOW` puts direct channel gates on the notes (also selects active channel via last pressed channel), while maintaining some pattern change control on triples.

The `CPY` light indicates when the copy source is the same as the paste source. The `PST` light indicates if the copy channel source is the same as the current paste channel.

The `C2` to `B5` keyboard MIDI range on `CV` triggered by `GATE` operate the buttons.
* `C#`, `D#` pair operate function and mode buttons left to right (in pairs).
* `C`, `D`, `E` operate triple buttons.
* `F#`, `G#`, `A#` also operate triple buttons for utility.
* `F`, `G`, `A`, `B` operate quater buttons.

O
===

*The Beigish One - Triple parabolic skew feedback oscillator*

Parameters
---
* Frequency `FRQ` (three)
* Feedback `FBK` (three)

Ins
---
* CV `CV`

Outs
---
* Output `OUT`

An algorithm for sound. Simple to use.

S
===

*A 30% Grey One - MIDI clock signalling to position CV*

Parameters
---
* BPM Estimate `BPM`
* Divide `DIV`

Ins
---
* Clock `CLK`
* Start `STRT`
* Stop `STOP`
* Continue `CONT` (suitable for `Y` module `JAZZ` input)

Outs
---
* Out `OUT`

For interfacing with a MIDI clock provided as an output by the MIDI-CV system module. 64 beats equals 10V and reset back to 0V unipolar signal.

E
===

*Another 30% Grey One - Envelope and VCA*

Parameters
---
* Attack `ATK`
* Release `REL`
* Modulation Level `MOD`

Ins
---
* In `IN`
* Trigger `TRIG`

Outs
---
* Modulation `MOD`
* Out `OUT`

A simple AR envelope with a VCA. The `MOD` control is a **centred dB scaling** gainuverter.

X
===

*Another 30% Grey One - Wave-folder*

Parameters
---
* Fold `FOLD`
* Kind `KIND`
* Mash Noise `MASH`
* Wet Mix `WET`

Ins
---
* In `IN`
* Modulate Fold `MFLD`

Outs
---
* Out `OUT`

An oversampled wave-folder based on Chebychev polynomials and some extras for a bit of a difference.

M
===

*Another 30% Grey One - Emphisis filter*

Parameters
---
* Low Frequency `LOW`
* Low Gain `LOdB`
* High Frequency `HIGH`
* High Gain `HIdB`

Ins
---
* CV Low `LO`
* CV High `HI`
* In `IN`
* Return `RTN`

Outs
---
* Send `SND`
* Out `OUT`

An equilazation filter to decrease the effect of non-linear signal processing on high frequencies throwing them into alias distortion. A return filter removes the emphisis. So the base is amplified and treble reduced on the forward send filter, and the opposite for the return filter. It may make interfacing a "phono" deck easier, but does not amplify the tiny signals from a deck.

It reduces the non-linear shaping of high frequencies in the send return path from injecting harmonics into the alias band above the sampling Nyquist frequency. So good for base harmonic generation. The headroom of VCV rack should support the +/- 50V peak to peak. But you might need a bit of gain reduction before this module.