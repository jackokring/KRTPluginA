KRTPluginA
===

Version: `RACK_VERSION.WORKING_MACHINES_COUNT.FIX_INCREMENT` supplied in tags. Any commit not tagged is not a version for compile, although it may work it could also be quite dangerous. The index is numbered so that `WORKING_MACHINES_COUNT` makes sense given the development ordering of the index.

Current public version: [here](https://library.vcvrack.com/KRTPluginA) achives for older versions of the demos maybe available [here](https://github.com/jackokring/KRTPluginADemo).

Compiled with: `gcc (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0` or later. Any issues due to edge case optimizations of `float` may be corrected if they are problems.

Less technical: [Captain's log](https://jackokring.github.io/KRTPluginADemo/) for less detail and more of an overview.

Index - Version - Demo
===

The version number indicates the API build compatibility and feature use. So 1 is VCVRack V1 API. These will be changed as higher API number features are used and adapted to.

1. [A](#a) - 2 - [DA](https://github.com/jackokring/KRTPluginADemo/blob/master/DA.vcv)
2. [μ](#μ) - 1 - [crisp](https://github.com/jackokring/KRTPluginADemo/blob/master/crisp.vcv)
3. [T](#t) - 1 - [I shat the she riff](https://github.com/jackokring/KRTPluginADemo/blob/master/I%20shat%20the%20she%20riff..vcv)
4. [L](#l) - 1 - [boring](https://github.com/jackokring/KRTPluginADemo/blob/master/boring.vcv)
5. [;D](#d) - 1 - [DA](https://github.com/jackokring/KRTPluginADemo/blob/master/DA.vcv)
6. [R](#r) - 1 - No demo required
7. [Ω](#Ω) - 2 - [Omicron](https://github.com/jackokring/KRTPluginADemo/blob/master/Omicron.vcv)
8. [V](#v) - 1 - [Omicron](https://github.com/jackokring/KRTPluginADemo/blob/master/Omicron.vcv)
9. [F](#f) - 2 - [morph](https://github.com/jackokring/KRTPluginADemo/blob/master/morph.vcv)
10. [W](#w) - 1 - [W](https://github.com/jackokring/KRTPluginADemo/blob/master/W.vcv)
11. [Y](#y) - 1 - [Y](https://github.com/jackokring/KRTPluginADemo/blob/master/Y.vcv)
12. [O](#o) - 1 - [parabolic](https://github.com/jackokring/KRTPluginADemo/blob/master/parabolic.vcv)
13. [S](#s) - 1 - [SE](https://github.com/jackokring/KRTPluginADemo/blob/master/SE.vcv)
14. [E](#e) - 1 - [SE](https://github.com/jackokring/KRTPluginADemo/blob/master/SE.vcv)
15. [X](#x) - 1 - [wave](https://github.com/jackokring/KRTPluginADemo/blob/master/wave.vcv)
16. [M](#m) - 1 - [M](https://github.com/jackokring/KRTPluginADemo/blob/master/M.vcv)
17. [Q](#q) - 1 - [Q](https://github.com/jackokring/KRTPluginADemo/blob/master/Q.vcv)
18. [C](#c) - 1 - No demo required
19. [U](#u) - 1
20. [B](#b) - 1 - I seems to be adding to patch [B](https://github.com/jackokring/KRTPluginADemo/blob/master/B.vcv)
21. [J](#j) - 1
22. [K](#k) - 1
23. [H](#h) - 1
24. [I](#i) - 1
25. [G](#g) - 1

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
* In `IN` (send filter input)
* Return `RTN` (return filter input)

Outs
---
* Send `SND` (send filter output)
* Out `OUT` (return filter output)

An equilazation filter to decrease the effect of non-linear signal processing on high frequencies throwing them into alias distortion. A return filter removes the emphisis. So the base is amplified and treble reduced on the forward send filter, and the opposite for the return filter. It may make interfacing a "phono" deck easier, but does not amplify the tiny signals from a deck.

It reduces the non-linear shaping of high frequencies in the send return path from injecting harmonics into the alias band above the sampling Nyquist frequency. So good for base harmonic generation. The headroom of VCV rack should support the +/- 50V peak to peak. But you might need a bit of gain reduction before this module.

**WARNING** The default gain settings are good for RIAA equalization, but could be reduced closer to 0 dB for general use. As the return filter is the opposite, that `HIdB` value can be misleading on what the return filter is doing. Under some circumstances it could **destroy tweeter speakers** just as the send filter could wobble the neighbourhood walls.

Q
===

*A Green One - Quantum gravimitator*
[The Big Manual?](Q.md)

Parameters
---
* Omega Frequency `OM`
* Sigularity Management `SING`
* Plank `PLK`
* Newton `NTN`
* Mass Ratio `MASS`
* Radial Tangential Angle `ANG`

Ins
---
* Omega Frequency Modulation `OM`
* Sigularity Management Modulation `SING`
* Plank Modulation `PLK`
* Newton Modulation `NTN`
* Mass Ratio Modulation `MASS`
* Radial Tangential Angle Modulation `ANG`
* Input `IN`

Outs
---
* Output `OUT`

A physical model simulation. Treats the input as a driven mass radius and calculates the uncertainty of the mass using a quantum and gravity connection.

I could explain by (https://drive.google.com/file/d/0B-avx1luFGRbRTVFOFhSWkluQWM/view?usp=sharing&resourcekey=0-YsVFVTBgEFU-1QAJ1hYz8A)[Mistakes and genius] as yes something are wrong, and somethings being applied have an error less than Planks constant cubed which is very small as to be imesurable, yet defined as certain by fixing the constant as a committie committed to, and so ...

Is dark matter the intergral over an average on a non-linear 1/r^2 law of an uncertainty in radial potential energy? Is dark energy a tangential version? As distances get smaller the F=kr of quark confinement something that 1/r maps to r becomes a strong force unseen equivelent? Dark strong? A prediction or just a tune from finding subharmonic greatest common division of the natural omega frequency of matter?

How to connect your modular to a scanning tunnelling microscope? Contact a physical? Enjoy!

To `H` or not to wage? Das ist da krewestion? Wether it is better to enter into mass statisctic of global endomology or to find yourself under a hierachy of perfection?

C
===

*A Purple One - Mixer sixer*

Parameters
---
* Channel 1 Polarity `G1`
* Channel 1 Low Pass `F1`
* Channel 2 Polarity `G2`
* Channel 2 Low Pass `F2`
* Channel 3 Polarity `G3`
* Channel 3 Low Pass `F3`


Ins
---
* Channel 1 In `IN1`
* Channel 2 In `IN2`
* Channel 3 In `IN3`
* Input `LINK`

Outs
---
* Channel 1 Out `OUT1`
* Channel 2 Out `OUT2`
* Channel 3 Out `OUT3`
* Output Mix `MIX`

The `G1`, `G2` and `G3` controls are **centred dB scaling** gainuverters. Each channel has a low pass single pole filter and an independent output. The `LINK` input is combined with all the outputs not connected to make the `MIX` output. Connecting to an output `OUT1`, `OUT2` or `OUT3` removes it form the `MIX` for utility as an attenuverter.

U
===

*A Pink One - Sample and hold quantizer*

Parameters
---
* Quantize `QUNT` (in semitones)
* Noise `NSE` (when no input color filter)

Ins
---
* Clock Sample `CLK`
* Input `IN`

Outs
---
* Output `OUT`
* Quantized Output `QOUT`

A simple triple sample and hold with semitone quantizer.

B
===

*A White One with Buttons - Performance switch*

Parameters
---
* 18 Route Buttons (unlabeled)
* Mode `MODE`

Ins
---
* `1`, `2`, `3`, `4`, `5` and `6`

Outs
---
* `A`, `B` and `C`

Performance controller switch. The `MODE` cycles through `GREEN`, `RED` and `BLUE` modes.

* `GREEN` mode: the routing buttons select switching of the inputs to the outputs. Multiple inputs can be selected and merged.
* `RED` mode: the routing buttons select one of 18 recorded patterns to recall. The active pattern is shown `RED` perhaps mixed with `GREEN` and `BLUE` if other data is shown.
* `BLUE` mode: the routing buttons select function merge processing. Each of the three outputs can have the different functions applied in all combinations (top to bottom application order).

J
===

*A Cyan One - All-pass filter*

Parameters
---
* Frequency `FRQ`
* Order `ODR`
* Bypass, High, All `BHA`
* Wet Mix `WET`
* LFO Frequency `LFO`
* LFO Depth `LVL`

Ins
---
* Frequency CV `FRQ`
* Order CV `ODR`
* Bypass, High, All CV `BHA`
* Wet Mix CV `WET`
* LFO Frequency CV `LFO`
* Input `IN`

Outs
---
* LFO Out `LFO`
* Output `OUT`

An all-pass filter with some phaser style effect possibilities. A direct `LFO` output is available, as this is more important than CV control over the `LVL` sweep range.

K
===

*A Black One - Phase modulation oscillator*

Parameters
---
* Frequency `FRQ`
* Filter `LPF`
* Ratio `RT1`
* Ratio `RT2`
* Modulation Depth `MD1`
* Modulation Depth `MD2`

Ins
---
* Frequency CV `FRQ`
* Filter CV `LPF`
* Ratio CV `RT1`
* Ratio CV `RT2`
* Modulation Depth CV `MD1`
* Modulation Depth CV `MD2`

Outs
---
* Modulation Out `MOD`
* Output `OUT`

H
===

*Another Grey One - Base Gimpathizer*

Parameters
---
* 9 Hammond Harminics `16'`, `5 1/3'`, `8'`, `4'`, `2 2/3'`, `2'`, `1 3/5'`, `1 1/3'` and `1'`

Ins
---
* 9 Hammond Harminic CV `16'`, `5 1/3'`, `8'`, `4'`, `2 2/3'`, `2'`, `1 3/5'`, `1 1/3'` and `1'`
* Frequency CV `FRQ`
* Phase Modulation CV `PM`

Outs
---
* Out `OUT`

Classic organ harmonics. Add a little phase modulation weighted more for the base harmonics, and make some interesting timbral textures. Hold on to your base bins. To not weight the phase modulation gets harmonically (n+1) very fast.

I
===

*A Light Grey One - Clock divider*

Parameters
---
* 3 Dividers `D1`, `D2`, `D3`
* 3 Phases `P1`, `P2`, `P3`

Ins
---
* Clock `CLK`
* Reset `RST`

Outs
---
* Down Beat `DWN`
* All But Down Beat `SANS`
* 3 Outs `O1`, `O2`, `O3`
* Exclusive Or of All 3 Outputs `XOR`

Apart from being 3 variable clock dividers which maintain a sychronization phase this module also extracts the down beat for modules when high `RST` does not override `CLK` and so need the first clock extracting (the downbeat) to synchronize them. Such "armed before play" (rising edge reset) modules **should** copy the **MIDI standard** but are different from the **normal electronic logic synchronous convention** of hold in the first state on `RST` and so **don't** ignore the first `CLK`. They **should** use an extra state (called "armed" here) and then need a `CLK` to enter the first state.

If a module resets on rising edge `RST` but also reacts after to rising edge `CLK` and also **doesn't** have an "armed" state, it jump forward one step out of sync. If it has an "armed" state and `RST` is slightly delayed compared to `CLK` is spend one step in the "armed" state based on a race of signals through wiring.

If the `CLK` is before the `RST`: the clock is the downbeat. `DWN` and `SANS` can be used as `RST` and `CLK` for downstream modules without confusion if they reset into the first state not needing a clock to transition from an "armed" state. The clock is truncated on `RST` and sent out `DWN` and is a glitch on `SANS`.

If the `CLK` is after the `RST` (but `RST` is still high): the clock is the downbeat. `DWN` and `SANS` can be used as `RST` and `CLK` for downstream modules without confusion if they reset into the first state not needing a clock to transition from an "armed" state. The clock is sent out `DWN` and does not appear out of `SANS`.

So basically that part of the module helps when synchronizing modules with a rising edge `RST` instead of a high continuous `RST` (with `CLK` ignore). Extracting the downbeat is in a sense a clock ignore on `SANS`. `DWN` is in a sense a `RST` but delayed until after `CLK`.

G
===

*Another Grey One - Compressor*

Parameters
---
* Attack `ATK`
* Decay `DCY`
* Threshold `THR`
* Ratio of compression `RTO` (or expansion -ve ratios)
* High Pass Cut `CUT`
* High Pass Resonance `Q`
* Mix to Makeup Level `MIX`
* Envelope Follower to CV `ENV`

Ins
---
* Frequency `FRQ`
* Sidechain `SCH` (normaled from `IN`)
* Input `IN`

Outs
---
* Frequency `FRQ` (with `ENV` mixed in with control gain)
* Envelope Follower `ENV`
* Compressed Output `OUT`

A normal compressor with a sidechain input. The envelope follower can be mixed with the `FRQ` in to produce some pinging harmonics by the output `FRQ` (modified by the envelope), along with a high pass filter which can be tuned down for a `Q` boost and sub-base cut. The `MIX` brings a compressed `OUT` back upto normalized levels like an auto-computed make up gain. The amount of applied compression could more accurately be the envelope applied via the `ENV` control to the `FRQ` chain. This chain also applies to the `CUT` to slide up the corner frequency of the high-pass filter.

The `CUT` filter is perhaps the most interesting bit of the design. The range of the control is slanted towards sun-octaves, while the `Q` has been somewhat limited as it is applied post compression. It makes a nice effect when compression "breathing" is swept using `CUT` and `ENV` controls to balance the style.
